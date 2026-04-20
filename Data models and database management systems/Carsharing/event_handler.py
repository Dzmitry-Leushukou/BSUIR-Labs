"""
Обработчик Pub/Sub событий для синхронизации между инстансами.
Обрабатывает уведомления об изменениях данных и сессиях.
"""

import asyncio
from typing import Callable, Dict, List
from redis_client import (
    redis_client, 
    pubsub_subscriber,
    CHANNEL_DATA_CHANGED, 
    CHANNEL_USER_SESSIONS,
    CHANNEL_INSTANCE_EVENTS
)


class EventHandler:
    """
    Централизованный обработчик событий от других инстансов.
    """

    def __init__(self):
        self._data_change_handlers: List[Callable] = []
        self._session_event_handlers: List[Callable] = []
        self._instance_event_handlers: List[Callable] = []

    async def on_data_changed(self, message: dict):
        """
        Обработчик событий изменения данных.
        Вызывается при изменении любой сущности в системе.
        """
        entity_type = message.get("entity_type")
        entity_id = message.get("entity_id")
        action = message.get("action")
        instance_id = message.get("instance_id")
        
        print(f"[EVENT] Data changed: {entity_type}#{entity_id} - {action} from {instance_id}")
        
        # Инвалидируем кэш для этой сущности
        await self._invalidate_entity_cache(entity_type, entity_id)
        
        # Вызываем зарегистрированные обработчики
        for handler in self._data_change_handlers:
            try:
                if asyncio.iscoroutinefunction(handler):
                    await handler(message)
                else:
                    handler(message)
            except Exception as e:
                print(f"Error in data change handler: {str(e)}")

    async def on_user_session_event(self, message: dict):
        """
        Обработчик событий сессий пользователей.
        """
        user_id = message.get("user_id")
        event_type = message.get("event_type")
        instance_id = message.get("instance_id")
        
        print(f"[EVENT HANDLER] User session event: user {user_id} - {event_type} from {instance_id}")
        
        # Вызываем зарегистрированные обработчики
        for handler in self._session_event_handlers:
            try:
                if asyncio.iscoroutinefunction(handler):
                    await handler(message)
                else:
                    handler(message)
            except Exception as e:
                print(f"Error in session event handler: {str(e)}")

    async def on_instance_event(self, message: dict):
        """
        Обработчик событий от инстансов.
        """
        instance_id = message.get("instance_id")
        event_type = message.get("event_type")
        
        print(f"[EVENT] Instance event: {instance_id} - {event_type}")
        
        # Вызываем зарегистрированные обработчики
        for handler in self._instance_event_handlers:
            try:
                if asyncio.iscoroutinefunction(handler):
                    await handler(message)
                else:
                    handler(message)
            except Exception as e:
                print(f"Error in instance event handler: {str(e)}")

    async def _invalidate_entity_cache(self, entity_type: str, entity_id: int = None):
        """
        Инвалидировать кэш при изменении данных.
        """
        if entity_id:
            # Инвалидируем кэш конкретной сущности
            redis_client.delete_cache(f"{entity_type}:id:{entity_id}")
        
        # Инвалидируем кэш списков
        redis_client.invalidate_list_cache(entity_type)

    def register_data_change_handler(self, handler: Callable):
        """
        Зарегистрировать обработчик изменений данных.
        """
        self._data_change_handlers.append(handler)

    def register_session_event_handler(self, handler: Callable):
        """
        Зарегистрировать обработчик событий сессий.
        """
        self._session_event_handlers.append(handler)

    def register_instance_event_handler(self, handler: Callable):
        """
        Зарегистрировать обработчик событий инстансов.
        """
        self._instance_event_handlers.append(handler)

    async def start(self):
        """
        Запустить прослушивание Pub/Sub каналов.
        """
        # Подписываемся на каналы
        await pubsub_subscriber.subscribe(CHANNEL_DATA_CHANGED, self.on_data_changed)
        await pubsub_subscriber.subscribe(CHANNEL_USER_SESSIONS, self.on_user_session_event)
        await pubsub_subscriber.subscribe(CHANNEL_INSTANCE_EVENTS, self.on_instance_event)
        
        # Запускаем прослушивание в фоновой задаче
        pubsub_subscriber.task = asyncio.create_task(pubsub_subscriber.start_listening())
        print(f"[EVENT HANDLER] Started listening to Pub/Sub channels")

    async def stop(self):
        """
        Остановить прослушивание.
        """
        pubsub_subscriber.stop_listening()
        await pubsub_subscriber.cleanup()
        print(f"[EVENT HANDLER] Stopped listening to Pub/Sub channels")


# Глобальный экземпляр
event_handler = EventHandler()
