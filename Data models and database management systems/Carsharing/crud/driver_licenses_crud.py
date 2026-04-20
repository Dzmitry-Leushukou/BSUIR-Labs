from schemas import DriverLicenseCreate, DriverLicenseUpdate, DriverLicense
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from typing import Optional


def _notify_driver_licenses_change(action: str, driver_id: int = None, license_data: dict = None, user_id: int = None):
    """Send pub/sub notification for driver license changes."""
    from middleware.session_middleware import notify_data_change
    
    # Convert date objects to strings for JSON serialization
    if license_data:
        license_data = dict(license_data)  # Convert RealDictRow to dict
        for key, value in license_data.items():
            if hasattr(value, 'isoformat'):  # date/datetime objects
                license_data[key] = value.isoformat()
    
    notify_data_change(
        entity_type="driver_licenses",
        action=action,
        entity_id=driver_id,
        new_data=license_data if action == "create" else None,
        user_id=user_id
    )

# Driver Licenses CRUD
def get_driver_licenses(offset: int = 0, limit: int = 100, driver_id: Optional[int] = None):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    if driver_id is not None:
        cur.execute("SELECT * FROM driver_licenses WHERE driver_id = %s ORDER BY driver_id ASC LIMIT %s OFFSET %s", (driver_id, limit, offset))
    else:
        cur.execute("""
            SELECT *
            FROM driver_licenses
            ORDER BY
                CASE
                    WHEN status = 'approved' OR status = 'rejected' THEN 1
                    ELSE 0
                END,
                driver_id ASC
            LIMIT %s OFFSET %s
        """, (limit, offset))
    
    licenses = cur.fetchall()
    cur.close()
    conn.close()
    return licenses

def get_driver_license(driver_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM driver_licenses WHERE driver_id = %s", (driver_id,))
    license = cur.fetchone()
    cur.close()
    conn.close()
    if not license:
        raise HTTPException(status_code=404, detail="Водительское удостоверение не найдено")
    return license

def create_driver_license(license: DriverLicenseCreate, driver_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO driver_licenses (license_number, issued_by, expiration_date, document_photo_id, document_photo_back_id, status, driver_id)
           VALUES (%s, %s, %s, %s, %s, %s, %s) RETURNING *""",
        (license.license_number, license.issued_by, license.expiration_date, license.document_photo_id, license.document_photo_back_id, license.status, driver_id)
    )
    new_license = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    
    # Log to MongoDB
    try:
        from crud.mongo_logs_crud import create_action_log_mongo
        create_action_log_mongo(
            actor_user_id=driver_id,
            action_type='driver_license_upload',
            description='Загрузка водительских прав',
            target_user_id=driver_id,
            new_values={
                'license_number': license.license_number,
                'issued_by': license.issued_by,
                'expiration_date': license.expiration_date.isoformat() if license.expiration_date else None,
                'status': license.status
            }
        )
    except Exception as e:
        print(f"Failed to log driver license creation to MongoDB: {str(e)}")
    
    # Send pub/sub notification
    _notify_driver_licenses_change("create", driver_id, dict(new_license), driver_id)

    return new_license

def update_driver_license(driver_id: int, license: DriverLicenseUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Get current license for logging
    cur.execute("SELECT * FROM driver_licenses WHERE driver_id = %s", (driver_id,))
    current_license = cur.fetchone()
    old_status = current_license['status'] if current_license else None

    # Build dynamic update query
    update_fields = []
    values = []

    if license.issued_by is not None:
        update_fields.append("issued_by = %s")
        values.append(license.issued_by)
    if license.expiration_date is not None:
        update_fields.append("expiration_date = %s")
        values.append(license.expiration_date)
    if license.document_photo_id is not None:
        update_fields.append("document_photo_id = %s")
        values.append(license.document_photo_id)
    if license.document_photo_back_id is not None:
        update_fields.append("document_photo_back_id = %s")
        values.append(license.document_photo_back_id)
    if license.status is not None:
        update_fields.append("status = %s")
        values.append(license.status)

    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")

    query = f"UPDATE driver_licenses SET {', '.join(update_fields)} WHERE driver_id = %s RETURNING *"
    values.append(driver_id)

    cur.execute(query, values)
    updated_license = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_license:
        raise HTTPException(status_code=404, detail="Водительское удостоверение не найдено")
    
    # Send pub/sub notification
    _notify_driver_licenses_change("update", driver_id, dict(updated_license))

    # Log to MongoDB
    try:
        from crud.mongo_logs_crud import create_action_log_mongo
        
        if license.status and license.status != old_status:
            action_type = {
                'approved': 'driver_license_approved',
                'rejected': 'driver_license_rejected'
            }.get(license.status, 'driver_license_update')
            
            description = {
                'approved': 'Водительские права одобрены',
                'rejected': 'Водительские права отклонены'
            }.get(license.status, 'Обновление водительских прав')
            
            # Get admin user ID from context if available
            actor_user_id = driver_id  # Default to the license owner
            create_action_log_mongo(
                actor_user_id=actor_user_id,
                action_type=action_type,
                description=description,
                target_user_id=driver_id,
                old_values={'status': old_status},
                new_values={'status': license.status}
            )
    except Exception as e:
        print(f"Failed to log driver license update to MongoDB: {str(e)}")
    
    return updated_license

def delete_driver_license(driver_id: int, user_id: int = None):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM driver_licenses WHERE driver_id = %s", (driver_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Водительское удостоверение не найдено")
    
    # Send pub/sub notification
    _notify_driver_licenses_change("delete", driver_id, user_id=user_id)
    
    return {"message": "Водительское удостоверение успешно удалено"}

def get_driver_licenses_count():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT COUNT(*) as count FROM driver_licenses")
    result = cur.fetchone()
    cur.close()
    conn.close()
    return result[0] if result else 0