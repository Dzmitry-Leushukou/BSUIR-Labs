from schemas import TripCompletionCreate, TripCompletionUpdate, TripCompletion
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from datetime import datetime
import pytz
from typing import List, Optional

# Trip Completions CRUD
def get_trip_completions(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Get trip completions with their associated photo IDs
    cur.execute("""
        SELECT tc.*, tc.created_at,
               ARRAY_AGG(tcp.photo_id) AS completion_photo_ids
        FROM trip_completions tc
        LEFT JOIN trip_completion_photos tcp ON tc.id = tcp.trip_completion_id
        GROUP BY tc.id
        ORDER BY
            CASE
                WHEN tc.admin_approved IS NOT NULL THEN 1  -- Move approved/rejected to end
                ELSE 0
            END,
            tc.id ASC  -- Order by oldest to newest for pending items
        LIMIT %s OFFSET %s
    """, (limit, offset))
    completions = cur.fetchall()
    
    # Process the results to handle the photo IDs array properly
    for completion in completions:
        # Convert array to list, handling potential None values
        if completion['completion_photo_ids'] is None or (isinstance(completion['completion_photo_ids'], list) and completion['completion_photo_ids'] == [None]):
            completion['completion_photo_ids'] = []
        elif isinstance(completion['completion_photo_ids'], list):
            # Filter out None values if present
            completion['completion_photo_ids'] = [pid for pid in completion['completion_photo_ids'] if pid is not None]
    
    cur.close()
    conn.close()
    return completions

def get_trip_completion(completion_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    cur.execute("""
        SELECT tc.*, tc.created_at,
               ARRAY_AGG(tcp.photo_id) AS completion_photo_ids
        FROM trip_completions tc
        LEFT JOIN trip_completion_photos tcp ON tc.id = tcp.trip_completion_id
        WHERE tc.id = %s
        GROUP BY tc.id
    """, (completion_id,))
    completion = cur.fetchone()
    
    if not completion:
        raise HTTPException(status_code=404, detail="Завершение поездки не найдено")
    
    # Process the photo IDs array
    if completion['completion_photo_ids'] is None or (isinstance(completion['completion_photo_ids'], list) and completion['completion_photo_ids'] == [None]):
        completion['completion_photo_ids'] = []
    elif isinstance(completion['completion_photo_ids'], list):
        # Filter out None values if present
        completion['completion_photo_ids'] = [pid for pid in completion['completion_photo_ids'] if pid is not None]
    
    cur.close()
    conn.close()
    return completion

def get_trip_completion_by_rental_id(rental_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    cur.execute("""
        SELECT tc.*, tc.created_at,
               ARRAY_AGG(tcp.photo_id) AS completion_photo_ids
        FROM trip_completions tc
        LEFT JOIN trip_completion_photos tcp ON tc.id = tcp.trip_completion_id
        WHERE tc.rental_id = %s
        GROUP BY tc.id
    """, (rental_id,))
    completion = cur.fetchone()
    
    if completion:
        # Process the photo IDs array
        if completion['completion_photo_ids'] is None or (isinstance(completion['completion_photo_ids'], list) and completion['completion_photo_ids'] == [None]):
            completion['completion_photo_ids'] = []
        elif isinstance(completion['completion_photo_ids'], list):
            # Filter out None values if present
            completion['completion_photo_ids'] = [pid for pid in completion['completion_photo_ids'] if pid is not None]
    
    cur.close()
    conn.close()
    return completion

def create_trip_completion(completion: TripCompletionCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Insert the trip completion record
    cur.execute(
        """INSERT INTO trip_completions (rental_id, admin_approved, admin_comment, admin_reviewed_by)
           VALUES (%s, %s, %s, %s) RETURNING *""",
        (completion.rental_id, completion.admin_approved, completion.admin_comment, completion.admin_reviewed_by)
    )
    new_completion = cur.fetchone()
    
    # If photo IDs were provided, link them to the trip completion
    if completion.completion_photo_ids:
        for i, photo_id in enumerate(completion.completion_photo_ids):
            # Set the first photo as primary
            is_primary = (i == 0)
            cur.execute(
                """INSERT INTO trip_completion_photos (trip_completion_id, photo_id, is_primary)
                   VALUES (%s, %s, %s)""",
                (new_completion['id'], photo_id, is_primary)
            )
    
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
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE trip_completions SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(completion_id)
    
    cur.execute(query, values)
    updated_completion = cur.fetchone()
    
    # Get the associated photo IDs after update
    cur.execute("""
        SELECT ARRAY_AGG(tcp.photo_id) AS completion_photo_ids
        FROM trip_completions tc
        LEFT JOIN trip_completion_photos tcp ON tc.id = tcp.trip_completion_id
        WHERE tc.id = %s
        GROUP BY tc.id
    """, (completion_id,))
    photo_result = cur.fetchone()
    
    if photo_result:
        # Process the photo IDs array
        if photo_result['completion_photo_ids'] is None or (isinstance(photo_result['completion_photo_ids'], list) and photo_result['completion_photo_ids'] == [None]):
            updated_completion['completion_photo_ids'] = []
        elif isinstance(photo_result['completion_photo_ids'], list):
            # Filter out None values if present
            updated_completion['completion_photo_ids'] = [pid for pid in photo_result['completion_photo_ids'] if pid is not None]
    else:
        updated_completion['completion_photo_ids'] = []
    
    conn.commit()
    cur.close()
    conn.close()
    if not updated_completion:
        raise HTTPException(status_code=404, detail="Завершение поездки не найдено")
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
        raise HTTPException(status_code=404, detail="Завершение поездки не найдено")
    return {"message": "Завершение поездки успешно удалено"}

def get_trip_completions_count():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT COUNT(*) as count FROM trip_completions")
    result = cur.fetchone()
    cur.close()
    conn.close()
    return result[0] if result else 0