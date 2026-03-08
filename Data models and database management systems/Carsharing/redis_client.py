import redis
import os
import json
from typing import Optional, Any, List
from decimal import Decimal
from datetime import datetime
from dotenv import load_dotenv

load_dotenv()

# Redis configuration
REDIS_HOST = os.getenv("REDIS_HOST", "localhost")
REDIS_PORT = int(os.getenv("REDIS_PORT", 6379))
REDIS_DB = int(os.getenv("REDIS_DB", 0))

# Blacklist configuration
BLACKLIST_PREFIX = "blacklist:"
FAILED_LOGIN_PREFIX = "failed_login:"
MAX_FAILED_ATTEMPTS = 3
BLACKLIST_TTL_MINUTES = 10
FAILED_LOGIN_TTL_MINUTES = 30  # Failed login counter expires after 30 minutes

# Cache configuration with TTL (Time To Live) in seconds
CACHE_PREFIX = "cache:"
ROLES_CACHE_TTL = 3600  # 1 hour - справочник, статичные данные
USERS_CACHE_TTL = 1800  # 30 minutes - список пользователей
CARS_CACHE_TTL = 600    # 10 minutes - каталог активных автомобилей (часто меняется)
SESSIONS_CACHE_TTL = 1800  # 30 minutes - сессионные данные
USER_CACHE_TTL = 300    # 5 minutes - отдельный пользователь
CAR_CACHE_TTL = 300     # 5 minutes - отдельный автомобиль
DRIVER_LICENSES_CACHE_TTL = 3600  # 1 hour - справочник


class CacheEncoder(json.JSONEncoder):
    """Custom JSON encoder for handling Decimal and datetime objects from PostgreSQL."""
    
    def default(self, obj):
        if isinstance(obj, Decimal):
            # Convert Decimal to float
            return float(obj)
        elif isinstance(obj, datetime):
            # Convert datetime to ISO format string
            return obj.isoformat()
        return super().default(obj)



class RedisClient:
    """Redis client for managing user blacklist and failed login attempts."""
    
    def __init__(self):
        self.redis_client = redis.Redis(
            host=REDIS_HOST,
            port=REDIS_PORT,
            db=REDIS_DB,
            decode_responses=True
        )
    
    def check_blacklist(self, email: str) -> bool:
        """
        Check if a user email is in the blacklist.
        
        Args:
            email: User email to check
            
        Returns:
            True if user is blacklisted, False otherwise
        """
        key = f"{BLACKLIST_PREFIX}{email}"
        return self.redis_client.exists(key)
    
    def add_to_blacklist(self, email: str) -> None:
        """
        Add a user email to the blacklist with TTL.
        
        Args:
            email: User email to blacklist
        """
        key = f"{BLACKLIST_PREFIX}{email}"
        ttl_seconds = BLACKLIST_TTL_MINUTES * 60
        self.redis_client.setex(key, ttl_seconds, "blacklisted")
    
    def remove_from_blacklist(self, email: str) -> None:
        """
        Remove a user email from the blacklist.
        
        Args:
            email: User email to remove from blacklist
        """
        key = f"{BLACKLIST_PREFIX}{email}"
        self.redis_client.delete(key)
    
    def get_blacklist_ttl(self, email: str) -> int:
        """
        Get remaining TTL for a blacklisted user.
        
        Args:
            email: User email to check
            
        Returns:
            Remaining TTL in seconds, 0 if not blacklisted
        """
        key = f"{BLACKLIST_PREFIX}{email}"
        return self.redis_client.ttl(key)
    
    def increment_failed_login(self, email: str) -> int:
        """
        Increment failed login counter for a user.
        
        Args:
            email: User email
            
        Returns:
            Current failed login count
        """
        key = f"{FAILED_LOGIN_PREFIX}{email}"
        count = self.redis_client.incr(key)
        
        # Set TTL on first increment
        if count == 1:
            ttl_seconds = FAILED_LOGIN_TTL_MINUTES * 60
            self.redis_client.expire(key, ttl_seconds)
        
        # If max attempts reached, add to blacklist
        if count >= MAX_FAILED_ATTEMPTS:
            self.add_to_blacklist(email)
        
        return count
    
    def reset_failed_login(self, email: str) -> None:
        """
        Reset failed login counter for a user (called on successful login).
        
        Args:
            email: User email
        """
        key = f"{FAILED_LOGIN_PREFIX}{email}"
        self.redis_client.delete(key)
    
    def get_failed_login_count(self, email: str) -> int:
        """
        Get current failed login count for a user.
        
        Args:
            email: User email
            
        Returns:
            Failed login count
        """
        key = f"{FAILED_LOGIN_PREFIX}{email}"
        count = self.redis_client.get(key)
        return int(count) if count else 0
    
    # =========================================================================
    # Cache management methods for general caching
    # =========================================================================
    
    def set_cache(self, key: str, value: Any, ttl: int = 3600) -> None:
        """
        Set a value in cache with TTL.
        
        Args:
            key: Cache key
            value: Value to cache (will be JSON serialized if dict/list)
            ttl: Time to live in seconds (default 1 hour)
        """
        try:
            # Serialize to JSON using custom encoder that handles Decimal, datetime, etc.
            if isinstance(value, (dict, list)) or value is not None:
                serialized_value = json.dumps(value, cls=CacheEncoder)
            else:
                serialized_value = value
            
            cache_key = f"{CACHE_PREFIX}{key}"
            self.redis_client.setex(cache_key, ttl, serialized_value)
        except Exception as e:
            # If caching fails, log but don't raise - don't break the app
            print(f"Cache set failed for key {key}: {str(e)}")
    
    def get_cache(self, key: str) -> Optional[Any]:
        """
        Get a value from cache.
        
        Args:
            key: Cache key
            
        Returns:
            Cached value (deserialized if JSON) or None if not found
        """
        try:
            cache_key = f"{CACHE_PREFIX}{key}"
            value = self.redis_client.get(cache_key)
            
            if value is None:
                return None
            
            # Try to deserialize JSON
            try:
                return json.loads(value)
            except (json.JSONDecodeError, TypeError):
                # Return as-is if not JSON
                return value
        except Exception as e:
            print(f"Cache get failed for key {key}: {str(e)}")
            return None
    
    def delete_cache(self, key: str) -> None:
        """
        Delete a value from cache.
        
        Args:
            key: Cache key to delete
        """
        try:
            cache_key = f"{CACHE_PREFIX}{key}"
            self.redis_client.delete(cache_key)
        except Exception as e:
            print(f"Cache delete failed for key {key}: {str(e)}")
    
    def delete_cache_by_pattern(self, pattern: str) -> int:
        """
        Delete cache entries matching a pattern (useful for invalidating related caches).
        
        Args:
            pattern: Pattern to match (e.g., "users:*" will match cache:users:* and cache:users:list)
            
        Returns:
            Number of keys deleted
        """
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
        """
        Invalidate list cache for a specific entity type.
        
        Args:
            entity_type: Type of entity (e.g., 'roles', 'users', 'cars')
        """
        self.delete_cache_by_pattern(f"{entity_type}:list")
        self.delete_cache_by_pattern(f"{entity_type}:ids")
    
    def is_connected(self) -> bool:
        """
        Check if Redis connection is available.
        
        Returns:
            True if connected, False otherwise
        """
        try:
            self.redis_client.ping()
            return True
        except redis.ConnectionError:
            return False


# Global Redis client instance
redis_client = RedisClient()
