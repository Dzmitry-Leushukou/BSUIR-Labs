from fastapi import APIRouter, HTTPException
from schemas import *
from crud.roles_crud import *
from typing import List

router = APIRouter(prefix="/roles", tags=["Roles"])

@router.get("/", response_model=List[Role])
def get_roles_endpoint(offset: int = 0, limit: int = 100):
    return get_roles(offset, limit)

@router.get("/{role_id}", response_model=Role)
def get_role_endpoint(role_id: int):
    if role_id <= 0:
        raise HTTPException(status_code=400, detail="Role ID must be a positive integer")
    return get_role(role_id)

@router.post("/", response_model=Role)
def create_role_endpoint(role: RoleCreate):
    return create_role(role)

@router.put("/{role_id}", response_model=Role)
def update_role_endpoint(role_id: int, role: RoleUpdate):
    if role_id <= 0:
        raise HTTPException(status_code=400, detail="Role ID must be a positive integer")
    return update_role(role_id, role)

@router.delete("/{role_id}")
def delete_role_endpoint(role_id: int):
    if role_id <= 0:
        raise HTTPException(status_code=400, detail="Role ID must be a positive integer")
    return delete_role(role_id)