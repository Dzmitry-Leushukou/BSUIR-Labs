from schemas import PaymentLogCreate, PaymentLog
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Payment Logs CRUD
def get_payment_logs(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM payment_logs ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    logs = cur.fetchall()
    cur.close()
    conn.close()
    return logs

def get_payment_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM payment_logs WHERE id = %s", (log_id,))
    log = cur.fetchone()
    cur.close()
    conn.close()
    if not log:
        raise HTTPException(status_code=404, detail="Payment log not found")
    return log

def create_payment_log(log: PaymentLogCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO payment_logs (rental_id, user_id, pay_type, price, ip) 
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (log.rental_id, log.user_id, log.pay_type, log.price, log.ip)
    )
    new_log = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_log

def delete_payment_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM payment_logs WHERE id = %s", (log_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Payment log not found")
    return {"message": "Payment log deleted successfully"}