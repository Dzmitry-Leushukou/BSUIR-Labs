from schemas import PaymentLogCreate, PaymentLog
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Payment Logs CRUD
def get_payment_logs(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM payment_logs ORDER BY id DESC LIMIT %s OFFSET %s", (limit, offset))
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
        raise HTTPException(status_code=404, detail="Лог платежа не найден")
    return log

def create_payment_log(log: PaymentLogCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO payment_logs (rental_id, user_id, pay_type, price)
           VALUES (%s, %s, %s, %s) RETURNING *""",
        (log.rental_id, log.user_id, log.pay_type, log.price)
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
        raise HTTPException(status_code=404, detail="Лог платежа не найден")
    return {"message": "Лог платежа успешно удален"}

def get_payment_logs_count():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT COUNT(*) as count FROM payment_logs")
    result = cur.fetchone()
    cur.close()
    conn.close()
    return result['count'] if result else 0