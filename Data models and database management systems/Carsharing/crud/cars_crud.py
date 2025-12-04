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
            """INSERT INTO cars (vin, plate_number, model, status, position)
               VALUES (%s, %s, %s, %s, ST_GeomFromText(%s, 4326)) RETURNING *""",
            (car.vin, car.plate_number, car.model, car.status, car.position)
        )
    else:
        cur.execute(
            """INSERT INTO cars (vin, plate_number, model, status)
               VALUES (%s, %s, %s, %s) RETURNING *""",
            (car.vin, car.plate_number, car.model, car.status)
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

def get_all_cars_positions():
    """Возвращает все машины с их позициями"""
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT id, vin, plate_number, model, status,
               ST_AsText(position) as position_text,
               ST_X(position::geometry) as longitude,
               ST_Y(position::geometry) as latitude
        FROM cars
        WHERE position IS NOT NULL
    """)
    cars = cur.fetchall()
    cur.close()
    conn.close()
    return cars