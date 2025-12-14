from fastapi import APIRouter, HTTPException
from schemas import *
from crud.payment_logs_crud import *
from typing import List

router = APIRouter(prefix="/payment_logs", tags=["Логи платежей"])

@router.get("/", response_model=List[PaymentLog])
def get_payment_logs_endpoint(offset: int = 0, limit: int = 10):
    return get_payment_logs(offset, limit)

@router.get("/{log_id}", response_model=PaymentLog)
def get_payment_log_endpoint(log_id: int):
    return get_payment_log(log_id)

@router.post("/", response_model=PaymentLog)
def create_payment_log_endpoint(log: PaymentLogCreate):
    return create_payment_log(log)

@router.delete("/{log_id}")
def delete_payment_log_endpoint(log_id: int):
    return delete_payment_log(log_id)

@router.get("/count", response_model=dict)
def get_payment_logs_count_endpoint():
    from crud.payment_logs_crud import get_payment_logs_count
    count = get_payment_logs_count()
    return {"count": count}