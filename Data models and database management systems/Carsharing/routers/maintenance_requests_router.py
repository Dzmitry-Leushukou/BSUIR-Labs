from fastapi import APIRouter, HTTPException, Depends
from schemas import *
from crud.maintenance_requests_crud import *
from typing import List
from .users_router import get_current_user_from_header

router = APIRouter(prefix="/maintenance_requests", tags=["Запросы на обслуживание"])

@router.get("/", response_model=List[MaintenanceRequestWithCarInfo])
def get_maintenance_requests_endpoint(offset: int = 0, limit: int = 10):
    return get_maintenance_requests(offset, limit)

@router.get("/count", response_model=dict)
def get_maintenance_requests_count_endpoint(current_user: dict = Depends(get_current_user_from_header)):
    from crud.maintenance_requests_crud import get_maintenance_requests_count
    count = get_maintenance_requests_count()
    return {"count": count}

@router.get("/{request_id}", response_model=MaintenanceRequest)
def get_maintenance_request_endpoint(request_id: int):
    return get_maintenance_request(request_id)

@router.post("/", response_model=MaintenanceRequest)
def create_maintenance_request_endpoint(request: MaintenanceRequestCreate):
    return create_maintenance_request(request)

@router.put("/{request_id}", response_model=MaintenanceRequest)
def update_maintenance_request_endpoint(request_id: int, request: MaintenanceRequestUpdate):
    return update_maintenance_request(request_id, request)

@router.delete("/{request_id}")
def delete_maintenance_request_endpoint(request_id: int):
    return delete_maintenance_request(request_id)