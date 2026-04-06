import redis
import redis.asyncio as aioredis
import os
import json
from typing import Optional, Any, List, Callable, Dict
from decimal import Decimal
from datetime import datetime
from dotenv import load_dotenv
import asyncio
import threading

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
SESSION_PREFIX = "session:"
ACTIVE_SESSIONS_PREFIX = "active_sessions:"
INSTANCE_PREFIX = "instance:"
ROLES_CACHE_TTL = 3600
USERS_CACHE_TTL = 1800
CARS_CACHE_TTL = 600
SESSIONS_CACHE_TTL = 1800
USER_CACHE_TTL = 300
CAR_CACHE_TTL = 300
DRIVER_LICENSES_CACHE_TTL = 3600
SESSION_TTL = 3600  # 1 час для сессий

# Pub/Sub каналы
CHANNEL_DATA_CHANGED = "data:changed"
CHANNEL_USER_SESSIONS = "user:sessions"
CHANNEL_INSTANCE_EVENTS = "instance:events"


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

    # ==========================================
    # Session Management в Redis
    # ==========================================

    def create_session(self, session_id: str, user_id: int, session_data: dict, ttl: int = SESSION_TTL) -> bool:
        """
        Создать сессию в Redis.
        session_id: уникальный ID сессии (UUID или токен)
        """
        try:
            key = f"{SESSION_PREFIX}{session_id}"
            # Добавляем user_id и данные сессии
            session_data['user_id'] = user_id
            session_data['session_id'] = session_id
            serialized = json.dumps(session_data, cls=CacheEncoder)
            pipe = self.redis_client.pipeline()
            pipe.setex(key, ttl, serialized)
            # Добавляем сессию в множество активных сессий пользователя
            pipe.sadd(f"{ACTIVE_SESSIONS_PREFIX}{user_id}", session_id)
            pipe.expire(f"{ACTIVE_SESSIONS_PREFIX}{user_id}", ttl)
            pipe.execute()
            return True
        except Exception as e:
            print(f"Session create failed for {session_id}: {str(e)}")
            return False

    def get_session(self, session_id: str) -> Optional[dict]:
        """
        Получить сессию из Redis.
        """
        try:
            key = f"{SESSION_PREFIX}{session_id}"
            data = self.redis_client.get(key)
            if data is None:
                return None
            return json.loads(data)
        except Exception as e:
            print(f"Session get failed for {session_id}: {str(e)}")
            return None

    def update_session(self, session_id: str, session_data: dict, ttl: int = SESSION_TTL) -> bool:
        """
        Обновить сессию в Redis.
        """
        try:
            key = f"{SESSION_PREFIX}{session_id}"
            # Получаем текущие данные
            current_data = self.get_session(session_id)
            if current_data is None:
                return False
            # Обновляем данные
            current_data.update(session_data)
            serialized = json.dumps(current_data, cls=CacheEncoder)
            self.redis_client.setex(key, ttl, serialized)
            return True
        except Exception as e:
            print(f"Session update failed for {session_id}: {str(e)}")
            return False

    def delete_session(self, session_id: str, user_id: int) -> bool:
        """
        Удалить сессию из Redis.
        """
        try:
            key = f"{SESSION_PREFIX}{session_id}"
            pipe = self.redis_client.pipeline()
            pipe.delete(key)
            pipe.srem(f"{ACTIVE_SESSIONS_PREFIX}{user_id}", session_id)
            pipe.execute()
            return True
        except Exception as e:
            print(f"Session delete failed for {session_id}: {str(e)}")
            return False

    def get_user_sessions(self, user_id: int) -> List[str]:
        """
        Получить все активные сессии пользователя.
        """
        try:
            key = f"{ACTIVE_SESSIONS_PREFIX}{user_id}"
            sessions = self.redis_client.smembers(key)
            return list(sessions) if sessions else []
        except Exception as e:
            print(f"Get user sessions failed for {user_id}: {str(e)}")
            return []

    def delete_user_sessions(self, user_id: int) -> int:
        """
        Удалить все сессии пользователя.
        """
        try:
            sessions = self.get_user_sessions(user_id)
            if not sessions:
                return 0
            pipe = self.redis_client.pipeline()
            for session_id in sessions:
                pipe.delete(f"{SESSION_PREFIX}{session_id}")
            pipe.delete(f"{ACTIVE_SESSIONS_PREFIX}{user_id}")
            pipe.execute()
            return len(sessions)
        except Exception as e:
            print(f"Delete user sessions failed for {user_id}: {str(e)}")
            return 0

    # ==========================================
    # Pub/Sub функциональность
    # ==========================================

    def publish(self, channel: str, message: dict) -> int:
        """
        Опубликовать сообщение в канал.
        """
        try:
            serialized = json.dumps(message, cls=CacheEncoder)
            return self.redis_client.publish(channel, serialized)
        except Exception as e:
            print(f"Publish failed for channel {channel}: {str(e)}")
            return 0

    def publish_data_changed(self, entity_type: str, entity_id: Optional[int] = None, 
                            action: str = "updated", instance_id: Optional[str] = None) -> int:
        """
        Опубликовать уведомление об изменении данных.
        """
        message = {
            "entity_type": entity_type,
            "entity_id": entity_id,
            "action": action,
            "instance_id": instance_id,
            "timestamp": datetime.utcnow().isoformat()
        }
        return self.publish(CHANNEL_DATA_CHANGED, message)

    def publish_user_session_event(self, user_id: int, event_type: str, 
                                   session_id: Optional[str] = None, instance_id: Optional[str] = None) -> int:
        """
        Опубликовать событие о сессии пользователя.
        """
        message = {
            "user_id": user_id,
            "event_type": event_type,
            "session_id": session_id,
            "instance_id": instance_id,
            "timestamp": datetime.utcnow().isoformat()
        }
        print(f"[REDIS PUBSUB] Publishing user session event: {event_type} for user {user_id}")
        return self.publish(CHANNEL_USER_SESSIONS, message)

    def publish_instance_event(self, instance_id: str, event_type: str, 
                               data: Optional[dict] = None) -> int:
        """
        Опубликовать событие от инстанса.
        """
        message = {
            "instance_id": instance_id,
            "event_type": event_type,
            "data": data or {},
            "timestamp": datetime.utcnow().isoformat()
        }
        return self.publish(CHANNEL_INSTANCE_EVENTS, message)


redis_client = RedisClient()


# ==========================================
# Async Pub/Sub Subscriber
# ==========================================

class RedisPubSubSubscriber:
    """
    Асинхронный клиент для подписки на каналы Redis.
    Используется для получения уведомлений об изменениях данных.
    """

    def __init__(self):
        self.async_redis: Optional[aioredis.Redis] = None
        self.pubsub: Optional[aioredis.client.PubSub] = None
        self.subscribers: Dict[str, List[Callable]] = {}
        self.running = False
        self.task: Optional[asyncio.Task] = None

    async def connect(self):
        """
        Подключиться к Redis с асинхронным клиентом.
        """
        if self.async_redis is None:
            self.async_redis = aioredis.Redis(
                host=REDIS_HOST,
                port=REDIS_PORT,
                db=REDIS_DB,
                decode_responses=True
            )
            self.pubsub = self.async_redis.pubsub()

    async def subscribe(self, channel: str, callback: Callable[[dict], None]):
        """
        Подписаться на канал с callback функцией.
        """
        if self.async_redis is None:
            await self.connect()

        if channel not in self.subscribers:
            await self.pubsub.subscribe(channel)
            self.subscribers[channel] = []

        self.subscribers[channel].append(callback)

    async def unsubscribe(self, channel: str, callback: Optional[Callable] = None):
        """
        Отписаться от канала.
        """
        if channel in self.subscribers:
            if callback:
                self.subscribers[channel].remove(callback)
                if not self.subscribers[channel]:
                    await self.pubsub.unsubscribe(channel)
                    del self.subscribers[channel]
            else:
                await self.pubsub.unsubscribe(channel)
                del self.subscribers[channel]

    async def start_listening(self):
        """
        Начать прослушивание каналов.
        """
        if self.pubsub is None:
            await self.connect()

        self.running = True
        while self.running:
            try:
                message = await self.pubsub.get_message(ignore_subscribe_messages=True, timeout=1.0)
                if message and message['type'] == 'message':
                    channel = message['channel']
                    if channel in self.subscribers:
                        try:
                            data = json.loads(message['data'])
                            for callback in self.subscribers[channel]:
                                # Вызываем callback асинхронно или синхронно
                                if asyncio.iscoroutinefunction(callback):
                                    asyncio.create_task(callback(data))
                                else:
                                    callback(data)
                        except json.JSONDecodeError:
                            print(f"Failed to parse message: {message['data']}")
            except Exception as e:
                print(f"Pub/Sub listener error: {str(e)}")
                await asyncio.sleep(1)

    def stop_listening(self):
        """
        Остановить прослушивание.
        """
        self.running = False
        if self.task:
            self.task.cancel()

    async def cleanup(self):
        """
        Очистить ресурсы.
        """
        self.stop_listening()
        if self.pubsub:
            await self.pubsub.unsubscribe()
            await self.pubsub.close()
        if self.async_redis:
            await self.async_redis.close()


# Глобальный экземпляр подписчика
pubsub_subscriber = RedisPubSubSubscriber()
