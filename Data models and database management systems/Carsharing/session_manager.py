"""
Redis-based session manager for cross-instance session storage.
Provides distributed session management for multiple application instances.
"""

import redis
import json
import os
from typing import Optional, Dict, Any
from datetime import datetime, timedelta
from decimal import Decimal
from dotenv import load_dotenv

load_dotenv()

REDIS_HOST = os.getenv("REDIS_HOST", "localhost")
REDIS_PORT = int(os.getenv("REDIS_PORT", 6379))
REDIS_DB = int(os.getenv("REDIS_DB", 0))

# Session configuration
SESSION_PREFIX = "session:"
SESSION_TTL_MINUTES = int(os.getenv("SESSION_TTL_MINUTES", "30"))
SESSION_REFRESH_ON_ACCESS = os.getenv("SESSION_REFRESH_ON_ACCESS", "true").lower() == "true"

# Active sessions set (for tracking all active sessions per user)
USER_SESSIONS_PREFIX = "user:sessions:"


class SessionEncoder(json.JSONEncoder):
    """Custom JSON encoder for session data."""

    def default(self, obj):
        if isinstance(obj, Decimal):
            return float(obj)
        elif isinstance(obj, datetime):
            return obj.isoformat()
        return super().default(obj)


class RedisSessionManager:
    """
    Manages user sessions in Redis for cross-instance consistency.
    
    Features:
    - Store session data in Redis for shared access across instances
    - Track active sessions per user
    - Automatic session expiration
    - Session invalidation across all instances
    """

    def __init__(self):
        self.redis_client = redis.Redis(
            host=REDIS_HOST,
            port=REDIS_PORT,
            db=REDIS_DB,
            decode_responses=True
        )
        self.session_ttl = SESSION_TTL_MINUTES * 60  # Convert to seconds

    def create_session(self, user_id: int, email: str, user_data: Dict[str, Any]) -> str:
        """
        Create a new session for a user.
        
        Args:
            user_id: User's ID
            email: User's email
            user_data: Additional user data to store in session
            
        Returns:
            Session token (JWT or custom token)
        """
        # Generate session token (could be JWT or UUID)
        import uuid
        session_token = str(uuid.uuid4())
        session_key = f"{SESSION_PREFIX}{session_token}"
        
        # Prepare session data
        session_data = {
            "user_id": user_id,
            "email": email,
            "user_data": user_data,
            "created_at": datetime.utcnow().isoformat(),
            "last_accessed": datetime.utcnow().isoformat(),
            "instance_id": os.getenv("INSTANCE_ID", "default")
        }
        
        # Store session in Redis
        self.redis_client.setex(
            session_key,
            self.session_ttl,
            json.dumps(session_data, cls=SessionEncoder)
        )
        
        # Track session in user's active sessions set
        user_sessions_key = f"{USER_SESSIONS_PREFIX}{user_id}"
        self.redis_client.sadd(user_sessions_key, session_token)
        self.redis_client.expire(user_sessions_key, self.session_ttl)
        
        return session_token

    def get_session(self, session_token: str) -> Optional[Dict[str, Any]]:
        """
        Retrieve session data by token.
        
        Args:
            session_token: Session token
            
        Returns:
            Session data dict or None if not found
        """
        session_key = f"{SESSION_PREFIX}{session_token}"
        session_data = self.redis_client.get(session_key)
        
        if session_data is None:
            return None
        
        try:
            data = json.loads(session_data)
            
            # Refresh TTL on access if configured
            if SESSION_REFRESH_ON_ACCESS:
                data["last_accessed"] = datetime.utcnow().isoformat()
                self.redis_client.setex(
                    session_key,
                    self.session_ttl,
                    json.dumps(data, cls=SessionEncoder)
                )
            
            return data
        except (json.JSONDecodeError, TypeError):
            return None

    def update_session(self, session_token: str, data: Dict[str, Any]) -> bool:
        """
        Update session data.
        
        Args:
            session_token: Session token
            data: Data to update
            
        Returns:
            True if updated, False if session not found
        """
        session_key = f"{SESSION_PREFIX}{session_token}"
        existing_data = self.redis_client.get(session_key)
        
        if existing_data is None:
            return False
        
        try:
            session_data = json.loads(existing_data)
            session_data.update(data)
            session_data["last_accessed"] = datetime.utcnow().isoformat()
            
            self.redis_client.setex(
                session_key,
                self.session_ttl,
                json.dumps(session_data, cls=SessionEncoder)
            )
            return True
        except (json.JSONDecodeError, TypeError):
            return False

    def delete_session(self, session_token: str) -> bool:
        """
        Delete a session.
        
        Args:
            session_token: Session token
            
        Returns:
            True if deleted, False if not found
        """
        session_key = f"{SESSION_PREFIX}{session_token}"
        session_data = self.redis_client.get(session_key)
        
        if session_data is None:
            return False
        
        try:
            data = json.loads(session_data)
            user_id = data.get("user_id")
            
            # Remove from user's active sessions
            if user_id:
                user_sessions_key = f"{USER_SESSIONS_PREFIX}{user_id}"
                self.redis_client.srem(user_sessions_key, session_token)
            
            # Delete session
            self.redis_client.delete(session_key)
            return True
        except (json.JSONDecodeError, TypeError):
            self.redis_client.delete(session_key)
            return True

    def delete_all_user_sessions(self, user_id: int) -> int:
        """
        Delete all sessions for a user (force logout from all instances).
        
        Args:
            user_id: User's ID
            
        Returns:
            Number of sessions deleted
        """
        user_sessions_key = f"{USER_SESSIONS_PREFIX}{user_id}"
        session_tokens = self.redis_client.smembers(user_sessions_key)
        
        deleted_count = 0
        for token in session_tokens:
            session_key = f"{SESSION_PREFIX}{token}"
            if self.redis_client.delete(session_key):
                deleted_count += 1
        
        # Clear the user sessions set
        self.redis_client.delete(user_sessions_key)
        
        return deleted_count

    def get_user_active_sessions(self, user_id: int) -> list:
        """
        Get all active session tokens for a user.
        
        Args:
            user_id: User's ID
            
        Returns:
            List of session tokens
        """
        user_sessions_key = f"{USER_SESSIONS_PREFIX}{user_id}"
        return list(self.redis_client.smembers(user_sessions_key))

    def is_session_valid(self, session_token: str) -> bool:
        """
        Check if a session is valid.
        
        Args:
            session_token: Session token
            
        Returns:
            True if valid, False otherwise
        """
        session_key = f"{SESSION_PREFIX}{session_token}"
        return self.redis_client.exists(session_key) == 1

    def refresh_session(self, session_token: str) -> bool:
        """
        Refresh session TTL.
        
        Args:
            session_token: Session token
            
        Returns:
            True if refreshed, False if not found
        """
        session_key = f"{SESSION_PREFIX}{session_token}"
        session_data = self.redis_client.get(session_key)
        
        if session_data is None:
            return False
        
        try:
            data = json.loads(session_data)
            data["last_accessed"] = datetime.utcnow().isoformat()
            
            self.redis_client.setex(
                session_key,
                self.session_ttl,
                json.dumps(data, cls=SessionEncoder)
            )
            return True
        except (json.JSONDecodeError, TypeError):
            return False

    def get_session_count(self) -> int:
        """
        Get total number of active sessions.
        
        Returns:
            Number of active sessions
        """
        keys = self.redis_client.keys(f"{SESSION_PREFIX}*")
        return len(keys)

    def is_connected(self) -> bool:
        """Check if Redis is connected."""
        try:
            self.redis_client.ping()
            return True
        except redis.ConnectionError:
            return False


# Global session manager instance
session_manager = RedisSessionManager()
