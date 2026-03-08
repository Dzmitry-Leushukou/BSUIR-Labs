from schemas import SessionCreate, Session, SessionBase
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from redis_client import redis_client, SESSIONS_CACHE_TTL

# Cache key constants
SESSIONS_LIST_CACHE_KEY = "sessions:list"
SESSION_CACHE_KEY_PREFIX = "sessions:id"
USER_SESSIONS_CACHE_KEY_PREFIX = "sessions:user"
SESSIONS_COUNT_CACHE_KEY = "sessions:count"


# Sessions CRUD
def get_sessions(offset: int = 0, limit: int = 10):
    """
    Get all sessions with caching.
    """
    cache_key = f"{SESSIONS_LIST_CACHE_KEY}:{offset}:{limit}"
    
    # Try to get from cache first
    cached_sessions = redis_client.get_cache(cache_key)
    if cached_sessions is not None:
        return cached_sessions
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM sessions ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    sessions = cur.fetchall()
    cur.close()
    conn.close()
    
    # Convert to list of dicts
    sessions_list = [dict(session) for session in sessions] if sessions else []
    
    # Cache the result
    redis_client.set_cache(cache_key, sessions_list, ttl=SESSIONS_CACHE_TTL)
    
    return sessions_list


def get_session(session_id: int):
    """
    Get a single session by ID with caching.
    """
    cache_key = f"{SESSION_CACHE_KEY_PREFIX}:{session_id}"
    
    # Try to get from cache first
    cached_session = redis_client.get_cache(cache_key)
    if cached_session is not None:
        return cached_session
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM sessions WHERE id = %s", (session_id,))
    session = cur.fetchone()
    cur.close()
    conn.close()
    
    if not session:
        raise HTTPException(status_code=404, detail="Session not found")
    
    # Convert to dict and cache
    session_dict = dict(session)
    redis_client.set_cache(cache_key, session_dict, ttl=SESSIONS_CACHE_TTL)
    
    return session_dict


def create_session(session: SessionCreate):
    """
    Create a new session and invalidate cache.
    Sessions store active user connections.
    """
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO sessions (user_id)
           VALUES (%s) RETURNING *""",
        (session.user_id,)
    )
    new_session = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    
    # Invalidate sessions list cache
    redis_client.invalidate_list_cache("sessions")
    
    # Also cache individual session
    session_dict = dict(new_session)
    session_id = session_dict.get('id')
    if session_id:
        redis_client.set_cache(f"{SESSION_CACHE_KEY_PREFIX}:{session_id}", session_dict, ttl=SESSIONS_CACHE_TTL)
    
    return new_session


def update_session(session_id: int, session: SessionBase):
    """
    Update an existing session and invalidate cache.
    """
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        "UPDATE sessions SET user_id = %s, updated_at = CURRENT_TIMESTAMP WHERE id = %s RETURNING *",
        (session.user_id, session_id)
    )
    updated_session = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    
    if not updated_session:
        raise HTTPException(status_code=404, detail="Session not found")
    
    # Invalidate session-specific cache and list cache
    redis_client.delete_cache(f"{SESSION_CACHE_KEY_PREFIX}:{session_id}")
    redis_client.invalidate_list_cache("sessions")
    
    return updated_session


def delete_session(session_id: int):
    """
    Delete a session and invalidate cache.
    """
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM sessions WHERE id = %s", (session_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Session not found")
    
    # Invalidate session-specific cache and list cache
    redis_client.delete_cache(f"{SESSION_CACHE_KEY_PREFIX}:{session_id}")
    redis_client.invalidate_list_cache("sessions")
    
    return {"message": "Session deleted successfully"}


def get_user_sessions(user_id: int):
    """
    Get all active sessions for a user with caching.
    Useful for tracking user connections.
    """
    cache_key = f"{USER_SESSIONS_CACHE_KEY_PREFIX}:{user_id}"
    
    # Try to get from cache first
    cached_sessions = redis_client.get_cache(cache_key)
    if cached_sessions is not None:
        return cached_sessions
    
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM sessions WHERE user_id = %s ORDER BY created_at DESC", (user_id,))
    sessions = cur.fetchall()
    cur.close()
    conn.close()
    
    # Convert to list of dicts
    sessions_list = [dict(session) for session in sessions] if sessions else []
    
    # Cache the result
    redis_client.set_cache(cache_key, sessions_list, ttl=SESSIONS_CACHE_TTL)
    
    return sessions_list
