from schemas import CarStateCreate, CarStateUpdate, CarState
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Car States CRUD
def get_car_states(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM car_states ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    states = cur.fetchall()
    cur.close()
    conn.close()
    return states

def get_car_state(state_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM car_states WHERE id = %s", (state_id,))
    state = cur.fetchone()
    cur.close()
    conn.close()
    if not state:
        raise HTTPException(status_code=404, detail="Car state not found")
    return state

def create_car_state(state: CarStateCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO car_states (car_id, checked_by, verified, comment) 
           VALUES (%s, %s, %s, %s) RETURNING *""",
        (state.car_id, state.checked_by, state.verified, state.comment)
    )
    new_state = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_state

def update_car_state(state_id: int, state: CarStateUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if state.checked_by is not None:
        update_fields.append("checked_by = %s")
        values.append(state.checked_by)
    if state.verified is not None:
        update_fields.append("verified = %s")
        values.append(state.verified)
    if state.comment is not None:
        update_fields.append("comment = %s")
        values.append(state.comment)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE car_states SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(state_id)
    
    cur.execute(query, values)
    updated_state = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_state:
        raise HTTPException(status_code=404, detail="Car state not found")
    return updated_state

def delete_car_state(state_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM car_states WHERE id = %s", (state_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Car state not found")
    return {"message": "Car state deleted successfully"}