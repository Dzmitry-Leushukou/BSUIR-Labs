from schemas import CarCreate, CarUpdate, Car
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Cars CRUD
def get_cars(offset: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM cars ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    cars = cur.fetchall()
    cur.close()
    conn.close()
    return cars

def get_car(car_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM cars WHERE id = %s", (car_id,))
    car = cur.fetchone()
    cur.close()
    conn.close()
    if not car:
        raise HTTPException(status_code=404, detail="Car not found")
    return car

def create_car(car: CarCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    if car.position:
        cur.execute(
            """INSERT INTO cars (vin, plate_number, model, status, position, main_photo_id)
               VALUES (%s, %s, %s, %s, ST_GeomFromText(%s, 4326), %s) RETURNING *""",
            (car.vin, car.plate_number, car.model, car.status, car.position, car.main_photo_id)
        )
    else:
        cur.execute(
            """INSERT INTO cars (vin, plate_number, model, status, main_photo_id)
               VALUES (%s, %s, %s, %s, %s) RETURNING *""",
            (car.vin, car.plate_number, car.model, car.status, car.main_photo_id)
        )
    new_car = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_car

def update_car(car_id: int, car: CarUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if car.model is not None:
        update_fields.append("model = %s")
        values.append(car.model)
    if car.status is not None:
        update_fields.append("status = %s")
        values.append(car.status)
    if car.position is not None:
        update_fields.append("position = ST_GeomFromText(%s, 4326)")
        values.append(car.position)
    if car.main_photo_id is not None:
        update_fields.append("main_photo_id = %s")
        values.append(car.main_photo_id)
    
    update_fields.append("updated_at = CURRENT_TIMESTAMP")
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE cars SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(car_id)
    
    cur.execute(query, values)
    updated_car = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_car:
        raise HTTPException(status_code=404, detail="Car not found")
    return updated_car

def delete_car(car_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM cars WHERE id = %s", (car_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Car not found")
    return {"message": "Car deleted successfully"}

def get_cars_positions_with_user_rental_status(user_id: int):
    """Возвращает машины для отображения на карте: если у пользователя есть активная аренда - только арендованная машина, иначе - только доступные машины"""
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
    return cars