"""
Утилита для публикации событий при изменении данных.
Используется для уведомления других инстансов об изменениях.
"""

from redis_client import redis_client
import os
from typing import Optional


def publish_entity_event(entity_type: str, entity_id: Optional[int] = None, 
                        action: str = "updated"):
    """
    Опубликовать событие об изменении сущности.
    
    :param entity_type: тип сущности (users, cars, sessions и т.д.)
    :param entity_id: ID измененной сущности
    :param action: действие (created, updated, deleted)
    """
    try:
        instance_id = os.getenv("INSTANCE_ID", "unknown")
        redis_client.publish_data_changed(
            entity_type=entity_type,
            entity_id=entity_id,
            action=action,
            instance_id=instance_id
        )
    except Exception as e:
        print(f"Failed to publish {entity_type} {action} event: {str(e)}")


def notify_users_updated(user_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении пользователя"""
    publish_entity_event("users", user_id, action)


def notify_cars_updated(car_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении автомобиля"""
    publish_entity_event("cars", car_id, action)


def notify_sessions_updated(session_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении сессии"""
    publish_entity_event("sessions", session_id, action)


def notify_rentals_updated(rental_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении аренды"""
    publish_entity_event("rentals", rental_id, action)


def notify_roles_updated(role_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении роли"""
    publish_entity_event("roles", role_id, action)


def notify_photos_updated(photo_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении фото"""
    publish_entity_event("photos", photo_id, action)


def notify_driver_licenses_updated(license_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении водительских прав"""
    publish_entity_event("driver_licenses", license_id, action)


def notify_car_states_updated(state_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении состояния автомобиля"""
    publish_entity_event("car_states", state_id, action)


def notify_maintenance_requests_updated(request_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении запроса на обслуживание"""
    publish_entity_event("maintenance_requests", request_id, action)


def notify_payment_logs_updated(payment_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении платежа"""
    publish_entity_event("payment_logs", payment_id, action)


def notify_trip_completions_updated(completion_id: Optional[int] = None, action: str = "updated"):
    """Уведомление об изменении завершения поездки"""
    publish_entity_event("trip_completions", completion_id, action)
