from schemas import SessionCreate, Session, SessionBase
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Sessions CRUD
def get_sessions(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM sessions ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    sessions = cur.fetchall()
    cur.close()
    conn.close()
    return sessions

def get_session(session_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM sessions WHERE id = %s", (session_id,))
    session = cur.fetchone()
    cur.close()
    conn.close()
    if not session:
        raise HTTPException(status_code=404, detail="Session not found")
    return session

def create_session(session: SessionCreate):
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
    return new_session

def update_session(session_id: int, session: SessionBase):
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
    return updated_session

def delete_session(session_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM sessions WHERE id = %s", (session_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Session not found")
    return {"message": "Session deleted successfully"}