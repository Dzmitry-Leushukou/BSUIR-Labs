from schemas import PaymentLogCreate, PaymentLog
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Payment Logs CRUD
def get_payment_logs(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT pl.*, u.email as user_email
        FROM payment_logs pl
        LEFT JOIN users u ON pl.user_id = u.id
        ORDER BY pl.id DESC LIMIT %s OFFSET %s
    """, (limit, offset))
    logs = cur.fetchall()
    cur.close()
    conn.close()
    return logs

def get_payment_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT pl.*, u.email as user_email
        FROM payment_logs pl
        LEFT JOIN users u ON pl.user_id = u.id
        WHERE pl.id = %s
    """, (log_id,))
    log = cur.fetchone()
    cur.close()
    conn.close()
    if not log:
        raise HTTPException(status_code=404, detail="Лог платежа не найден")
    return log

def create_payment_log(log: PaymentLogCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Check if a payment log with the same rental_id, user_id, pay_type, and price already exists
    cur.execute(
        """SELECT id FROM payment_logs
           WHERE rental_id = %s AND user_id = %s AND pay_type = %s AND price = %s""",
        (log.rental_id, log.user_id, log.pay_type, log.price)
    )
    existing_log = cur.fetchone()
    
    if existing_log:
        # If a duplicate is found, return the existing log instead of creating a new one
        cur.execute(
            """SELECT pl.*, u.email as user_email
               FROM payment_logs pl
               LEFT JOIN users u ON pl.user_id = u.id
               WHERE pl.id = %s""",
            (existing_log['id'],)
        )
        result = cur.fetchone()
        cur.close()
        conn.close()
        return result
    
    # If no duplicate found, create a new payment log
    cur.execute(
        """INSERT INTO payment_logs (rental_id, user_id, pay_type, card_number, price)
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (log.rental_id, log.user_id, log.pay_type, log.card_number, log.price)
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
    return result[0] if result else 0