from schemas import ActionLogCreate, ActionLog
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
import json

# Action Logs CRUD
def get_action_logs(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT
            al.*,
            u1.email as actor_email,
            u2.email as target_user_email,
            c.vin as target_car_vin
        FROM action_logs al
        LEFT JOIN users u1 ON al.actor_user_id = u1.id
        LEFT JOIN users u2 ON al.target_user_id = u2.id
        LEFT JOIN cars c ON al.target_car_id = c.id
        ORDER BY al.id DESC
        LIMIT %s OFFSET %s
    """, (limit, offset))
    logs = cur.fetchall()
    cur.close()
    conn.close()
    
    # Process the logs to only show changed values in old_values and new_values
    for log in logs:
        if log['old_values'] and log['new_values']:
            old_values = log['old_values'] if isinstance(log['old_values'], dict) else json.loads(log['old_values'])
            new_values = log['new_values'] if isinstance(log['new_values'], dict) else json.loads(log['new_values'])
            
            # Find the differences between old and new values
            changed_values_old = {}
            changed_values_new = {}
            
            all_keys = set(old_values.keys()) | set(new_values.keys())
            for key in all_keys:
                old_val = old_values.get(key)
                new_val = new_values.get(key)
                if old_val != new_val:
                    if old_val is not None:
                        changed_values_old[key] = old_val
                    if new_val is not None:
                        changed_values_new[key] = new_val
            
            # Update the log with only the changed values, ensuring empty dicts are set to None
            log['old_values'] = changed_values_old if changed_values_old else None
            log['new_values'] = changed_values_new if changed_values_new else None
    
    return logs

def get_action_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM action_logs WHERE id = %s", (log_id,))
    log = cur.fetchone()
    cur.close()
    conn.close()
    if not log:
        raise HTTPException(status_code=404, detail="Лог действий не найден")
    
    # Process the log to only show changed values in old_values and new_values
    if log['old_values'] and log['new_values']:
        old_values = log['old_values'] if isinstance(log['old_values'], dict) else json.loads(log['old_values'])
        new_values = log['new_values'] if isinstance(log['new_values'], dict) else json.loads(log['new_values'])
        
        # Find the differences between old and new values
        changed_values_old = {}
        changed_values_new = {}
        
        all_keys = set(old_values.keys()) | set(new_values.keys())
        for key in all_keys:
            old_val = old_values.get(key)
            new_val = new_values.get(key)
            if old_val != new_val:
                if old_val is not None:
                    changed_values_old[key] = old_val
                if new_val is not None:
                    changed_values_new[key] = new_val
        
        # Update the log with only the changed values, ensuring empty dicts are set to None
        log['old_values'] = changed_values_old if changed_values_old else None
        log['new_values'] = changed_values_new if changed_values_new else None
    
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
        raise HTTPException(status_code=404, detail="Лог действий не найден")
    return {"message": "Лог действий успешно удален"}

def get_action_logs_count():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT COUNT(*) FROM action_logs")
    result = cur.fetchone()
    cur.close()
    conn.close()
    return result[0] if result else 0