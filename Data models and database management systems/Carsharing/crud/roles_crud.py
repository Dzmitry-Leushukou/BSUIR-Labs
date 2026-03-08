from schemas import RoleCreate, RoleUpdate, Role
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from fastapi import HTTPException
from redis_client import redis_client, ROLES_CACHE_TTL

# Cache key constants
ROLES_LIST_CACHE_KEY = "roles:list"
ROLE_CACHE_KEY_PREFIX = "roles:id"


# Roles CRUD
def get_roles(offset: int = 0, limit: int = 100):
    """
    Get all roles with caching.
    Cacheable for standard pagination (offset=0, limit=100)
    """
    # Create cache key based on pagination
    cache_key = f"{ROLES_LIST_CACHE_KEY}:{offset}:{limit}"
    
    # Try to get from cache first
    cached_roles = redis_client.get_cache(cache_key)
    if cached_roles is not None:
        return cached_roles
    
    # If not in cache, query database
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM roles ORDER BY id LIMIT %s OFFSET %s", (limit, offset))
    roles = cur.fetchall()
    cur.close()
    conn.close()
    
    # Convert to list of dicts for JSON serialization
    roles_list = [dict(role) for role in roles] if roles else []
    
    # Cache the result
    redis_client.set_cache(cache_key, roles_list, ttl=ROLES_CACHE_TTL)
    
    return roles_list


def get_role(role_id: int):
    """
    Get a single role by ID with caching.
    """
    cache_key = f"{ROLE_CACHE_KEY_PREFIX}:{role_id}"
    
    # Try to get from cache first
    cached_role = redis_client.get_cache(cache_key)
    if cached_role is not None:
        return cached_role
    
    # If not in cache, query database
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM roles WHERE id = %s", (role_id,))
    role = cur.fetchone()
    cur.close()
    conn.close()
    
    if not role:
        raise HTTPException(status_code=404, detail=f"Role with ID {role_id} not found")
    
    # Convert to dict and cache
    role_dict = dict(role)
    redis_client.set_cache(cache_key, role_dict, ttl=ROLES_CACHE_TTL)
    
    return role_dict


def create_role(role: RoleCreate):
    """
    Create a new role and invalidate cache.
    """
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
    
    # Invalidate roles list cache
    redis_client.invalidate_list_cache("roles")
    
    return dict(new_role) if new_role else None


def update_role(role_id: int, role: RoleUpdate):
    """
    Update an existing role and invalidate cache.
    """
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
    
    # Invalidate specific role cache and list cache
    redis_client.delete_cache(f"{ROLE_CACHE_KEY_PREFIX}:{role_id}")
    redis_client.invalidate_list_cache("roles")
    
    return dict(updated_role)


def delete_role(role_id: int):
    """
    Delete a role and invalidate cache.
    """
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM roles WHERE id = %s", (role_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Role not found")
    
    # Invalidate specific role cache and list cache
    redis_client.delete_cache(f"{ROLE_CACHE_KEY_PREFIX}:{role_id}")
    redis_client.invalidate_list_cache("roles")
    
    return {"message": "Role deleted successfully"}
