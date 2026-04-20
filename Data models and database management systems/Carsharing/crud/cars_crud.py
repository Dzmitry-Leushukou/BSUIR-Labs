from schemas import CarCreate, CarUpdate, Car
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from redis_client import redis_client, CARS_CACHE_TTL, CAR_CACHE_TTL
import pytz
from datetime import datetime

# Cache key constants
CARS_LIST_CACHE_KEY = "cars:list"
CARS_AVAILABLE_CACHE_KEY = "cars:available"
CAR_CACHE_KEY_PREFIX = "cars:id"
CARS_POSITIONS_CACHE_KEY = "cars:positions"
CARS_COUNT_CACHE_KEY = "cars:count"


def _notify_cars_change(action: str, car_id: int = None, car_data: dict = None, user_id: int = None):
    """Send pub/sub notification for car changes."""
    from middleware.session_middleware import notify_data_change
    notify_data_change(
        entity_type="cars",
        action=action,
        entity_id=car_id,
        new_data=car_data if action == "create" else None,
        user_id=user_id
    )


# Cars CRUD
def get_cars(offset: int = 0, limit: int = 100):
    """
    Get all cars with caching.
    Cache invalidated on every update/delete due to frequent status changes.
    """
    cache_key = f"{CARS_LIST_CACHE_KEY}:{offset}:{limit}"
    
    # Try to get from cache first
    cached_cars = redis_client.get_cache(cache_key)
    if cached_cars is not None:
        return cached_cars
    
    # If not in cache, query database
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT id, vin, plate_number, model, status, ST_AsText(position) as position, main_photo_id, updated_at 
        FROM cars 
        ORDER BY id 
        LIMIT %s OFFSET %s
    """, (limit, offset))
    cars = cur.fetchall()
    cur.close()
    conn.close()
    
    # Convert to list of dicts
    cars_list = [dict(car) for car in cars] if cars else []
    
    # Cache the result (10 minutes - cars status changes frequently)
    redis_client.set_cache(cache_key, cars_list, ttl=CARS_CACHE_TTL)
    
    return cars_list


def get_car(car_id: int):
    """
    Get a single car by ID with caching.
    """
    cache_key = f"{CAR_CACHE_KEY_PREFIX}:{car_id}"
    
    # Try to get from cache first
    cached_car = redis_client.get_cache(cache_key)
    if cached_car is not None:
        return cached_car
    
    # If not in cache, query database
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT id, vin, plate_number, model, status, ST_AsText(position) as position, main_photo_id, updated_at 
        FROM cars 
        WHERE id = %s
    """, (car_id,))
    car = cur.fetchone()
    cur.close()
    conn.close()
    
    if not car:
        raise HTTPException(status_code=404, detail=f"Автомобиль с ID {car_id} не найден")
    
    # Convert to dict and cache
    car_dict = dict(car)
    redis_client.set_cache(cache_key, car_dict, ttl=CAR_CACHE_TTL)
    
    return car_dict


def create_car(car: CarCreate):
    """
    Create a new car and invalidate cache.
    """
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    try:
        if car.position:
            # Проверяем, является ли позиция в формате POINT
            if car.position.startswith('POINT'):
                # Если это уже формат POINT, используем ST_GeomFromText без SRID
                cur.execute(
                    """INSERT INTO cars (vin, plate_number, model, status, position, main_photo_id)
                       VALUES (%s, %s, %s, 'available', ST_GeomFromText(%s), %s) RETURNING *""",
                    (car.vin, car.plate_number, car.model, car.position, car.main_photo_id)
                )
            else:
                # Если это строка координат, используем ST_GeomFromText с SRID 4326
                cur.execute(
                    """INSERT INTO cars (vin, plate_number, model, status, position, main_photo_id)
                       VALUES (%s, %s, %s, 'available', ST_GeomFromText(%s, 4326), %s) RETURNING *""",
                    (car.vin, car.plate_number, car.model, car.position, car.main_photo_id)
                )
        else:
            cur.execute(
                """INSERT INTO cars (vin, plate_number, model, status, main_photo_id)
                   VALUES (%s, %s, %s, 'available', %s) RETURNING *""",
                (car.vin, car.plate_number, car.model, car.main_photo_id)
            )
        new_car = cur.fetchone()
        conn.commit()
        cur.close()
        conn.close()

        # Invalidate cars cache
        redis_client.invalidate_list_cache("cars")
        
        # Send pub/sub notification
        _notify_cars_change("create", new_car['id'], dict(new_car))

        return new_car
    except Exception as e:
        conn.rollback()
        cur.close()
        conn.close()
        # Проверяем тип ошибки и возвращаем соответствующее сообщение
        if 'duplicate key value violates unique constraint "cars_vin_key"' in str(e):
            raise HTTPException(status_code=400, detail="Автомобиль с таким VIN уже существует")
        elif 'duplicate key value violates unique constraint "cars_plate_number_key"' in str(e):
            raise HTTPException(status_code=400, detail="Автомобиль с таким номерным знаком уже существует")
        else:
            raise HTTPException(status_code=400, detail=f"Ошибка при создании автомобиля: {str(e)}")


def update_car(car_id: int, car: CarUpdate):
    """
    Update an existing car and invalidate cache.
    """
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    try:
        # Build dynamic update query
        update_fields = []
        values = []
        
        if car.model is not None:
            update_fields.append("model = %s")
            values.append(car.model)
        # Пропускаем обновление статуса - он остается неизменным
        if car.position is not None:
            # Проверяем, является ли позиция в формате POINT
            if car.position.startswith('POINT'):
                # Если это уже формат POINT, используем ST_GeomFromText без SRID
                update_fields.append("position = ST_GeomFromText(%s)")
            else:
                # Если это строка координат, используем ST_GeomFromText с SRID 4326
                update_fields.append("position = ST_GeomFromText(%s, 4326)")
            values.append(car.position)
        if car.main_photo_id is not None:
            update_fields.append("main_photo_id = %s")
            values.append(car.main_photo_id)
        
        # Добавляем VIN и номерной знак в список обновляемых полей
        if car.vin is not None:
            update_fields.append("vin = %s")
            values.append(car.vin)
        if car.plate_number is not None:
            update_fields.append("plate_number = %s")
            values.append(car.plate_number)
        
        # Set updated_at to current time in UTC+3
        utc_plus_3 = pytz.timezone('Europe/Moscow')  # Using Europe/Moscow as it's in the same timezone as Minsk
        updated_at = datetime.now(utc_plus_3)
        update_fields.append("updated_at = %s")
        values.append(updated_at)
        
        if not update_fields:
            raise HTTPException(status_code=400, detail="Нет полей для обновления")
        
        query = f"UPDATE cars SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
        values.append(car_id)
        
        cur.execute(query, values)
        updated_car = cur.fetchone()
        conn.commit()
        cur.close()
        conn.close()
        
        if not updated_car:
            raise HTTPException(status_code=404, detail="Автомобиль не найден")

        # Invalidate car-specific cache and list caches
        redis_client.delete_cache(f"{CAR_CACHE_KEY_PREFIX}:{car_id}")
        redis_client.invalidate_list_cache("cars")
        redis_client.delete_cache(CARS_POSITIONS_CACHE_KEY)
        redis_client.delete_cache(CARS_COUNT_CACHE_KEY)
        
        # Send pub/sub notification
        _notify_cars_change("update", car_id, dict(updated_car))

        return updated_car
    except Exception as e:
        conn.rollback()
        cur.close()
        conn.close()
        # Проверяем тип ошибки и возвращаем соответствующее сообщение
        if 'duplicate key value violates unique constraint "cars_vin_key"' in str(e):
            raise HTTPException(status_code=400, detail="Автомобиль с таким VIN уже существует")
        elif 'duplicate key value violates unique constraint "cars_plate_number_key"' in str(e):
            raise HTTPException(status_code=400, detail="Автомобиль с таким номерным знаком уже существует")
        else:
            raise HTTPException(status_code=400, detail=f"Ошибка при обновлении автомобиля: {str(e)}")


def delete_car(car_id: int, user_id: int = None):
    """
    Delete a car and invalidate cache.
    """
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM cars WHERE id = %s", (car_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()

    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Автомобиль не найден")

    # Invalidate car-specific cache and list caches
    redis_client.delete_cache(f"{CAR_CACHE_KEY_PREFIX}:{car_id}")
    redis_client.invalidate_list_cache("cars")
    redis_client.delete_cache(CARS_POSITIONS_CACHE_KEY)
    redis_client.delete_cache(CARS_COUNT_CACHE_KEY)
    
    # Send pub/sub notification
    _notify_cars_change("delete", car_id, user_id=user_id)

    return {"message": "Автомобиль успешно удален"}


def get_cars_positions_with_user_rental_status(user_id: int):
    """
    Get cars for map display.
    If user has active rental: only rented car, otherwise: only available cars.
    Cache is invalidated when rentals change.
    """
    cache_key = f"{CARS_POSITIONS_CACHE_KEY}:user:{user_id}"
    
    # Try to get from cache first
    cached_cars = redis_client.get_cache(cache_key)
    if cached_cars is not None:
        return cached_cars
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Сначала проверяем, есть ли у пользователя активная аренда
    cur.execute("""
        SELECT c.id, c.vin, c.plate_number, c.model, c.status, c.main_photo_id,
               ST_AsText(c.position) as position_text,
               ST_X(c.position::geometry) as longitude,
               ST_Y(c.position::geometry) as latitude,
               TRUE as is_rented_by_user
        FROM cars c
        JOIN rentals r ON c.id = r.car_id
        WHERE r.user_id = %s AND r.status = 'active' AND c.position IS NOT NULL
    """, (user_id,))
    
    rented_cars = cur.fetchall()
    
    if rented_cars:
        # Если у пользователя есть активная аренда, возвращаем только арендованные им машины
        cars = rented_cars
    else:
        # Если у пользователя нет активной аренды, возвращаем только доступные машины
        cur.execute("""
            SELECT c.id, c.vin, c.plate_number, c.model, c.status, c.main_photo_id,
                   ST_AsText(c.position) as position_text,
                   ST_X(c.position::geometry) as longitude,
                   ST_Y(c.position::geometry) as latitude,
                   FALSE as is_rented_by_user
            FROM cars c
            WHERE c.status = 'available' AND c.position IS NOT NULL
        """)
        cars = cur.fetchall()
    
    cur.close()
    conn.close()
    
    # Convert to list of dicts and cache
    cars_list = [dict(car) for car in cars] if cars else []
    redis_client.set_cache(cache_key, cars_list, ttl=CARS_CACHE_TTL)
    
    return cars_list

    
def get_cars_count():
    """
    Get total cars count with caching.
    """
    # Try to get from cache first
    cached_count = redis_client.get_cache(CARS_COUNT_CACHE_KEY)
    if cached_count is not None:
        return cached_count
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT COUNT(*) as count FROM cars")
    result = cur.fetchone()
    cur.close()
    conn.close()
    
    count = result['count'] if result else 0
    
    # Cache the result
    redis_client.set_cache(CARS_COUNT_CACHE_KEY, count, ttl=CARS_CACHE_TTL)
    
    return count
