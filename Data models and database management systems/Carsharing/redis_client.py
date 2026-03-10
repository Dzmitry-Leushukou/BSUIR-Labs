import redis
import os
import json
from typing import Optional, Any, List
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
    
    def is_connected(self) -> bool:
        try:
            self.redis_client.ping()
            return True
        except redis.ConnectionError:
            return False


redis_client = RedisClient()
