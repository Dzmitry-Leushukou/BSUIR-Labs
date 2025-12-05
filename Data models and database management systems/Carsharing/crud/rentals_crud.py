from schemas import RentalCreate, RentalUpdate, Rental
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from datetime import datetime

# Rentals CRUD
def get_rentals(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM rentals ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    rentals = cur.fetchall()
    cur.close()
    conn.close()
    return rentals

def get_rentals_by_user_id(user_id: int, offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM rentals WHERE user_id = %s ORDER BY id LIMIT %s OFFSET %s", (user_id, limit, offset))
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
        raise HTTPException(status_code=404, detail="Rental not found")
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
        raise HTTPException(status_code=400, detail="User already has an active rental")
    
    # Если started_at не предоставлен, используем текущее время сервера
    started_at = rental.started_at if rental.started_at is not None else datetime.utcnow()
    
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
    return new_rental

def update_rental(rental_id: int, rental: RentalUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if rental.ended_at is not None:
        update_fields.append("ended_at = %s")
        values.append(rental.ended_at)
    if rental.status is not None:
        update_fields.append("status = %s")
        values.append(rental.status)
    if rental.price is not None:
        update_fields.append("price = %s")
        values.append(rental.price)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE rentals SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(rental_id)
    
    cur.execute(query, values)
    updated_rental = cur.fetchone()
    
    # Если статус аренды изменяется на "completed", обновляем статус машины на "available"
    if rental.status == "completed":
        # Получаем ID машины из обновленной аренды
        car_id = updated_rental['car_id']
        cur.execute(
            "UPDATE cars SET status = 'available' WHERE id = %s",
            (car_id,)
        )
    
    conn.commit()
    cur.close()
    conn.close()
    if not updated_rental:
        raise HTTPException(status_code=404, detail="Rental not found")
    return updated_rental

def delete_rental(rental_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM rentals WHERE id = %s", (rental_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Rental not found")
    return {"message": "Rental deleted successfully"}