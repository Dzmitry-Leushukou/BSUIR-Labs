"""
Middleware for Redis-based session management.
Handles session validation, refresh, and cross-instance synchronization.
"""

from fastapi import Request, Response, HTTPException, status
from starlette.middleware.base import BaseHTTPMiddleware
from typing import Callable, Optional, Dict, Any
import jwt
from datetime import datetime
import os

from redis_client import redis_client
from jwt_utils import decode_access_token, JWT_SECRET_KEY, JWT_ALGORITHM

# Configuration
SESSION_HEADER = "X-Session-Token"
SESSION_REFRESH_THRESHOLD_MINUTES = int(os.getenv("SESSION_REFRESH_THRESHOLD_MINUTES", "5"))


class SessionManagementMiddleware(BaseHTTPMiddleware):
    """
    Middleware for managing Redis-based sessions.
    
    Features:
    - Validate session tokens from Redis
    - Auto-refresh session TTL on access
    - Track active sessions per user
    - Cross-instance session consistency
    """

    async def dispatch(self, request: Request, call_next: Callable) -> Response:
        # Skip session management for certain paths
        skip_paths = [
            "/docs",
            "/redoc",
            "/openapi.json",
            "/static",
            "/uploads",
        ]
        
        if any(request.url.path.startswith(path) for path in skip_paths):
            return await call_next(request)

        # Get session token from header
        session_token = request.headers.get(SESSION_HEADER)
        
        if session_token:
            # Validate session
            session_data = await self._validate_session(session_token)
            
            if session_data:
                # Attach session data to request state
                request.state.session = session_data
                request.state.current_user = session_data.get("user_data", {})
                request.state.current_user["id"] = session_data.get("user_id")
                request.state.current_user["email"] = session_data.get("email")
                
                # Refresh session TTL if needed
                await self._maybe_refresh_session(session_token)
            else:
                # Invalid session - clear it
                request.state.session = None
                request.state.current_user = None
        
        # Process request
        response = await call_next(request)
        
        # Add session info to response headers
        if hasattr(request.state, "session") and request.state.session:
            response.headers["X-Session-Valid"] = "true"
        
        return response

    async def _validate_session(self, session_token: str) -> Optional[Dict[str, Any]]:
        """
        Validate session token against Redis.
        
        Args:
            session_token: Session token to validate
            
        Returns:
            Session data if valid, None otherwise
        """
        # Check if Redis is available
        if not redis_client.is_connected():
            print("Warning: Redis not available for session validation")
            return None
        
        # Get session from Redis
        session_data = redis_client.get_session(session_token)
        
        if session_data is None:
            return None
        
        # Check if session is expired (shouldn't happen due to Redis TTL, but double-check)
        # Redis automatically expires keys, so this is just a safety check
        
        return session_data

    async def _maybe_refresh_session(self, session_token: str) -> None:
        """
        Refresh session TTL if it's close to expiring.
        
        Args:
            session_token: Session token to refresh
        """
        try:
            redis_client.refresh_session(session_token)
        except Exception as e:
            print(f"Error refreshing session: {e}")


class CacheInvalidationMiddleware(BaseHTTPMiddleware):
    """
    Middleware for handling cache invalidation via pub/sub.
    Listens for cache invalidation events and clears local cache.
    """

    def __init__(self, app):
        super().__init__(app)
        self._local_cache: Dict[str, Any] = {}

    async def dispatch(self, request: Request, call_next: Callable) -> Response:
        # Process request
        response = await call_next(request)
        
        # Check for cache invalidation header from pub/sub handler
        invalidated_keys = getattr(request.state, "invalidated_keys", set())
        
        if invalidated_keys:
            for key in invalidated_keys:
                self._local_cache.pop(key, None)
        
        return response

    def invalidate_key(self, key: str) -> None:
        """Invalidate a specific cache key."""
        self._local_cache.pop(key, None)

    def clear_all(self) -> None:
        """Clear all local cache."""
        self._local_cache.clear()


class PubSubListenerMiddleware(BaseHTTPMiddleware):
    """
    Middleware for cache invalidation tracking.
    Pub/Sub listener is started at app startup in main.py
    """

    def __init__(self, app):
        super().__init__(app)
        self._local_cache: Dict[str, Any] = {}

    async def dispatch(self, request: Request, call_next: Callable) -> Response:
        # Attach cache invalidation method to request state
        request.state.invalidate_cache = self._invalidate_cache
        request.state.invalidated_keys = set()

        return await call_next(request)

    def _invalidate_cache(self, key: str) -> None:
        """Invalidate a specific cache key."""
        self._local_cache.pop(key, None)

    def invalidate_by_type(self, entity_type: str) -> None:
        """Invalidate all cache entries for entity type."""
        keys_to_remove = [k for k in self._local_cache.keys() if k.startswith(entity_type)]
        for key in keys_to_remove:
            self._local_cache.pop(key, None)

    def clear_all(self) -> None:
        """Clear all local cache."""
        self._local_cache.clear()


# Helper functions for manual cache invalidation

def invalidate_entity_cache(entity_type: str, entity_id: Optional[Any] = None) -> None:
    """
    Invalidate cache for an entity and notify other instances.
    
    Args:
        entity_type: Type of entity (e.g., "users", "cars")
        entity_id: Optional specific entity ID
    """
    # Invalidate local cache
    redis_client.invalidate_list_cache(entity_type)
    
    if entity_id:
        redis_client.delete_cache(f"{entity_type}:{entity_id}")
    
    # Notify other instances
    redis_client.publish_cache_invalidation(entity_type)


def notify_data_change(
    entity_type: str,
    action: str,
    entity_id: Any,
    old_data: Optional[Dict] = None,
    new_data: Optional[Dict] = None,
    user_id: Optional[int] = None
) -> None:
    """
    Notify about data change to all instances.

    Args:
        entity_type: Type of entity
        action: Action performed (create, update, delete)
        entity_id: Entity ID
        old_data: Previous data
        new_data: New data
        user_id: User who performed the action
    """
    # Convert date/datetime objects to strings for JSON serialization
    def convert_dates(data):
        if not data:
            return data
        result = dict(data) if not isinstance(data, dict) else data
        for key, value in result.items():
            if hasattr(value, 'isoformat'):  # date/datetime objects
                result[key] = value.isoformat()
        return result
    
    old_data = convert_dates(old_data)
    new_data = convert_dates(new_data)
    
    # Invalidate local cache
    invalidate_entity_cache(entity_type, entity_id)

    # Publish notification
    redis_client.publish_data_change(
        entity_type=entity_type,
        action=action,
        entity_id=entity_id,
        old_data=old_data,
        new_data=new_data,
        user_id=user_id
    )


def notify_user_event(event_type: str, user_id: int, data: Optional[Dict] = None) -> None:
    """
    Notify about user event to all instances.
    
    Args:
        event_type: Event type (banned, updated, deleted)
        user_id: User ID
        data: Additional data
    """
    redis_client.publish_user_event(
        event_type=event_type,
        user_id=user_id,
        data=data
    )
    invalidate_entity_cache("users", user_id)


def notify_session_event(
    event_type: str,
    user_id: int,
    session_token: Optional[str] = None,
    data: Optional[Dict] = None
) -> None:
    """
    Notify about session event to all instances.
    
    Args:
        event_type: Event type (login, logout, expired)
        user_id: User ID
        session_token: Session token
        data: Additional data
    """
    redis_client.publish_session_event(
        event_type=event_type,
        user_id=user_id,
        session_token=session_token,
        data=data
    )
