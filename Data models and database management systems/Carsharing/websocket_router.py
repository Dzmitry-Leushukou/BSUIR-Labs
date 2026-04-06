"""
WebSocket router для real-time уведомлений.
Позволяет клиентам подписываться на события через WebSocket.
"""

from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Depends, HTTPException
from typing import Dict, Set, Optional
import asyncio
import json
from datetime import datetime

from redis_client import (
    redis_client,
    pubsub_subscriber,
    CHANNEL_DATA_CHANGED,
    CHANNEL_USER_SESSIONS,
    CHANNEL_INSTANCE_EVENTS
)
from event_handler import event_handler
import uuid

router = APIRouter(prefix="/ws", tags=["WebSocket"])


class ConnectionManager:
    """
    Управляет WebSocket подключениями.
    """

    def __init__(self):
        # {user_id: {connection_id: websocket}}
        self.active_connections: Dict[int, Dict[str, WebSocket]] = {}
        # {connection_id: user_id}
        self.connection_to_user: Dict[str, int] = {}

    async def connect(self, websocket: WebSocket, user_id: int) -> str:
        """
        Подключить WebSocket для пользователя.
        Возвращает connection_id.
        """
        await websocket.accept()
        
        connection_id = str(uuid.uuid4())
        
        if user_id not in self.active_connections:
            self.active_connections[user_id] = {}
        
        self.active_connections[user_id][connection_id] = websocket
        self.connection_to_user[connection_id] = user_id
        
        return connection_id

    def disconnect(self, connection_id: str):
        """
        Отключить WebSocket.
        """
        if connection_id in self.connection_to_user:
            user_id = self.connection_to_user[connection_id]
            if user_id in self.active_connections:
                self.active_connections[user_id].pop(connection_id, None)
                if not self.active_connections[user_id]:
                    del self.active_connections[user_id]
            del self.connection_to_user[connection_id]

    async def send_personal_message(self, message: dict, user_id: int, connection_id: Optional[str] = None):
        """
        Отправить сообщение конкретному подключению или всем подключениям пользователя.
        """
        if user_id not in self.active_connections:
            return

        if connection_id:
            # Отправляем конкретному подключению
            websocket = self.active_connections[user_id].get(connection_id)
            if websocket:
                try:
                    await websocket.send_text(json.dumps(message, default=str))
                except Exception as e:
                    print(f"Failed to send message: {str(e)}")
        else:
            # Отправляем всем подключениям пользователя
            for conn_id, websocket in list(self.active_connections[user_id].items()):
                try:
                    await websocket.send_text(json.dumps(message, default=str))
                except Exception as e:
                    print(f"Failed to send message to {conn_id}: {str(e)}")

    async def broadcast(self, message: dict):
        """
        Отправить сообщение всем подключенным клиентам.
        """
        for user_id, connections in list(self.active_connections.items()):
            for connection_id, websocket in list(connections.items()):
                try:
                    await websocket.send_text(json.dumps(message, default=str))
                except Exception as e:
                    print(f"Failed to broadcast to {connection_id}: {str(e)}")


# Глобальный менеджер подключений
manager = ConnectionManager()


async def handle_data_changed(message: dict):
    """
    Обработчик событий изменения данных для WebSocket.
    """
    # Отправляем всем клиентам
    await manager.broadcast({
        "type": "data_changed",
        "entity_type": message.get("entity_type"),
        "entity_id": message.get("entity_id"),
        "action": message.get("action"),
        "timestamp": message.get("timestamp")
    })


async def handle_user_session_event(message: dict):
    """
    Обработчик событий сессий для WebSocket.
    """
    user_id = message.get("user_id")
    event_type = message.get("event_type")
    print(f"[WS HANDLER] Received session event: user_id={user_id}, event_type={event_type}")
    
    if user_id:
        print(f"[WS HANDLER] Sending to user {user_id}")
        await manager.send_personal_message({
            "type": "session_event",
            "event_type": message.get("event_type"),
            "session_id": message.get("session_id"),
            "timestamp": message.get("timestamp")
        }, user_id)
    else:
        print(f"[WS HANDLER] No user_id in message, broadcasting")
        await manager.broadcast({
            "type": "session_event",
            "event_type": event_type,
            "timestamp": message.get("timestamp")
        })


async def handle_instance_event(message: dict):
    """
    Обработчик событий инстансов для WebSocket.
    """
    # Отправляем всем клиентам
    await manager.broadcast({
        "type": "instance_event",
        "event_type": message.get("event_type"),
        "instance_id": message.get("instance_id"),
        "timestamp": message.get("timestamp")
    })


@router.on_event("startup")
async def websocket_startup():
    """
    Зарегистрировать обработчики событий для WebSocket.
    """
    event_handler.register_data_change_handler(handle_data_changed)
    event_handler.register_session_event_handler(handle_user_session_event)
    event_handler.register_instance_event_handler(handle_instance_event)


@router.websocket("/notifications/{user_id}")
async def websocket_notifications(websocket: WebSocket, user_id: int):
    """
    WebSocket endpoint для получения real-time уведомлений.
    
    Подключение: ws://host:port/ws/notifications/{user_id}
    """
    connection_id = await manager.connect(websocket, user_id)
    
    try:
        # Отправляем приветственное сообщение
        await websocket.send_text(json.dumps({
            "type": "connected",
            "connection_id": connection_id,
            "message": "Connected to notification stream",
            "timestamp": datetime.utcnow().isoformat()
        }))
        
        # Держим соединение открытым
        while True:
            # Получаем сообщения от клиента (например, ping/pong)
            data = await websocket.receive_text()
            
            # Обрабатываем команды от клиента
            try:
                message = json.loads(data)
                if message.get("type") == "ping":
                    await websocket.send_text(json.dumps({
                        "type": "pong",
                        "timestamp": datetime.utcnow().isoformat()
                    }))
                elif message.get("type") == "subscribe":
                    # Клиент хочет подписаться на определенные события
                    event_types = message.get("event_types", [])
                    await websocket.send_text(json.dumps({
                        "type": "subscribed",
                        "event_types": event_types,
                        "timestamp": datetime.utcnow().isoformat()
                    }))
            except json.JSONDecodeError:
                pass
                
    except WebSocketDisconnect:
        manager.disconnect(connection_id)
    except Exception as e:
        print(f"WebSocket error for user {user_id}: {str(e)}")
        manager.disconnect(connection_id)


@router.get("/ws/connections")
async def get_active_connections():
    """
    Получить статистику активных WebSocket подключений.
    """
    return {
        "total_users": len(manager.active_connections),
        "total_connections": len(manager.connection_to_user),
        "users": list(manager.active_connections.keys())
    }
