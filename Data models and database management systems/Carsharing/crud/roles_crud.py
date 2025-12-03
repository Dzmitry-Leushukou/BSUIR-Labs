from schemas import RoleCreate, RoleUpdate, Role
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException

# Roles CRUD
def get_roles(offset: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM roles ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    roles = cur.fetchall()
    cur.close()
    conn.close()
    return roles

def get_role(role_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM roles WHERE id = %s", (role_id,))
    role = cur.fetchone()
    cur.close()
    conn.close()
    if not role:
        raise HTTPException(status_code=404, detail="Role not found")
    return role

def create_role(role: RoleCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        "INSERT INTO roles (name, description) VALUES (%s, %s) RETURNING *",
        (role.name, role.description)
    )
    new_role = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_role

def update_role(role_id: int, role: RoleUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        "UPDATE roles SET name = %s, description = %s WHERE id = %s RETURNING *",
        (role.name, role.description, role_id)
    )
    updated_role = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_role:
        raise HTTPException(status_code=404, detail="Role not found")
    return updated_role

def delete_role(role_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM roles WHERE id = %s", (role_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Role not found")
    return {"message": "Role deleted successfully"}