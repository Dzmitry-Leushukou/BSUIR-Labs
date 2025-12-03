from fastapi import APIRouter, HTTPException
from schemas import *
from crud.users_crud import *
from typing import List

router = APIRouter(prefix="/users", tags=["Users"])

@router.get("/", response_model=List[User])
def get_users_endpoint(offset: int = 0, limit: int = 100):
    return get_users(offset, limit)

@router.get("/{user_id}", response_model=User)
def get_user_endpoint(user_id: int):
    return get_user(user_id)

@router.post("/", response_model=User)
def create_user_endpoint(user: UserCreate):
    return create_user(user)

@router.put("/{user_id}", response_model=User)
def update_user_endpoint(user_id: int, user: UserUpdate):
    return update_user(user_id, user)

@router.delete("/{user_id}")
def delete_user_endpoint(user_id: int):
    return delete_user(user_id)