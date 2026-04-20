from schemas import UserCreate, UserUpdate, User
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from redis_client import redis_client, USERS_CACHE_TTL, USER_CACHE_TTL
from crud.event_publisher import notify_users_updated
from redis_client import redis_client
import os
import pytz
from datetime import datetime

# Cache key constants
USERS_LIST_CACHE_KEY = "users:list"
USER_CACHE_KEY_PREFIX = "users:id"
USER_BY_EMAIL_CACHE_KEY_PREFIX = "users:email"
USERS_COUNT_CACHE_KEY = "users:count"


# Users CRUD
def get_users(offset: int = 0, limit: int = 100):
    """
    Get all users with caching.
    """
    cache_key = f"{USERS_LIST_CACHE_KEY}:{offset}:{limit}"
    
    # Try to get from cache first
    cached_users = redis_client.get_cache(cache_key)
    if cached_users is not None:
        return cached_users
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT u.*, r.name as role_name
        FROM users u
        LEFT JOIN roles r ON u.role_id = r.id
        ORDER BY u.id
        LIMIT %s OFFSET %s
    """, (limit, offset))
    users = cur.fetchall()
    cur.close()
    conn.close()
    
    # Convert to list of dicts
    users_list = [dict(user) for user in users] if users else []
    
    # Cache the result
    redis_client.set_cache(cache_key, users_list, ttl=USERS_CACHE_TTL)
    
    return users_list


def get_user(user_id: int):
    """
    Get a single user by ID with caching.
    """
    cache_key = f"{USER_CACHE_KEY_PREFIX}:{user_id}"
    
    # Try to get from cache first
    cached_user = redis_client.get_cache(cache_key)
    if cached_user is not None:
        return cached_user
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM users WHERE id = %s", (user_id,))
    user = cur.fetchone()
    cur.close()
    conn.close()
    
    if not user:
        raise HTTPException(status_code=404, detail=f"Пользователь с ID {user_id} не найден")
    
    # Convert to dict and cache
    user_dict = dict(user)
    redis_client.set_cache(cache_key, user_dict, ttl=USER_CACHE_TTL)
    
    return user_dict


def create_user(user: UserCreate):
    """
    Create a new user and invalidate cache.
    """
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
        
        # Invalidate users list cache
        redis_client.invalidate_list_cache("users")
        
        # Публикуем событие о создании пользователя
        notify_users_updated(action="created")

        return new_user
    except Exception as e:
        conn.rollback()
        # Check if the error is due to unique constraint violation
        if "duplicate key value violates unique constraint" in str(e).lower():
            raise HTTPException(status_code=400, detail="Пользователь с этим email уже существует")
        else:
            raise e
    finally:
        cur.close()
        conn.close()


def update_user(user_id: int, user: UserUpdate):
    """
    Update an existing user and invalidate cache.
    """
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Get current user data for logging
    cur.execute("SELECT * FROM users WHERE id = %s", (user_id,))
    current_user = cur.fetchone()
    old_values = {}
    new_values = {}

    # Build dynamic update query
    update_fields = []
    values = []

    if user.email is not None:
        update_fields.append("email = %s")
        values.append(user.email)
        if current_user and current_user['email'] != user.email:
            old_values['email'] = current_user['email']
            new_values['email'] = user.email
    if user.name is not None:
        update_fields.append("name = %s")
        values.append(user.name)
        if current_user and current_user['name'] != user.name:
            old_values['name'] = current_user['name']
            new_values['name'] = user.name
    if user.surname is not None:
        update_fields.append("surname = %s")
        values.append(user.surname)
        if current_user and current_user['surname'] != user.surname:
            old_values['surname'] = current_user['surname']
            new_values['surname'] = user.surname
    if user.cashback is not None:
        update_fields.append("cashback = %s")
        values.append(user.cashback)
        if current_user and current_user['cashback'] != user.cashback:
            old_values['cashback'] = float(current_user['cashback']) if current_user['cashback'] else 0
            new_values['cashback'] = user.cashback
    if user.role_id is not None:
        update_fields.append("role_id = %s")
        values.append(user.role_id)
        if current_user and current_user['role_id'] != user.role_id:
            old_values['role_id'] = current_user['role_id']
            new_values['role_id'] = user.role_id
    if user.status is not None:
        update_fields.append("status = %s")
        values.append(user.status)
        if current_user and current_user['status'] != user.status:
            old_values['status'] = current_user['status']
            new_values['status'] = user.status

    # Set updated_at to current time in UTC+3
    utc_plus_3 = pytz.timezone('Europe/Moscow')  # Using Europe/Moscow as it's in the same timezone as Minsk
    updated_at = datetime.now(utc_plus_3)
    update_fields.append("updated_at = %s")
    values.append(updated_at)

    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")

    query = f"UPDATE users SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(user_id)

    cur.execute(query, values)
    updated_user = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()

    if not updated_user:
        raise HTTPException(status_code=404, detail="Пользователь не найден")

    # Log to MongoDB
    if old_values and new_values:
        try:
            from crud.mongo_logs_crud import create_action_log_mongo
            create_action_log_mongo(
                actor_user_id=user_id,
                action_type='profile_update',
                description='Обновление профиля пользователя',
                target_user_id=user_id,
                old_values=old_values if old_values else None,
                new_values=new_values if new_values else None
            )
        except Exception as e:
            print(f"Failed to log profile update to MongoDB: {str(e)}")

    # Invalidate user-specific cache and list caches
    redis_client.delete_cache(f"{USER_CACHE_KEY_PREFIX}:{user_id}")
    redis_client.delete_cache(f"{USER_BY_EMAIL_CACHE_KEY_PREFIX}:{updated_user.get('email')}")
    redis_client.invalidate_list_cache("users")
    redis_client.delete_cache(USERS_COUNT_CACHE_KEY)
    
    # Публикуем событие об обновлении пользователя
    notify_users_updated(user_id=user_id, action="updated")
    
    # Если обновлялся кэшбэк, публикуем специальное событие
    if 'cashback' in [k for k in user.model_dump().keys() if getattr(user, k) is not None]:
        redis_client.publish_user_session_event(
            user_id=user_id,
            event_type="cashback_updated",
            instance_id=os.getenv("INSTANCE_ID", "unknown")
        )

    return updated_user


def delete_user(user_id: int):
    """
    Delete a user and invalidate cache.
    """
    # Get user email first for cache invalidation
    user = get_user(user_id)
    user_email = user.get('email')
    
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM users WHERE id = %s", (user_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Пользователь не найден")
    
    # Invalidate user-specific cache and list caches
    redis_client.delete_cache(f"{USER_CACHE_KEY_PREFIX}:{user_id}")
    if user_email:
        redis_client.delete_cache(f"{USER_BY_EMAIL_CACHE_KEY_PREFIX}:{user_email}")
    redis_client.invalidate_list_cache("users")
    redis_client.delete_cache(USERS_COUNT_CACHE_KEY)
    
    # Публикуем событие об удалении пользователя
    notify_users_updated(user_id=user_id, action="deleted")

    return {"message": "Пользователь успешно удален"}


def get_user_by_email(email: str):
    """
    Get a user by email with caching.
    """
    cache_key = f"{USER_BY_EMAIL_CACHE_KEY_PREFIX}:{email}"
    
    # Try to get from cache first
    cached_user = redis_client.get_cache(cache_key)
    if cached_user is not None:
        return cached_user
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM users WHERE email = %s", (email,))
    user = cur.fetchone()
    cur.close()
    conn.close()
    
    if user:
        # Convert to dict and cache
        user_dict = dict(user)
        redis_client.set_cache(cache_key, user_dict, ttl=USER_CACHE_TTL)
        return user_dict
    
    return None

    
def get_users_count():
    """
    Get total users count with caching.
    """
    # Try to get from cache first
    cached_count = redis_client.get_cache(USERS_COUNT_CACHE_KEY)
    if cached_count is not None:
        return cached_count
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT COUNT(*) as count FROM users")
    result = cur.fetchone()
    cur.close()
    conn.close()
    
    count = result['count'] if result else 0
    
    # Cache the result
    redis_client.set_cache(USERS_COUNT_CACHE_KEY, count, ttl=USERS_CACHE_TTL)
    
    return count
