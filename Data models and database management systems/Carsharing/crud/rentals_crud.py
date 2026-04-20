from schemas import RentalCreate, RentalUpdate, Rental
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from datetime import datetime, timezone
import pytz
import os
from redis_client import redis_client
from crud.event_publisher import notify_rentals_updated, notify_cars_updated

# Константы кэша (определены здесь для использования)
CARS_POSITIONS_CACHE_KEY = "cars:positions"


def _notify_rentals_change(action: str, rental_id: int = None, rental_data: dict = None, user_id: int = None):
    """Send pub/sub notification for rental changes."""
    from middleware.session_middleware import notify_data_change
    notify_data_change(
        entity_type="rentals",
        action=action,
        entity_id=rental_id,
        new_data=rental_data if action == "create" else None,
        user_id=user_id
    )
    
    # Invalidate cars cache to update map display
    from redis_client import redis_client
    redis_client.delete_cache_by_pattern("cars:positions:user:*")

# Rentals CRUD
def get_rentals(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM rentals ORDER BY id DESC LIMIT %s OFFSET %s", (limit, offset))
    rentals = cur.fetchall()
    cur.close()
    conn.close()
    return rentals

def get_rentals_by_user_id(user_id: int, offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM rentals WHERE user_id = %s ORDER BY id DESC LIMIT %s OFFSET %s", (user_id, limit, offset))
    rentals = cur.fetchall()
    cur.close()
    conn.close()
    return rentals

def get_rentals_with_car_info_by_user_id(user_id: int, offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT r.*, c.vin, c.plate_number, c.model, c.main_photo_id
        FROM rentals r
        JOIN cars c ON r.car_id = c.id
        WHERE r.user_id = %s ORDER BY r.id DESC LIMIT %s OFFSET %s
    """, (user_id, limit, offset))
    rentals = cur.fetchall()
    cur.close()
    conn.close()
    return rentals

def get_rental(rental_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM rentals WHERE id = %s", (rental_id,))
    rental = cur.fetchone()
    cur.close()
    conn.close()
    if not rental:
        raise HTTPException(status_code=404, detail="Аренда не найдена")
    return rental

def create_rental(rental: RentalCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)

    # Проверяем, есть ли у пользователя уже активная аренда
    cur.execute(
        "SELECT id FROM rentals WHERE user_id = %s AND status = 'active'",
        (rental.user_id,)
    )
    active_rental = cur.fetchone()

    if active_rental:
        conn.close()
        raise HTTPException(status_code=400, detail="Пользователь уже имеет активную аренду")

    # Если started_at не предоставлен, используем текущее время сервера в UTC+3
    utc_plus_3 = pytz.timezone('Europe/Moscow')  # Using Europe/Moscow as it's in the same timezone as Minsk
    started_at = rental.started_at if rental.started_at is not None else datetime.now(utc_plus_3)

    cur.execute(
        """INSERT INTO rentals (user_id, car_id, started_at, price, status)
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (rental.user_id, rental.car_id, started_at, rental.price, rental.status)
    )
    new_rental = cur.fetchone()

    # Обновляем статус машины на "rented"
    cur.execute(
        "UPDATE cars SET status = 'rented' WHERE id = %s",
        (rental.car_id,)
    )

    conn.commit()
    cur.close()
    conn.close()

<<<<<<< HEAD
=======
    # Инвалидируем кэш позиций автомобилей для этого пользователя
    redis_client.delete_cache(f"{CARS_POSITIONS_CACHE_KEY}:user:{rental.user_id}")
    # Также инвалидируем общий кэш позиций
    redis_client.delete_cache(CARS_POSITIONS_CACHE_KEY)
    # Инвалидируем кэш конкретного автомобиля
    redis_client.delete_cache(f"cars:id:{rental.car_id}")
    # Инвалидируем список автомобилей
    redis_client.invalidate_list_cache("cars")
    
    # Публикуем события об изменениях
    notify_rentals_updated(rental_id=new_rental['id'], action="created")
    notify_cars_updated(car_id=rental.car_id, action="updated")
    
    # Публикуем событие о начале аренды для фронтенда
    redis_client.publish_user_session_event(
        user_id=rental.user_id,
        event_type="rental_created",
        instance_id=os.getenv("INSTANCE_ID", "unknown")
    )

>>>>>>> be6f42effb671486e8433257025662a233e281e2
    # Log to MongoDB
    try:
        from crud.mongo_logs_crud import create_action_log_mongo
        create_action_log_mongo(
            actor_user_id=rental.user_id,
            action_type='car_rental_start',
            description='Начало аренды автомобиля',
            target_car_id=rental.car_id,
            target_rental_id=new_rental['id'],
            new_values={
                'started_at': started_at.isoformat(),
                'price': rental.price,
                'status': rental.status
            }
        )
    except Exception as e:
        print(f"Failed to log rental creation to MongoDB: {str(e)}")
    
    # Send pub/sub notification
    _notify_rentals_change("create", new_rental['id'], dict(new_rental), rental.user_id)

    return new_rental

def update_rental(rental_id: int, rental: RentalUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)

    # Сначала получаем текущую запись, чтобы знать начальное состояние
    cur.execute("SELECT ended_at, status, car_id, user_id FROM rentals WHERE id = %s", (rental_id,))
    current_rental = cur.fetchone()
    if not current_rental:
        cur.close()
        conn.close()
        raise HTTPException(status_code=404, detail="Аренда не найдена")

    old_status = current_rental['status']
    
    # Build dynamic update query
    update_fields = []
    values = []

    if rental.ended_at is not None:
        # Ensure ended_at is in UTC+3 timezone
        utc_plus_3 = pytz.timezone('Europe/Moscow')  # Using Europe/Moscow as it's in the same timezone as Minsk
        if rental.ended_at.tzinfo is None:
            # If no timezone info, assume it's in UTC+3
            ended_at = utc_plus_3.localize(rental.ended_at)
        else:
            # Convert to UTC+3
            ended_at = rental.ended_at.astimezone(utc_plus_3)
        update_fields.append("ended_at = %s")
        values.append(ended_at)
    if rental.status is not None:
        update_fields.append("status = %s")
        values.append(rental.status)
    if rental.price is not None:
        update_fields.append("price = %s")
        values.append(rental.price)

    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")

    query = f"UPDATE rentals SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(rental_id)

    cur.execute(query, values)
    updated_rental = cur.fetchone()

    # Если статус аренды изменяется на "completed", обновляем статус машины на "available"
    # и устанавливаем ended_at, если он еще не был установлен до обновления
    if rental.status == "completed":
        # Получаем ID машины из обновленной аренды
        car_id = updated_rental['car_id']
        cur.execute(
            "UPDATE cars SET status = 'available' WHERE id = %s",
            (car_id,)
        )
    # Если статус аренды изменяется на "pending_completion", обновляем статус машины на "pending_completion"
    elif rental.status == "pending_completion":
        car_id = updated_rental['car_id']
        cur.execute(
            "UPDATE cars SET status = 'pending_completion' WHERE id = %s",
            (car_id,)
        )

    conn.commit()
    cur.close()
    conn.close()

    # Инвалидируем кэш при изменении статуса аренды
    if rental.status in ['completed', 'pending_completion']:
        user_id = current_rental['user_id']
        car_id = updated_rental['car_id']
        # Инвалидируем кэш позиций для пользователя
        redis_client.delete_cache(f"{CARS_POSITIONS_CACHE_KEY}:user:{user_id}")
        redis_client.delete_cache(CARS_POSITIONS_CACHE_KEY)
        # Инвалидируем кэш автомобиля
        redis_client.delete_cache(f"cars:id:{car_id}")
        redis_client.invalidate_list_cache("cars")
        
        # Публикуем события
        notify_rentals_updated(rental_id=rental_id, action="updated")
        notify_cars_updated(car_id=car_id, action="updated")
        
        # Специальное событие о завершении аренды для фронтенда
        if rental.status in ['completed', 'pending_completion']:
            instance_id = os.getenv("INSTANCE_ID", "unknown")
            event_type_str = "rental_completed" if rental.status == 'completed' else "rental_pending_completion"
            print(f"[RENTALS] Publishing {event_type_str} event for user {user_id} from {instance_id}")
            redis_client.publish_user_session_event(
                user_id=user_id,
                event_type=event_type_str,
                instance_id=instance_id
            )

    # Проверяем, что обновленная аренда существует
    if not updated_rental:
        raise HTTPException(status_code=404, detail="Аренда не найдена после обновления")
    
    # Send pub/sub notification
    _notify_rentals_change("update", rental_id, dict(updated_rental), current_rental['user_id'])

    # Log to MongoDB
    try:
        from crud.mongo_logs_crud import create_action_log_mongo
        
        if rental.status and rental.status != old_status:
            action_type = {
                'completed': 'car_rental_end',
                'cancelled': 'car_rental_cancel',
                'pending_completion': 'car_rental_pending_completion'
            }.get(rental.status, 'car_rental_update')
            
            description = {
                'completed': 'Завершение аренды автомобиля',
                'cancelled': 'Отмена аренды автомобиля',
                'pending_completion': 'Ожидание завершения аренды'
            }.get(rental.status, 'Обновление статуса аренды')
            
            create_action_log_mongo(
                actor_user_id=current_rental['user_id'],
                action_type=action_type,
                description=description,
                target_car_id=updated_rental['car_id'],
                target_rental_id=rental_id,
                old_values={'status': old_status},
                new_values={'status': rental.status, 'ended_at': rental.ended_at.isoformat() if rental.ended_at else None}
            )
    except Exception as e:
        print(f"Failed to log rental update to MongoDB: {str(e)}")
    
    return updated_rental

<<<<<<< HEAD
def delete_rental(rental_id: int, user_id: int = None):
=======
def delete_rental(rental_id: int):
    # Получаем информацию об аренде перед удалением
>>>>>>> be6f42effb671486e8433257025662a233e281e2
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT user_id, car_id, status FROM rentals WHERE id = %s", (rental_id,))
    rental_to_delete = cur.fetchone()
    
    cur.execute("DELETE FROM rentals WHERE id = %s", (rental_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Аренда не найдена")
    
<<<<<<< HEAD
    # Send pub/sub notification
    _notify_rentals_change("delete", rental_id, user_id=user_id)
=======
    # Если аренда была активная, инвалидируем кэш позиций
    if rental_to_delete and rental_to_delete['status'] == 'active':
        user_id = rental_to_delete['user_id']
        car_id = rental_to_delete['car_id']
        redis_client.delete_cache(f"{CARS_POSITIONS_CACHE_KEY}:user:{user_id}")
        redis_client.delete_cache(CARS_POSITIONS_CACHE_KEY)
        redis_client.delete_cache(f"cars:id:{car_id}")
        redis_client.invalidate_list_cache("cars")
        
        notify_rentals_updated(rental_id=rental_id, action="deleted")
        notify_cars_updated(car_id=car_id, action="updated")
>>>>>>> be6f42effb671486e8433257025662a233e281e2
    
    return {"message": "Аренда успешно удалена"}

def get_rentals_count_by_user_id(user_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT COUNT(*) as count FROM rentals WHERE user_id = %s", (user_id,))
    result = cur.fetchone()
    cur.close()
    conn.close()
    return result[0] if result else 0

def get_rentals_with_user_and_car_info(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT r.*, u.email, c.vin
        FROM rentals r
        JOIN users u ON r.user_id = u.id
        JOIN cars c ON r.car_id = c.id
        ORDER BY r.id DESC LIMIT %s OFFSET %s
    """, (limit, offset))
    rentals = cur.fetchall()
    cur.close()
    conn.close()
    return rentals

def get_rentals_count():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT COUNT(*) as count FROM rentals")
    result = cur.fetchone()
    cur.close()
    conn.close()
    return result[0] if result else 0