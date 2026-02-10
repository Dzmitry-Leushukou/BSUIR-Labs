from schemas import PhotoCreate, PhotoUpdate, Photo
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Photos CRUD
def get_photos(offset: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM photos ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    photos = cur.fetchall()
    cur.close()
    conn.close()
    return photos

def get_photo(photo_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM photos WHERE id = %s", (photo_id,))
    photo = cur.fetchone()
    cur.close()
    conn.close()
    if not photo:
        raise HTTPException(status_code=404, detail="Photo not found")
    return photo

def create_photo(photo: PhotoCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO photos (object_type, user_id, car_id, file_data, filename, content_type, file_size, uploaded_by)
           VALUES (%s, %s, %s, %s, %s, %s, %s, %s) RETURNING *""",
        (photo.object_type, photo.user_id, photo.car_id, photo.file_data, photo.filename, photo.content_type, photo.file_size, photo.uploaded_by)
    )
    new_photo = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_photo

def update_photo(photo_id: int, photo: PhotoUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if photo.uploaded_by is not None:
        update_fields.append("uploaded_by = %s")
        values.append(photo.uploaded_by)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="No fields to update")
    
    query = f"UPDATE photos SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(photo_id)
    
    cur.execute(query, values)
    updated_photo = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_photo:
        raise HTTPException(status_code=404, detail="Photo not found")
    return updated_photo

def delete_photo(photo_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM photos WHERE id = %s", (photo_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Photo not found")
    return {"message": "Photo deleted successfully"}

def get_photos_by_trip_completion_id(trip_completion_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("""
        SELECT p.*
        FROM photos p
        JOIN trip_completion_photos tcp ON p.id = tcp.photo_id
        WHERE tcp.trip_completion_id = %s
        ORDER BY tcp.is_primary DESC, tcp.created_at
    """, (trip_completion_id,))
    photos = cur.fetchall()
    cur.close()
    conn.close()
    return photos