from schemas import UserCreate, UserUpdate, User
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
import pytz
from datetime import datetime

# Users CRUD
def get_users(offset: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM users ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    users = cur.fetchall()
    cur.close()
    conn.close()
    return users

def get_user(user_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM users WHERE id = %s", (user_id,))
    user = cur.fetchone()
    cur.close()
    conn.close()
    if not user:
        raise HTTPException(status_code=404, detail="Пользователь не найден")
    return user

def create_user(user: UserCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    try:
        # Set created_at and updated_at to current time in UTC+3
        utc_plus_3 = pytz.timezone('Europe/Moscow')  # Using Europe/Moscow as it's in the same timezone as Minsk
        current_time = datetime.now(utc_plus_3)
        
        cur.execute(
            """INSERT INTO users (email, hashed_password, name, surname, cashback, role_id, status, created_at, updated_at)
               VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s) RETURNING *""",
            (user.email, user.hashed_password, user.name, user.surname, user.cashback, user.role_id, user.status, current_time, current_time)
        )
        new_user = cur.fetchone()
        conn.commit()
        return new_user
    except Exception as e:
        conn.rollback()
        # Check if the error is due to unique constraint violation
        if "duplicate key value violates unique constraint" in str(e).lower():
            raise HTTPException(status_code=400, detail="User with this email already exists")
        else:
            raise e
    finally:
        cur.close()
        conn.close()

def update_user(user_id: int, user: UserUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if user.email is not None:
        update_fields.append("email = %s")
        values.append(user.email)
    if user.name is not None:
        update_fields.append("name = %s")
        values.append(user.name)
    if user.surname is not None:
        update_fields.append("surname = %s")
        values.append(user.surname)
    if user.cashback is not None:
        update_fields.append("cashback = %s")
        values.append(user.cashback)
    if user.role_id is not None:
        update_fields.append("role_id = %s")
        values.append(user.role_id)
    if user.status is not None:
        update_fields.append("status = %s")
        values.append(user.status)
    
    # Set updated_at to current time in UTC+3
    utc_plus_3 = pytz.timezone('Europe/Moscow')  # Using Europe/Moscow as it's in the same timezone as Minsk
    updated_at = datetime.now(utc_plus_3)
    update_fields.append("updated_at = %s")
    values.append(updated_at)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE users SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(user_id)
    
    cur.execute(query, values)
    updated_user = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_user:
        raise HTTPException(status_code=404, detail="User not found")
    return updated_user

def delete_user(user_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM users WHERE id = %s", (user_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="User not found")
    return {"message": "User deleted successfully"}

def get_user_by_email(email: str):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM users WHERE email = %s", (email,))
    user = cur.fetchone()
    cur.close()
    conn.close()
    return user