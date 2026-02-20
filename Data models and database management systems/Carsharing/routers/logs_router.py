from fastapi import APIRouter, HTTPException, Depends
from schemas import *
from crud.logs_crud import *
from typing import List
from .users_router import get_current_user

router = APIRouter(prefix="/logs", tags=["Логи"])

@router.get("/", response_model=List[Log])
def get_logs_endpoint(offset: int = 0, limit: int = 10):
    return get_logs(offset, limit)

@router.get("/count", response_model=dict)
def get_logs_count_endpoint(current_user: dict = Depends(get_current_user)):
    from crud.logs_crud import get_logs_count
    count = get_logs_count()
    return {"count": count}

@router.get("/{log_id}", response_model=Log)
def get_log_endpoint(log_id: int):
    return get_log(log_id)

@router.post("/", response_model=Log)
def create_log_endpoint(log: LogCreate, current_user: dict = Depends(get_current_user)):
    return create_log(log)

@router.delete("/{log_id}")
def delete_log_endpoint(log_id: int, current_user: dict = Depends(get_current_user)):
    return delete_log(log_id)