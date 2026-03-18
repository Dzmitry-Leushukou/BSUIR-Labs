from fastapi import APIRouter, HTTPException, Request, Depends, Query
from schemas import DbQueryLogMongo, ErrorLogMongo
from crud.mongo_logs_crud import (
    create_db_query_log_mongo,
    get_db_query_logs_mongo,
    get_db_query_logs_count_mongo,
    create_error_log_mongo,
    get_error_logs_mongo,
    get_error_logs_count_mongo,
)
from .users_router import get_current_user
from typing import List, Optional
from datetime import datetime


router = APIRouter(prefix="/logs", tags=["Логи запросов и ошибок (MongoDB)"])


# =============================================================================
# DB Query Logs endpoints
# =============================================================================

@router.get("/db_queries", response_model=List[DbQueryLogMongo])
def get_db_query_logs_endpoint(
    offset: int = Query(0, ge=0),
    limit: int = Query(10, ge=1, le=100),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    table_name: Optional[str] = None,
    query_type: Optional[str] = None,
    user_id: Optional[int] = None,
    endpoint: Optional[str] = None,
    current_user: dict = Depends(get_current_user),
):
    """
    Get database query logs with filtering and pagination.
    
    Filters:
    - start_date: Filter logs from this date (ISO 8601 format)
    - end_date: Filter logs until this date (ISO 8601 format)
    - table_name: Filter by table name (e.g., 'users', 'cars')
    - query_type: Filter by query type (SELECT, INSERT, UPDATE, DELETE)
    - user_id: Filter by user ID
    - endpoint: Filter by API endpoint
    """
    return get_db_query_logs_mongo(
        offset=offset,
        limit=limit,
        start_date=start_date,
        end_date=end_date,
        table_name=table_name,
        query_type=query_type,
        user_id=user_id,
        endpoint=endpoint,
    )


@router.get("/db_queries/count", response_model=dict)
def get_db_query_logs_count_endpoint(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    table_name: Optional[str] = None,
    query_type: Optional[str] = None,
    current_user: dict = Depends(get_current_user),
):
    """
    Get the total count of database query logs with optional filtering.
    """
    count = get_db_query_logs_count_mongo(
        start_date=start_date,
        end_date=end_date,
        table_name=table_name,
        query_type=query_type,
    )
    return {"count": count}


# =============================================================================
# Error Logs endpoints
# =============================================================================

@router.get("/errors", response_model=List[ErrorLogMongo])
def get_error_logs_endpoint(
    offset: int = Query(0, ge=0),
    limit: int = Query(10, ge=1, le=100),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    error_type: Optional[str] = None,
    severity: Optional[str] = None,
    user_id: Optional[int] = None,
    endpoint: Optional[str] = None,
    current_user: dict = Depends(get_current_user),
):
    """
    Get error logs with filtering and pagination.
    
    Filters:
    - start_date: Filter logs from this date (ISO 8601 format)
    - end_date: Filter logs until this date (ISO 8601 format)
    - error_type: Filter by error type (e.g., 'HTTPException', 'ValueError')
    - severity: Filter by severity level (DEBUG, INFO, WARNING, ERROR, CRITICAL)
    - user_id: Filter by user ID
    - endpoint: Filter by API endpoint
    """
    return get_error_logs_mongo(
        offset=offset,
        limit=limit,
        start_date=start_date,
        end_date=end_date,
        error_type=error_type,
        severity=severity,
        user_id=user_id,
        endpoint=endpoint,
    )


@router.get("/errors/count", response_model=dict)
def get_error_logs_count_endpoint(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    error_type: Optional[str] = None,
    severity: Optional[str] = None,
    current_user: dict = Depends(get_current_user),
):
    """
    Get the total count of error logs with optional filtering.
    """
    count = get_error_logs_count_mongo(
        start_date=start_date,
        end_date=end_date,
        error_type=error_type,
        severity=severity,
    )
    return {"count": count}
