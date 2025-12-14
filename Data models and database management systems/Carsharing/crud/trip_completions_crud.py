from schemas import TripCompletionCreate, TripCompletionUpdate, TripCompletion
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from datetime import datetime
import pytz

# Trip Completions CRUD
def get_trip_completions(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT *, created_at
        FROM trip_completions
        ORDER BY
            CASE
                WHEN admin_approved IS NOT NULL THEN 1  -- Move approved/rejected to end
                ELSE 0
            END,
            id ASC  -- Order by oldest to newest for pending items
        LIMIT %s OFFSET %s
    """, (limit, offset))
    completions = cur.fetchall()
    cur.close()
    conn.close()
    return completions

def get_trip_completion(completion_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT *, created_at FROM trip_completions WHERE id = %s", (completion_id,))
    completion = cur.fetchone()
    cur.close()
    conn.close()
    if not completion:
        raise HTTPException(status_code=404, detail="Завершение поездки не найдено")
    return completion

def get_trip_completion_by_rental_id(rental_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT *, created_at FROM trip_completions WHERE rental_id = %s", (rental_id,))
    completion = cur.fetchone()
    cur.close()
    conn.close()
    return completion

def create_trip_completion(completion: TripCompletionCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO trip_completions (rental_id, completion_photo_id, admin_approved, admin_comment, admin_reviewed_by)
           VALUES (%s, %s, %s, %s, %s) RETURNING *, created_at""",
        (completion.rental_id, completion.completion_photo_id, completion.admin_approved, completion.admin_comment, completion.admin_reviewed_by)
    )
    new_completion = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_completion

def update_trip_completion(completion_id: int, completion: TripCompletionUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if completion.admin_approved is not None:
        update_fields.append("admin_approved = %s")
        values.append(completion.admin_approved)
    if completion.admin_comment is not None:
        update_fields.append("admin_comment = %s")
        values.append(completion.admin_comment)
    if completion.admin_reviewed_by is not None:
        update_fields.append("admin_reviewed_by = %s")
        values.append(completion.admin_reviewed_by)
    if completion.admin_reviewed_at is not None:
        # Ensure admin_reviewed_at is in UTC+3 timezone
        utc_plus_3 = pytz.timezone('Europe/Moscow')  # Using Europe/Moscow as it's in the same timezone as Minsk
        if completion.admin_reviewed_at.tzinfo is None:
            # If no timezone info, assume it's in UTC+3
            admin_reviewed_at = utc_plus_3.localize(completion.admin_reviewed_at)
        else:
            # Convert to UTC+3
            admin_reviewed_at = completion.admin_reviewed_at.astimezone(utc_plus_3)
        update_fields.append("admin_reviewed_at = %s")
        values.append(admin_reviewed_at)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE trip_completions SET {', '.join(update_fields)} WHERE id = %s RETURNING *, created_at"
    values.append(completion_id)
    
    cur.execute(query, values)
    updated_completion = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_completion:
        raise HTTPException(status_code=404, detail="Trip completion not found")
    return updated_completion

def delete_trip_completion(completion_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM trip_completions WHERE id = %s", (completion_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Trip completion not found")
    return {"message": "Trip completion deleted successfully"}