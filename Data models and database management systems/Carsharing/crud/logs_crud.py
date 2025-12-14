from schemas import LogCreate, Log
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Logs CRUD
def get_logs(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM logs ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    logs = cur.fetchall()
    cur.close()
    conn.close()
    return logs

def get_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM logs WHERE id = %s", (log_id,))
    log = cur.fetchone()
    cur.close()
    conn.close()
    if not log:
        raise HTTPException(status_code=404, detail="Лог не найден")
    return log

def create_log(log: LogCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO logs (actor_user_id, action_type, target_id)
           VALUES (%s, %s, %s) RETURNING *""",
        (log.actor_user_id, log.action_type, log.target_id)
    )
    new_log = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_log

def delete_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM logs WHERE id = %s", (log_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Лог не найден")
    return {"message": "Лог успешно удален"}