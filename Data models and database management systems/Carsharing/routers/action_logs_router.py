from fastapi import APIRouter, HTTPException
from schemas import *
from crud.action_logs_crud import *
from typing import List

router = APIRouter(prefix="/action_logs", tags=["Action Logs"])

@router.get("/", response_model=List[ActionLog])
def get_action_logs_endpoint(offset: int = 0, limit: int = 10):
    return get_action_logs(offset, limit)

@router.get("/{log_id}", response_model=ActionLog)
def get_action_log_endpoint(log_id: int):
    return get_action_log(log_id)

@router.post("/", response_model=ActionLog)
def create_action_log_endpoint(log: ActionLogCreate):
    return create_action_log(log)

@router.delete("/{log_id}")
def delete_action_log_endpoint(log_id: int):
    return delete_action_log(log_id)