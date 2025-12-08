from schemas import ActionLogCreate, ActionLog
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Action Logs CRUD
def get_action_logs(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM action_logs ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    logs = cur.fetchall()
    cur.close()
    conn.close()
    return logs

def get_action_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM action_logs WHERE id = %s", (log_id,))
    log = cur.fetchone()
    cur.close()
    conn.close()
    if not log:
        raise HTTPException(status_code=404, detail="Action log not found")
    return log

def create_action_log(log: ActionLogCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO action_logs (actor_user_id, action_type, target_user_id, target_car_id, target_rental_id, description, old_values, new_values, user_agent)
           VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s) RETURNING *""",
        (log.actor_user_id, log.action_type, log.target_user_id, log.target_car_id, log.target_rental_id,
         log.description, log.old_values, log.new_values, log.user_agent)
    )
    new_log = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_log

def delete_action_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM action_logs WHERE id = %s", (log_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Action log not found")
    return {"message": "Action log deleted successfully"}