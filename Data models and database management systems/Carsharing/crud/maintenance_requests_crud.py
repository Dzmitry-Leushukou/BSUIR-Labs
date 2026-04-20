from schemas import MaintenanceRequestCreate, MaintenanceRequestUpdate, MaintenanceRequest
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
import pytz
from datetime import datetime


def _notify_maintenance_requests_change(action: str, request_id: int = None, request_data: dict = None, user_id: int = None):
    """Send pub/sub notification for maintenance request changes."""
    from middleware.session_middleware import notify_data_change
    notify_data_change(
        entity_type="maintenance_requests",
        action=action,
        entity_id=request_id,
        new_data=request_data if action == "create" else None,
        user_id=user_id
    )

# Maintenance Requests CRUD
def get_maintenance_requests(offset: int = 0, limit: int = 10):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT mr.*, c.vin, u.email as reported_by_email
        FROM maintenance_requests mr
        LEFT JOIN cars c ON mr.car_id = c.id
        LEFT JOIN users u ON mr.reported_by = u.id
        ORDER BY
            CASE
                WHEN mr.status = 'resolved' THEN 1
                ELSE 0
            END,
            mr.id ASC
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
    
    # Send pub/sub notification
    _notify_maintenance_requests_change("create", new_request['id'], dict(new_request), request.reported_by)
    
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
        # Ensure resolved_at is not earlier than created_at to satisfy the check constraint
        # First get the created_at from the database record
        cur.execute("SELECT created_at FROM maintenance_requests WHERE id = %s", (request_id,))
        result = cur.fetchone()
        if result:
            created_at = result['created_at']
            # Compare the resolved_at with created_at, ensuring resolved_at >= created_at
            
            # Handle timezone-awareness for both dates to prevent comparison error
            if request.resolved_at.tzinfo is None:
                # If no timezone info, assume it's in UTC+3 (Europe/Moscow)
                utc_plus_3 = pytz.timezone('Europe/Moscow')
                resolved_at = utc_plus_3.localize(request.resolved_at)
            else:
                # Convert to UTC+3 timezone
                utc_plus_3 = pytz.timezone('Europe/Moscow')
                resolved_at = request.resolved_at.astimezone(utc_plus_3)
            
            # Ensure created_at is also timezone-aware for comparison
            if created_at.tzinfo is None:
                # If created_at from DB is naive, assume it's in UTC (as stored in DB)
                created_at = pytz.utc.localize(created_at)
            else:
                # Convert created_at to UTC first, then to UTC+3 for consistent comparison
                created_at = created_at.astimezone(utc_plus_3)
            
            # Ensure resolved_at is not earlier than created_at
            if resolved_at < created_at:
                resolved_at = created_at
        else:
            # If we can't get the created_at, use the resolved_at as provided
            if request.resolved_at.tzinfo is None:
                utc_plus_3 = pytz.timezone('Europe/Moscow')
                resolved_at = utc_plus_3.localize(request.resolved_at)
            else:
                utc_plus_3 = pytz.timezone('Europe/Moscow')
                resolved_at = request.resolved_at.astimezone(utc_plus_3)
        
        update_fields.append("resolved_at = %s")
        values.append(resolved_at)
    if request.status is not None:
        # Validate status value
        if request.status not in ['open', 'resolved']:
            raise HTTPException(status_code=400, detail="Недопустимое значение статуса. Разрешенные значения: 'open', 'resolved'")
        
        # If status is being changed to 'resolved' and resolved_at is not provided, set it to current time
        if request.status == 'resolved':
            # Check if resolved_at is already in the update fields
            resolved_at_already_set = any('resolved_at' in field for field in update_fields)
            if not resolved_at_already_set:
                # Get current time in UTC+3 timezone
                utc_plus_3 = pytz.timezone('Europe/Moscow')
                current_time = datetime.now(utc_plus_3)
                
                # Get the created_at from database to ensure resolved_at >= created_at
                cur.execute("SELECT created_at FROM maintenance_requests WHERE id = %s", (request_id,))
                result = cur.fetchone()
                if result:
                    created_at = result['created_at']
                    # Ensure created_at is timezone-aware for comparison
                    if created_at.tzinfo is None:
                        # If created_at from DB is naive, assume it's in UTC (as stored in DB)
                        created_at = pytz.utc.localize(created_at)
                    else:
                        # Convert created_at to UTC first, then to UTC+3 for consistent comparison
                        created_at = created_at.astimezone(utc_plus_3)
                    
                    # Ensure resolved_at is not earlier than created_at
                    if current_time < created_at:
                        current_time = created_at
                
                update_fields.append("resolved_at = %s")
                values.append(current_time)
        
        update_fields.append("status = %s")
        values.append(request.status)
    if request.description is not None:
        update_fields.append("description = %s")
        values.append(request.description)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE maintenance_requests SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(request_id)
    
    cur.execute(query, values)
    updated_request = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_request:
        raise HTTPException(status_code=404, detail="Запрос на обслуживание не найден")
    
    # Send pub/sub notification
    _notify_maintenance_requests_change("update", request_id, dict(updated_request))
    
    return updated_request


def delete_maintenance_request(request_id: int, user_id: int = None):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM maintenance_requests WHERE id = %s", (request_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Запрос на обслуживание не найден")
    
    # Send pub/sub notification
    _notify_maintenance_requests_change("delete", request_id, user_id=user_id)
    
    return {"message": "Запрос на обслуживание успешно удален"}

def get_maintenance_requests_count():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT COUNT(*) as count FROM maintenance_requests")
    result = cur.fetchone()
    cur.close()
    conn.close()
    return result[0] if result else 0