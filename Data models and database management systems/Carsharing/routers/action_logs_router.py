from fastapi import APIRouter, HTTPException, Request, Depends, Query
from schemas import ActionLogMongo, LogFilter
from crud.mongo_logs_crud import (
    create_action_log_mongo,
    get_action_logs_mongo,
    get_action_logs_count_mongo,
    get_action_log_by_id_mongo,
    delete_action_log_mongo,
)
from .users_router import get_current_user
from typing import List, Optional
from datetime import datetime


router = APIRouter(prefix="/action_logs", tags=["Логи действий (MongoDB)"])


@router.get("/", response_model=List[ActionLogMongo])
def get_action_logs_endpoint(
    offset: int = Query(0, ge=0),
    limit: int = Query(10, ge=1, le=100),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    user_id: Optional[int] = None,
    action_type: Optional[str] = None,
    target_user_id: Optional[int] = None,
    target_car_id: Optional[int] = None,
    current_user: dict = Depends(get_current_user),
):
    """
    Get action logs with filtering and pagination.
    
    Filters:
    - start_date: Filter logs from this date (ISO 8601 format)
    - end_date: Filter logs until this date (ISO 8601 format)
    - user_id: Filter by actor user ID
    - action_type: Filter by action type (e.g., 'user_login', 'car_create')
    - target_user_id: Filter by target user ID
    - target_car_id: Filter by target car ID
    """
    return get_action_logs_mongo(
        offset=offset,
        limit=limit,
        start_date=start_date,
        end_date=end_date,
        user_id=user_id,
        action_type=action_type,
        target_user_id=target_user_id,
        target_car_id=target_car_id,
    )


@router.get("/count", response_model=dict)
def get_action_logs_count_endpoint(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    user_id: Optional[int] = None,
    action_type: Optional[str] = None,
    current_user: dict = Depends(get_current_user),
):
    """
    Get the total count of action logs with optional filtering.
    """
    count = get_action_logs_count_mongo(
        start_date=start_date,
        end_date=end_date,
        user_id=user_id,
        action_type=action_type,
    )
    return {"count": count}


@router.get("/{log_id}", response_model=ActionLogMongo)
def get_action_log_endpoint(log_id: str, current_user: dict = Depends(get_current_user)):
    """Get a single action log by ID."""
    log = get_action_log_by_id_mongo(log_id)
    if not log:
        raise HTTPException(status_code=404, detail="Лог действий не найден")
    return log


@router.post("/", response_model=ActionLogMongo)
def create_action_log_endpoint(
    log: ActionLogMongo,
    request: Request,
    current_user: dict = Depends(get_current_user),
):
    """Create a new action log entry."""
    ip_address = request.client.host if request.client else None
    
    return create_action_log_mongo(
        actor_user_id=log.actor_user_id,
        action_type=log.action_type,
        description=log.description,
        target_user_id=log.target_user_id,
        target_car_id=log.target_car_id,
        target_rental_id=log.target_rental_id,
        old_values=log.old_values,
        new_values=log.new_values,
        user_agent=log.user_agent,
        ip_address=ip_address,
        actor_email=log.actor_email,
        target_user_email=log.target_user_email,
        target_car_vin=log.target_car_vin,
    )


@router.delete("/{log_id}")
def delete_action_log_endpoint(log_id: str, current_user: dict = Depends(get_current_user)):
    """Delete an action log by ID."""
    success = delete_action_log_mongo(log_id)
    if not success:
        raise HTTPException(status_code=404, detail="Лог действий не найден")
    return {"message": "Лог действий успешно удален"}
