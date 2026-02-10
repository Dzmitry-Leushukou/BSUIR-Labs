from fastapi import APIRouter, HTTPException, Request, Depends
from schemas import *
from crud.action_logs_crud import *
from .users_router import get_current_user_from_header
from typing import List

router = APIRouter(prefix="/action_logs", tags=["Логи действий"])

@router.get("/", response_model=List[ActionLogWithEmails])
def get_action_logs_endpoint(offset: int = 0, limit: int = 10, current_user: dict = Depends(get_current_user_from_header)):
    return get_action_logs(offset, limit)

@router.get("/count", response_model=dict)
def get_action_logs_count_endpoint(current_user: dict = Depends(get_current_user_from_header)):
    """
    Get the total count of action logs
    """
    count = get_action_logs_count()
    return {"count": count}

@router.get("/{log_id}", response_model=ActionLog)
def get_action_log_endpoint(log_id: int, current_user: dict = Depends(get_current_user_from_header)):
    return get_action_log(log_id)

@router.post("/", response_model=ActionLog)
def create_action_log_endpoint(log: ActionLogCreate, current_user: dict = Depends(get_current_user_from_header)):
    return create_action_log(log)

@router.delete("/{log_id}")
def delete_action_log_endpoint(log_id: int, current_user: dict = Depends(get_current_user_from_header)):
    return delete_action_log(log_id)