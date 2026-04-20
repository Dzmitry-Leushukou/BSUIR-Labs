import redis
import os
import json
from typing import Optional, Any, List, Callable, Dict
from decimal import Decimal
from datetime import datetime
from dotenv import load_dotenv

load_dotenv()

REDIS_HOST = os.getenv("REDIS_HOST", "localhost")
REDIS_PORT = int(os.getenv("REDIS_PORT", 6379))
REDIS_DB = int(os.getenv("REDIS_DB", 0))

BLACKLIST_PREFIX = "blacklist:"
FAILED_LOGIN_PREFIX = "failed_login:"
MAX_FAILED_ATTEMPTS = 3
BLACKLIST_TTL_MINUTES = 10
FAILED_LOGIN_TTL_MINUTES = 30

CACHE_PREFIX = "cache:"
ROLES_CACHE_TTL = 3600
USERS_CACHE_TTL = 1800
CARS_CACHE_TTL = 600
SESSIONS_CACHE_TTL = 1800
USER_CACHE_TTL = 300
CAR_CACHE_TTL = 300
DRIVER_LICENSES_CACHE_TTL = 3600

# Session configuration
SESSION_PREFIX = "session:"
SESSION_TTL_MINUTES = int(os.getenv("SESSION_TTL_MINUTES", "30"))
USER_SESSIONS_PREFIX = "user:sessions:"

# Pub/Sub channels
CHANNEL_DATA_CHANGES = "data:changes"
CHANNEL_CACHE_INVALIDATION = "cache:invalidation"
CHANNEL_SESSION_EVENTS = "session:events"
CHANNEL_USER_EVENTS = "user:events"

# Instance identification
INSTANCE_ID = os.getenv("INSTANCE_ID", "default")


class CacheEncoder(json.JSONEncoder):
    
    def default(self, obj):
        if isinstance(obj, Decimal):
            return float(obj)
        elif isinstance(obj, datetime):
            return obj.isoformat()
        return super().default(obj)

class RedisClient:
    
    def __init__(self):
        self.redis_client = redis.Redis(
            host=REDIS_HOST,
            port=REDIS_PORT,
            db=REDIS_DB,
            decode_responses=True
        )
    
    def check_blacklist(self, email: str) -> bool:
        key = f"{BLACKLIST_PREFIX}{email}"
        return self.redis_client.exists(key)
    
    def add_to_blacklist(self, email: str) -> None:
        key = f"{BLACKLIST_PREFIX}{email}"
        ttl_seconds = BLACKLIST_TTL_MINUTES * 60
        self.redis_client.setex(key, ttl_seconds, "blacklisted")
    
    def remove_from_blacklist(self, email: str) -> None:
        key = f"{BLACKLIST_PREFIX}{email}"
        self.redis_client.delete(key)
    
    def get_blacklist_ttl(self, email: str) -> int:
        key = f"{BLACKLIST_PREFIX}{email}"
        return self.redis_client.ttl(key)
    
    def increment_failed_login(self, email: str) -> int:
        key = f"{FAILED_LOGIN_PREFIX}{email}"
        count = self.redis_client.incr(key)
        
        if count == 1:
            ttl_seconds = FAILED_LOGIN_TTL_MINUTES * 60
            self.redis_client.expire(key, ttl_seconds)
        
        if count >= MAX_FAILED_ATTEMPTS:
            self.add_to_blacklist(email)
        
        return count
    
    def reset_failed_login(self, email: str) -> None:
        key = f"{FAILED_LOGIN_PREFIX}{email}"
        self.redis_client.delete(key)
    
    def get_failed_login_count(self, email: str) -> int:
        key = f"{FAILED_LOGIN_PREFIX}{email}"
        count = self.redis_client.get(key)
        return int(count) if count else 0
    
    def set_cache(self, key: str, value: Any, ttl: int = 3600) -> None:
        try:
            if isinstance(value, (dict, list)) or value is not None:
                serialized_value = json.dumps(value, cls=CacheEncoder)
            else:
                serialized_value = value
            
            cache_key = f"{CACHE_PREFIX}{key}"
            self.redis_client.setex(cache_key, ttl, serialized_value)
        except Exception as e:
            print(f"Cache set failed for key {key}: {str(e)}")
    
    def get_cache(self, key: str) -> Optional[Any]:
        try:
            cache_key = f"{CACHE_PREFIX}{key}"
            value = self.redis_client.get(cache_key)
            
            if value is None:
                return None
            
            try:
                return json.loads(value)
            except (json.JSONDecodeError, TypeError):
                return value
        except Exception as e:
            print(f"Cache get failed for key {key}: {str(e)}")
            return None
    
    def delete_cache(self, key: str) -> None:

        try:
            cache_key = f"{CACHE_PREFIX}{key}"
            self.redis_client.delete(cache_key)
        except Exception as e:
            print(f"Cache delete failed for key {key}: {str(e)}")
    
    def delete_cache_by_pattern(self, pattern: str) -> int:
        try:
            full_pattern = f"{CACHE_PREFIX}{pattern}"
            # Find all keys matching pattern
            keys = self.redis_client.keys(full_pattern)
            
            if keys:
                deleted_count = self.redis_client.delete(*keys)
                return deleted_count
            return 0
        except Exception as e:
            print(f"Cache delete pattern failed for pattern {pattern}: {str(e)}")
            return 0
    
    def invalidate_list_cache(self, entity_type: str) -> None:
        self.delete_cache_by_pattern(f"{entity_type}:list")
        self.delete_cache_by_pattern(f"{entity_type}:ids")

    # ==================== Session Management ====================

    def create_session(self, user_id: int, email: str, user_data: Dict[str, Any], session_token: str) -> None:
        """
        Create a new session in Redis.
        
        Args:
            user_id: User's ID
            email: User's email
            user_data: Additional user data to store
            session_token: Session token (JWT or custom)
        """
        session_key = f"{SESSION_PREFIX}{session_token}"
        session_ttl = SESSION_TTL_MINUTES * 60
        
        session_data = {
            "user_id": user_id,
            "email": email,
            "user_data": user_data,
            "created_at": datetime.utcnow().isoformat(),
            "last_accessed": datetime.utcnow().isoformat(),
            "instance_id": INSTANCE_ID
        }
        
        self.redis_client.setex(
            session_key,
            session_ttl,
            json.dumps(session_data, cls=CacheEncoder)
        )
        
        # Track session in user's active sessions set
        user_sessions_key = f"{USER_SESSIONS_PREFIX}{user_id}"
        self.redis_client.sadd(user_sessions_key, session_token)
        self.redis_client.expire(user_sessions_key, session_ttl)

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
            return json.loads(session_data)
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
            
            session_ttl = SESSION_TTL_MINUTES * 60
            self.redis_client.setex(
                session_key,
                session_ttl,
                json.dumps(session_data, cls=CacheEncoder)
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

    def get_user_active_sessions(self, user_id: int) -> List[str]:
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
            
            session_ttl = SESSION_TTL_MINUTES * 60
            self.redis_client.setex(
                session_key,
                session_ttl,
                json.dumps(data, cls=CacheEncoder)
            )
            return True
        except (json.JSONDecodeError, TypeError):
            return False

    # ==================== Pub/Sub Methods ====================

    def publish_data_change(
        self,
        entity_type: str,
        action: str,
        entity_id: Any,
        old_data: Optional[Dict] = None,
        new_data: Optional[Dict] = None,
        user_id: Optional[int] = None
    ) -> int:
        """
        Publish a data change notification.
        
        Args:
            entity_type: Type of entity (e.g., "users", "cars")
            action: Action performed (e.g., "create", "update", "delete")
            entity_id: ID of the entity
            old_data: Previous data (for updates)
            new_data: New data (for creates/updates)
            user_id: ID of user who performed the action
            
        Returns:
            Number of subscribers notified
        """
        payload = json.dumps({
            "entity_type": entity_type,
            "action": action,
            "entity_id": entity_id,
            "old_data": old_data,
            "new_data": new_data,
            "user_id": user_id,
            "instance_id": INSTANCE_ID,
            "timestamp": datetime.utcnow().isoformat()
        }, cls=CacheEncoder)
        return self.redis_client.publish(CHANNEL_DATA_CHANGES, payload)

    def publish_cache_invalidation(self, entity_type: str, key_pattern: Optional[str] = None) -> int:
        """
        Publish cache invalidation notification.
        
        Args:
            entity_type: Type of entity to invalidate
            key_pattern: Optional specific key pattern
            
        Returns:
            Number of subscribers
        """
        payload = json.dumps({
            "entity_type": entity_type,
            "key_pattern": key_pattern,
            "timestamp": datetime.utcnow().isoformat(),
            "instance_id": INSTANCE_ID
        }, cls=CacheEncoder)
        return self.redis_client.publish(CHANNEL_CACHE_INVALIDATION, payload)

    def publish_session_event(
        self,
        event_type: str,
        user_id: int,
        session_token: Optional[str] = None,
        data: Optional[Dict] = None
    ) -> int:
        """
        Publish session event notification.
        
        Args:
            event_type: Type of event (e.g., "login", "logout", "expired")
            user_id: User's ID
            session_token: Optional session token
            data: Optional additional data
            
        Returns:
            Number of subscribers
        """
        payload = json.dumps({
            "event_type": event_type,
            "user_id": user_id,
            "session_token": session_token,
            "data": data,
            "timestamp": datetime.utcnow().isoformat(),
            "instance_id": INSTANCE_ID
        }, cls=CacheEncoder)
        return self.redis_client.publish(CHANNEL_SESSION_EVENTS, payload)

    def publish_user_event(
        self,
        event_type: str,
        user_id: int,
        data: Optional[Dict] = None
    ) -> int:
        """
        Publish user event notification.
        
        Args:
            event_type: Type of event (e.g., "banned", "updated", "deleted")
            user_id: User's ID
            data: Optional additional data
            
        Returns:
            Number of subscribers
        """
        payload = json.dumps({
            "event_type": event_type,
            "user_id": user_id,
            "data": data,
            "timestamp": datetime.utcnow().isoformat(),
            "instance_id": INSTANCE_ID
        }, cls=CacheEncoder)
        return self.redis_client.publish(CHANNEL_USER_EVENTS, payload)

    def subscribe(self, channel: str, callback: Callable[[str], None]) -> None:
        """
        Subscribe to a channel with a callback.
        Note: For production use, consider using a separate connection for pub/sub.
        
        Args:
            channel: Channel name
            callback: Function to call on message received
        """
        # This is a simplified implementation
        # For full pub/sub, use the pubsub_manager module
        pass

    def is_connected(self) -> bool:
        try:
            self.redis_client.ping()
            return True
        except redis.ConnectionError:
            return False


redis_client = RedisClient()
