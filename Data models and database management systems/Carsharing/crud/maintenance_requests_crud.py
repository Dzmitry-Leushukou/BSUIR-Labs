from schemas import MaintenanceRequestCreate, MaintenanceRequestUpdate, MaintenanceRequest
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
import pytz
from datetime import datetime

# Maintenance Requests CRUD
def get_maintenance_requests(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT *
        FROM maintenance_requests
        ORDER BY
            CASE
                WHEN status = 'resolved' THEN 1
                ELSE 0
            END,
            id ASC
        LIMIT %s OFFSET %s
    """, (limit, offset))
    requests = cur.fetchall()
    cur.close()
    conn.close()
    return requests

def get_maintenance_request(request_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM maintenance_requests WHERE id = %s", (request_id,))
    request = cur.fetchone()
    cur.close()
    conn.close()
    if not request:
        raise HTTPException(status_code=404, detail="Запрос на обслуживание не найден")
    return request

def create_maintenance_request(request: MaintenanceRequestCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO maintenance_requests (car_id, reported_by, status, description) 
           VALUES (%s, %s, %s, %s) RETURNING *""",
        (request.car_id, request.reported_by, request.status, request.description)
    )
    new_request = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_request

def update_maintenance_request(request_id: int, request: MaintenanceRequestUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if request.reported_by is not None:
        update_fields.append("reported_by = %s")
        values.append(request.reported_by)
    if request.resolved_at is not None:
        # Ensure resolved_at is in UTC+3 timezone
        utc_plus_3 = pytz.timezone('Europe/Moscow')  # Using Europe/Moscow as it's in the same timezone as Minsk
        if request.resolved_at.tzinfo is None:
            # If no timezone info, assume it's in UTC+3
            resolved_at = utc_plus_3.localize(request.resolved_at)
        else:
            # Convert to UTC+3
            resolved_at = request.resolved_at.astimezone(utc_plus_3)
        update_fields.append("resolved_at = %s")
        values.append(resolved_at)
    if request.status is not None:
        # Validate status value
        if request.status not in ['open', 'resolved']:
            raise HTTPException(status_code=400, detail="Invalid status value. Allowed values: 'open', 'resolved'")
        update_fields.append("status = %s")
        values.append(request.status)
    if request.description is not None:
        update_fields.append("description = %s")
        values.append(request.description)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE maintenance_requests SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(request_id)
    
    cur.execute(query, values)
    updated_request = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_request:
        raise HTTPException(status_code=404, detail="Maintenance request not found")
    return updated_request

def delete_maintenance_request(request_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM maintenance_requests WHERE id = %s", (request_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Maintenance request not found")
    return {"message": "Maintenance request deleted successfully"}