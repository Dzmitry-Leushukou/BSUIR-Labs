"""
<<<<<<< HEAD
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
=======
Менеджер сессий пользователей в Redis.
Обеспечивает синхронизацию состояния между несколькими инстансами приложения.
"""

import uuid
import os
from typing import Optional, Dict, List
from datetime import datetime
from redis_client import redis_client, SESSION_TTL
import json


class SessionManager:
    """
    Управляет сессиями пользователей в Redis.
    Поддерживает синхронизацию между несколькими инстансами приложения.
    """

    def __init__(self):
        self.instance_id = os.getenv("INSTANCE_ID", f"instance-{uuid.uuid4().hex[:8]}")

    def create_session(self, user_id: int, user_data: dict) -> str:
        """
        Создать новую сессию для пользователя.
        Возвращает session_id (токен).
        """
        session_id = str(uuid.uuid4())
        
        session_data = {
            "user_id": user_id,
            "email": user_data.get("email"),
            "created_at": datetime.utcnow().isoformat(),
            "last_activity": datetime.utcnow().isoformat(),
            "instance_id": self.instance_id,
            "ip_address": user_data.get("ip_address"),
            "user_agent": user_data.get("user_agent")
        }

        success = redis_client.create_session(session_id, user_id, session_data, ttl=SESSION_TTL)
        
        if success:
            # Публикуем событие о создании сессии
            redis_client.publish_user_session_event(
                user_id=user_id,
                event_type="session_created",
                session_id=session_id,
                instance_id=self.instance_id
            )
            return session_id
        
        raise Exception("Failed to create session in Redis")

    def get_session(self, session_id: str) -> Optional[dict]:
        """
        Получить сессию по ID.
        """
        return redis_client.get_session(session_id)

    def validate_session(self, session_id: str) -> Optional[dict]:
        """
        Проверить валидность сессии.
        Возвращает данные сессии или None.
        """
        session = self.get_session(session_id)
        
        if session is None:
            return None
        
        # Обновляем время последней активности
        session['last_activity'] = datetime.utcnow().isoformat()
        redis_client.update_session(session_id, {"last_activity": session['last_activity']})
        
        return session

    def update_session(self, session_id: str, user_id: int, updates: dict) -> bool:
        """
        Обновить данные сессии.
        """
        success = redis_client.update_session(session_id, updates)
        
        if success:
            # Публикуем событие об обновлении сессии
            redis_client.publish_user_session_event(
                user_id=user_id,
                event_type="session_updated",
                session_id=session_id,
                instance_id=self.instance_id
            )
        
        return success

    def delete_session(self, session_id: str, user_id: int) -> bool:
        """
        Удалить сессию.
        """
        success = redis_client.delete_session(session_id, user_id)
        
        if success:
            # Публикуем событие об удалении сессии
            redis_client.publish_user_session_event(
                user_id=user_id,
                event_type="session_deleted",
                session_id=session_id,
                instance_id=self.instance_id
            )
        
        return success

    def get_user_sessions(self, user_id: int) -> List[dict]:
        """
        Получить все активные сессии пользователя.
        """
        session_ids = redis_client.get_user_sessions(user_id)
        sessions = []
        
        for session_id in session_ids:
            session = self.get_session(session_id)
            if session:
                sessions.append(session)
        
        return sessions

    def delete_all_user_sessions(self, user_id: int) -> int:
        """
        Удалить все сессии пользователя.
        """
        count = redis_client.delete_user_sessions(user_id)
        
        if count > 0:
            # Публикуем событие
            redis_client.publish_user_session_event(
                user_id=user_id,
                event_type="all_sessions_deleted",
                instance_id=self.instance_id
            )
        
        return count

    def invalidate_session_by_instance(self, instance_id: str) -> int:
        """
        Инвалидировать все сессии конкретного инстанса.
        Используется при перезапуске инстанса.
        """
        # Получаем все сессии и проверяем их instance_id
        # Это операция expensive, поэтому используем с осторожностью
        invalidated_count = 0
        
        # В реальном приложении лучше использовать отдельные ключи для индексации по instance_id
        # Для упрощения пропускаем эту операцию
        return invalidated_count

    def refresh_session_ttl(self, session_id: str, user_id: int) -> bool:
        """
        Обновить TTL сессии (продлить жизнь).
        """
        try:
            key = f"session:{session_id}"
            return redis_client.redis_client.expire(key, SESSION_TTL)
        except Exception as e:
            print(f"Failed to refresh session TTL: {str(e)}")
            return False

    def get_active_sessions_count(self) -> dict:
        """
        Получить статистику по активным сессиям.
        """
        try:
            # Получаем все ключи активных сессий
            pattern = f"active_sessions:*"
            keys = redis_client.redis_client.keys(pattern)
            
            total_users = len(keys)
            total_sessions = 0
            
            for key in keys:
                count = redis_client.redis_client.scard(key)
                total_sessions += count
            
            return {
                "total_users": total_users,
                "total_sessions": total_sessions,
                "instance_id": self.instance_id
            }
        except Exception as e:
            print(f"Failed to get active sessions count: {str(e)}")
            return {"total_users": 0, "total_sessions": 0, "instance_id": self.instance_id}


# Глобальный экземпляр
session_manager = SessionManager()
>>>>>>> be6f42effb671486e8433257025662a233e281e2
