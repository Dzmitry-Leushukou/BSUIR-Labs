from schemas import RentalCreate, RentalUpdate, Rental
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Rentals CRUD
def get_rentals(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM rentals ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
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
    cur.execute(
        """INSERT INTO rentals (user_id, car_id, started_at, price, status) 
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (rental.user_id, rental.car_id, rental.started_at, rental.price, rental.status)
    )
    new_rental = cur.fetchone()
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
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE rentals SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(rental_id)
    
    cur.execute(query, values)
    updated_rental = cur.fetchone()
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