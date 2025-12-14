from fastapi import APIRouter, HTTPException
from schemas import *
from crud.logs_crud import *
from typing import List

router = APIRouter(prefix="/logs", tags=["Логи"])

@router.get("/", response_model=List[Log])
def get_logs_endpoint(offset: int = 0, limit: int = 10):
    return get_logs(offset, limit)

@router.get("/{log_id}", response_model=Log)
def get_log_endpoint(log_id: int):
    return get_log(log_id)

@router.post("/", response_model=Log)
def create_log_endpoint(log: LogCreate):
    return create_log(log)

@router.delete("/{log_id}")
def delete_log_endpoint(log_id: int):
    return delete_log(log_id)

@router.get("/count", response_model=dict)
def get_logs_count_endpoint():
    from crud.logs_crud import get_logs_count
    count = get_logs_count()
    return {"count": count}