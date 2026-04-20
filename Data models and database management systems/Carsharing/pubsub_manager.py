"""
Redis Pub/Sub manager for cross-instance data synchronization.
Provides real-time notifications for data changes across multiple application instances.
"""

import redis
import json
import os
import asyncio
import threading
from typing import Optional, Callable, Dict, Any, List
from datetime import datetime
from decimal import Decimal
from dotenv import load_dotenv

load_dotenv()

REDIS_HOST = os.getenv("REDIS_HOST", "localhost")
REDIS_PORT = int(os.getenv("REDIS_PORT", 6379))
REDIS_DB = int(os.getenv("REDIS_DB", 0))

# Pub/Sub channels
CHANNEL_DATA_CHANGES = "data:changes"
CHANNEL_CACHE_INVALIDATION = "cache:invalidation"
CHANNEL_SESSION_EVENTS = "session:events"
CHANNEL_USER_EVENTS = "user:events"

# Instance identification
INSTANCE_ID = os.getenv("INSTANCE_ID", "default")


class PubSubEncoder(json.JSONEncoder):
    """Custom JSON encoder for pub/sub messages."""

    def default(self, obj):
        if isinstance(obj, Decimal):
            return float(obj)
        elif isinstance(obj, datetime):
            return obj.isoformat()
        return super().default(obj)


class DataChangeMessage:
    """Represents a data change notification message."""

    def __init__(
        self,
        entity_type: str,
        action: str,
        entity_id: Any,
        old_data: Optional[Dict] = None,
        new_data: Optional[Dict] = None,
        user_id: Optional[int] = None,
        instance_id: str = INSTANCE_ID
    ):
        self.entity_type = entity_type  # e.g., "users", "cars", "rentals"
        self.action = action  # e.g., "create", "update", "delete"
        self.entity_id = entity_id
        self.old_data = old_data
        self.new_data = new_data
        self.user_id = user_id
        self.instance_id = instance_id
        self.timestamp = datetime.utcnow().isoformat()

    def to_dict(self) -> Dict[str, Any]:
        return {
            "entity_type": self.entity_type,
            "action": self.action,
            "entity_id": self.entity_id,
            "old_data": self.old_data,
            "new_data": self.new_data,
            "user_id": self.user_id,
            "instance_id": self.instance_id,
            "timestamp": self.timestamp
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "DataChangeMessage":
        msg = cls(
            entity_type=data["entity_type"],
            action=data["action"],
            entity_id=data["entity_id"],
            old_data=data.get("old_data"),
            new_data=data.get("new_data"),
            user_id=data.get("user_id"),
            instance_id=data.get("instance_id", "unknown")
        )
        msg.timestamp = data.get("timestamp", datetime.utcnow().isoformat())
        return msg


class RedisPubSubManager:
    """
    Manages Redis Pub/Sub for cross-instance communication.
    
    Features:
    - Publish data change notifications
    - Subscribe to data changes
    - Cache invalidation across instances
    - Session event broadcasting
    - User event broadcasting
    """

    def __init__(self):
        self.redis_client = redis.Redis(
            host=REDIS_HOST,
            port=REDIS_PORT,
            db=REDIS_DB,
            decode_responses=True
        )
        self.pubsub = None
        self.subscriptions: Dict[str, List[Callable]] = {}
        self._running = False
        self._thread = None
        self._message_handlers: Dict[str, List[Callable]] = {}

    def publish_data_change(self, message: DataChangeMessage) -> int:
        """
        Publish a data change notification.
        
        Args:
            message: DataChangeMessage object
            
        Returns:
            Number of subscribers that received the message
        """
        payload = json.dumps(message.to_dict(), cls=PubSubEncoder)
        return self.redis_client.publish(CHANNEL_DATA_CHANGES, payload)

    def publish_cache_invalidation(self, entity_type: str, key_pattern: Optional[str] = None) -> int:
        """
        Publish cache invalidation notification.
        
        Args:
            entity_type: Type of entity to invalidate (e.g., "users", "cars")
            key_pattern: Optional specific key pattern
            
        Returns:
            Number of subscribers
        """
        payload = json.dumps({
            "entity_type": entity_type,
            "key_pattern": key_pattern,
            "timestamp": datetime.utcnow().isoformat(),
            "instance_id": INSTANCE_ID
        }, cls=PubSubEncoder)
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
        }, cls=PubSubEncoder)
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
        }, cls=PubSubEncoder)
        return self.redis_client.publish(CHANNEL_USER_EVENTS, payload)

    def subscribe(self, channel: str, callback: Callable[[Dict], None]) -> None:
        """
        Subscribe to a channel with a callback.
        
        Args:
            channel: Channel name
            callback: Function to call on message received
        """
        if channel not in self._message_handlers:
            self._message_handlers[channel] = []
        self._message_handlers[channel].append(callback)

    def unsubscribe(self, channel: str, callback: Optional[Callable] = None) -> None:
        """
        Unsubscribe from a channel.
        
        Args:
            channel: Channel name
            callback: Specific callback to remove (or all if None)
        """
        if channel in self._message_handlers:
            if callback:
                self._message_handlers[channel].remove(callback)
            else:
                del self._message_handlers[channel]

    def _handle_message(self, message: Dict[str, Any]) -> None:
        """Handle incoming pub/sub message."""
        channel = message.get("channel")
        data = message.get("data")

        if channel in self._message_handlers:
            for callback in self._message_handlers[channel]:
                try:
                    callback(data)
                except Exception as e:
                    print(f"Error in pub/sub callback for channel {channel}: {e}")

    def start_listening(self) -> None:
        """Start listening to pub/sub channels in a background thread."""
        if self._running:
            return

        self._running = True
        self.pubsub = self.redis_client.pubsub()
        
        # Subscribe to all channels
        channels = [
            CHANNEL_DATA_CHANGES,
            CHANNEL_CACHE_INVALIDATION,
            CHANNEL_SESSION_EVENTS,
            CHANNEL_USER_EVENTS
        ]
        self.pubsub.subscribe(*channels)

        # Start listener thread
        self._thread = threading.Thread(target=self._listen_loop, daemon=True)
        self._thread.start()

    def _listen_loop(self) -> None:
        """Background thread loop for listening to messages."""
        while self._running:
            try:
                message = self.pubsub.get_message(timeout=1.0)
                if message and message["type"] == "message":
                    self._handle_message({
                        "channel": message["channel"],
                        "data": message["data"]
                    })
            except redis.ConnectionError:
                print("Redis connection lost, reconnecting...")
                self._reconnect_pubsub()
            except Exception as e:
                print(f"Error in pub/sub listener: {e}")

    def _reconnect_pubsub(self) -> None:
        """Reconnect pub/sub after connection loss."""
        try:
            self.pubsub.close()
        except:
            pass

        self.pubsub = self.redis_client.pubsub()
        channels = [
            CHANNEL_DATA_CHANGES,
            CHANNEL_CACHE_INVALIDATION,
            CHANNEL_SESSION_EVENTS,
            CHANNEL_USER_EVENTS
        ]
        self.pubsub.subscribe(*channels)

    def stop_listening(self) -> None:
        """Stop listening to pub/sub channels."""
        self._running = False
        if self.pubsub:
            self.pubsub.unsubscribe()
            self.pubsub.close()
        if self._thread:
            self._thread.join(timeout=5.0)

    def notify_entity_change(
        self,
        entity_type: str,
        action: str,
        entity_id: Any,
        old_data: Optional[Dict] = None,
        new_data: Optional[Dict] = None,
        user_id: Optional[int] = None
    ) -> int:
        """
        Convenience method to notify about entity changes.
        
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
        message = DataChangeMessage(
            entity_type=entity_type,
            action=action,
            entity_id=entity_id,
            old_data=old_data,
            new_data=new_data,
            user_id=user_id
        )
        return self.publish_data_change(message)

    def is_connected(self) -> bool:
        """Check if Redis is connected."""
        try:
            self.redis_client.ping()
            return True
        except redis.ConnectionError:
            return False


# Global pub/sub manager instance
pubsub_manager = RedisPubSubManager()
