"""
CRUD декораторы с pub/sub уведомлениями для всех сущностей.
"""

from functools import wraps
from typing import Any, Callable, Optional, Dict


def notify_crud_changes(entity_type: str):
    """
    Универсальный декоратор для уведомлений об изменениях сущностей.
    
    Использование:
    @notify_crud_changes("cars")
    def create_car(...):
        ...
    """
    def decorator(func: Callable):
        @wraps(func)
        def wrapper(*args, **kwargs):
            from middleware.session_middleware import notify_data_change
            
            # Определяем тип операции по имени функции
            action = "unknown"
            if func.__name__.startswith("create"):
                action = "create"
            elif func.__name__.startswith("update"):
                action = "update"
            elif func.__name__.startswith("delete"):
                action = "delete"
            
            # Получаем current_user из kwargs или args
            current_user = kwargs.get('current_user')
            if not current_user:
                for arg in args:
                    if isinstance(arg, dict) and 'id' in arg and 'email' in arg:
                        current_user = arg
                        break
            
            user_id = current_user.get('id') if current_user else None
            
            # Выполняем оригинальную функцию
            result = func(*args, **kwargs)
            
            # Отправляем уведомление
            if result and action != "unknown":
                entity_id = None
                
                if action == "delete":
                    # Для delete берем id из аргументов
                    for arg in args:
                        if isinstance(arg, int):
                            entity_id = arg
                            break
                elif isinstance(result, dict):
                    entity_id = result.get('id')
                elif hasattr(result, 'id'):
                    entity_id = result.id
                
                if entity_id is not None:
                    notify_data_change(
                        entity_type=entity_type,
                        action=action,
                        entity_id=entity_id,
                        old_data=None,
                        new_data=result if isinstance(result, dict) else None,
                        user_id=user_id
                    )
            
            return result
        return wrapper
    return decorator
