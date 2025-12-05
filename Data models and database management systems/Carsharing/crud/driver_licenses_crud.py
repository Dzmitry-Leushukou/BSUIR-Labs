from schemas import DriverLicenseCreate, DriverLicenseUpdate, DriverLicense
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Driver Licenses CRUD
def get_driver_licenses(offset: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM driver_licenses ORDER BY driver_id LIMIT %s OFFSET %s", (limit, offset))
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
        raise HTTPException(status_code=404, detail="Driver license not found")
    return license

def create_driver_license(license: DriverLicenseCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO driver_licenses (license_number, issued_by, expiration_date, document_photo_id, status) 
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (license.license_number, license.issued_by, license.expiration_date, license.document_photo_id, license.status)
    )
    new_license = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_license

def update_driver_license(driver_id: int, license: DriverLicenseUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
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
    if license.status is not None:
        update_fields.append("status = %s")
        values.append(license.status)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE driver_licenses SET {', '.join(update_fields)} WHERE driver_id = %s RETURNING *"
    values.append(driver_id)
    
    cur.execute(query, values)
    updated_license = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_license:
        raise HTTPException(status_code=404, detail="Driver license not found")
    return updated_license

def delete_driver_license(driver_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM driver_licenses WHERE driver_id = %s", (driver_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Driver license not found")
    return {"message": "Driver license deleted successfully"}