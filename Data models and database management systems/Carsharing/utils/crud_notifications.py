"""
Helper module for CRUD operations with pub/sub notifications.
Provides decorated functions for automatic cache invalidation and cross-instance notifications.
"""

from typing import Any, Callable, Optional, Dict
from functools import wraps
from fastapi import Request

from redis_client import redis_client
from middleware.session_middleware import notify_data_change, notify_user_event


def notify_on_create(entity_type: str, id_field: str = "id"):
    """
    Decorator to notify about entity creation.
    
    Args:
        entity_type: Type of entity (e.g., "cars", "rentals")
        id_field: Field name containing the entity ID
    """
    def decorator(func: Callable):
        @wraps(func)
        def wrapper(*args, **kwargs):
            result = func(*args, **kwargs)
            
            if result:
                entity_id = result.get(id_field) if isinstance(result, dict) else getattr(result, id_field, None)
                
                # Get current user from kwargs or args (if available)
                current_user = kwargs.get('current_user')
                user_id = current_user.get('id') if current_user else None
                
                # Notify about creation
                notify_data_change(
                    entity_type=entity_type,
                    action="create",
                    entity_id=entity_id,
                    new_data=result if isinstance(result, dict) else None,
                    user_id=user_id
                )
            
            return result
        return wrapper
    return decorator


def notify_on_update(entity_type: str, id_field: str = "id"):
    """
    Decorator to notify about entity update.
    
    Args:
        entity_type: Type of entity
        id_field: Field name containing the entity ID
    """
    def decorator(func: Callable):
        @wraps(func)
        def wrapper(*args, old_data: Optional[Dict] = None, **kwargs):
            result = func(*args, **kwargs)
            
            if result:
                entity_id = result.get(id_field) if isinstance(result, dict) else getattr(result, id_field, None)
                
                # Get current user from kwargs or args
                current_user = kwargs.get('current_user')
                user_id = current_user.get('id') if current_user else None
                
                # Notify about update
                notify_data_change(
                    entity_type=entity_type,
                    action="update",
                    entity_id=entity_id,
                    old_data=old_data,
                    new_data=result if isinstance(result, dict) else None,
                    user_id=user_id
                )
            
            return result
        return wrapper
    return decorator


def notify_on_delete(entity_type: str, id_field: str = "id"):
    """
    Decorator to notify about entity deletion.
    
    Args:
        entity_type: Type of entity
        id_field: Field name containing the entity ID
    """
    def decorator(func: Callable):
        @wraps(func)
        def wrapper(*args, entity_id: Optional[Any] = None, **kwargs):
            # Get old data before deletion if not provided
            if entity_id is None and args:
                entity_id = args[0] if args else None
            
            # Get current user from kwargs or args
            current_user = kwargs.get('current_user')
            user_id = current_user.get('id') if current_user else None
            
            result = func(*args, **kwargs)
            
            # Notify about deletion
            notify_data_change(
                entity_type=entity_type,
                action="delete",
                entity_id=entity_id,
                user_id=user_id
            )
            
            return result
        return wrapper
    return decorator


def invalidate_cache_on_change(entity_type: str):
    """
    Decorator to invalidate cache when entity changes.
    
    Args:
        entity_type: Type of entity to invalidate
    """
    def decorator(func: Callable):
        @wraps(func)
        def wrapper(*args, **kwargs):
            result = func(*args, **kwargs)
            
            # Invalidate list cache
            redis_client.invalidate_list_cache(entity_type)
            
            # Publish cache invalidation event
            redis_client.publish_cache_invalidation(entity_type)
            
            return result
        return wrapper
    return decorator


def with_session_tracking(func: Callable) -> Callable:
    """
    Decorator to track session activity.
    Refreshes session TTL on each authenticated request.
    """
    @wraps(func)
    def wrapper(request: Request, *args, **kwargs):
        session_token = request.headers.get('X-Session-Token')
        
        if session_token:
            # Refresh session TTL
            redis_client.refresh_session(session_token)
        
        return func(request, *args, **kwargs)
    return wrapper


def get_entity_from_request(request: Request) -> Optional[Dict]:
    """
    Get current user from request state.
    
    Args:
        request: FastAPI request object
        
    Returns:
        User data dict or None
    """
    if hasattr(request.state, 'current_user'):
        return request.state.current_user
    return None


def get_session_from_request(request: Request) -> Optional[Dict]:
    """
    Get session from request state.
    
    Args:
        request: FastAPI request object
        
    Returns:
        Session data dict or None
    """
    if hasattr(request.state, 'session'):
        return request.state.session
    return None
