from fastapi import APIRouter, HTTPException, Depends, Request
from schemas import *
from crud.driver_licenses_crud import *
from typing import List
from .users_router import get_current_user
from crud.users_crud import get_user

router = APIRouter(prefix="/driver_licenses", tags=["Водительские удостоверения"])

@router.get("/", response_model=List[DriverLicense])
def get_driver_licenses_endpoint(request: Request, offset: int = 0, limit: int = 10, current_user: dict = Depends(get_current_user)):
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    if is_admin:
        return get_driver_licenses(offset, limit)
    else:
        return get_driver_licenses(offset, limit, driver_id=current_user['id'])

@router.get("/count", response_model=dict)
def get_driver_licenses_count_endpoint(current_user: dict = Depends(get_current_user)):
    from crud.driver_licenses_crud import get_driver_licenses_count
    count = get_driver_licenses_count()
    return {"count": count}

@router.get("/{driver_id}", response_model=DriverLicense)
def get_driver_license_endpoint(request: Request, driver_id: int, current_user: dict = Depends(get_current_user)):
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    if not is_admin and current_user['id'] != driver_id:
        raise HTTPException(status_code=403, detail="Нет прав для доступа к этому ресурсу")

    return get_driver_license(driver_id)

@router.post("/", response_model=DriverLicense)
def create_driver_license_endpoint(request: Request, license: DriverLicenseCreate, current_user: dict = Depends(get_current_user)):
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    if is_admin and hasattr(license, 'driver_id') and license.driver_id:
        driver_id = license.driver_id
    else:
        driver_id = current_user['id']

    try:
        existing_license = get_driver_license(driver_id)
        update_data = DriverLicenseUpdate(
            license_number=license.license_number,
            issued_by=license.issued_by,
            expiration_date=license.expiration_date,
            document_photo_id=license.document_photo_id,
            document_photo_back_id=license.document_photo_back_id,
            status='pending'
        )
        return update_driver_license(driver_id, update_data)
    except:
        return create_driver_license(license, driver_id)

@router.put("/{driver_id}", response_model=DriverLicense)
def update_driver_license_endpoint(request: Request, driver_id: int, license: DriverLicenseUpdate, current_user: dict = Depends(get_current_user)):
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    return update_driver_license(driver_id, license)

@router.delete("/{driver_id}")
def delete_driver_license_endpoint(request: Request, driver_id: int, current_user: dict = Depends(get_current_user)):
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    return delete_driver_license(driver_id)