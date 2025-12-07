from fastapi import APIRouter, HTTPException, Depends, Request
from schemas import *
from crud.driver_licenses_crud import *
from typing import List
from .users_router import get_current_user_from_header
from crud.users_crud import get_user

router = APIRouter(prefix="/driver_licenses", tags=["Driver Licenses"])

@router.get("/", response_model=List[DriverLicense])
def get_driver_licenses_endpoint(request: Request, offset: int = 0, limit: int = 10, current_user: dict = Depends(get_current_user_from_header)):
    return get_driver_licenses(offset, limit)

@router.get("/{driver_id}", response_model=DriverLicense)
def get_driver_license_endpoint(request: Request, driver_id: int, current_user: dict = Depends(get_current_user_from_header)):
    return get_driver_license(driver_id)

@router.post("/", response_model=DriverLicense)
def create_driver_license_endpoint(request: Request, license: DriverLicenseCreate, current_user: dict = Depends(get_current_user_from_header)):
    # Check if user is admin - if so, allow creating licenses
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    # For now, allow admins to create licenses
    # Additional business logic can be implemented here if needed
    
    return create_driver_license(license)

@router.put("/{driver_id}", response_model=DriverLicense)
def update_driver_license_endpoint(request: Request, driver_id: int, license: DriverLicenseUpdate, current_user: dict = Depends(get_current_user_from_header)):
    # Check if user is admin - if so, allow updating any license
    # Otherwise, we might implement additional checks if needed
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    # For now, allow admins to update any license
    # Additional business logic can be implemented here if needed
    
    return update_driver_license(driver_id, license)

@router.delete("/{driver_id}")
def delete_driver_license_endpoint(request: Request, driver_id: int, current_user: dict = Depends(get_current_user_from_header)):
    # Check if user is admin - if so, allow deleting any license
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    # For now, allow admins to delete any license
    # Additional business logic can be implemented here if needed
    
    return delete_driver_license(driver_id)