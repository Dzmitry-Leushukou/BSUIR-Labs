import redis
import os
from typing import Optional
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
