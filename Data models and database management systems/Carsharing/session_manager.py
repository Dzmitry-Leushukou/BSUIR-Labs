"""
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
