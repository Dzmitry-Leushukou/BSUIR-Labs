from fastapi import APIRouter, HTTPException
from schemas import *
from crud.driver_licenses_crud import *
from typing import List

router = APIRouter(prefix="/driver_licenses", tags=["Driver Licenses"])

@router.get("/", response_model=List[DriverLicense])
def get_driver_licenses_endpoint(offset: int = 0, limit: int = 10):
    return get_driver_licenses(offset, limit)

@router.get("/{driver_id}", response_model=DriverLicense)
def get_driver_license_endpoint(driver_id: int):
    return get_driver_license(driver_id)

@router.post("/", response_model=DriverLicense)
def create_driver_license_endpoint(license: DriverLicenseCreate):
    return create_driver_license(license)

@router.put("/{driver_id}", response_model=DriverLicense)
def update_driver_license_endpoint(driver_id: int, license: DriverLicenseUpdate):
    return update_driver_license(driver_id, license)

@router.delete("/{driver_id}")
def delete_driver_license_endpoint(driver_id: int):
    return delete_driver_license(driver_id)