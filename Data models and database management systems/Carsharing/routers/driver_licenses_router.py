from fastapi import APIRouter, HTTPException, Depends, Request
from schemas import *
from crud.driver_licenses_crud import *
from typing import List
from .users_router import get_current_user_from_header
from crud.users_crud import get_user

router = APIRouter(prefix="/driver_licenses", tags=["Водительские удостоверения"])

@router.get("/", response_model=List[DriverLicense])
def get_driver_licenses_endpoint(request: Request, offset: int = 0, limit: int = 10, current_user: dict = Depends(get_current_user_from_header)):
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    # Если пользователь администратор, возвращаем все водительские права
    if is_admin:
        return get_driver_licenses(offset, limit)
    # Если обычный пользователь, возвращаем только его права
    else:
        return get_driver_licenses(offset, limit, driver_id=current_user['id'])

@router.get("/count", response_model=dict)
def get_driver_licenses_count_endpoint(current_user: dict = Depends(get_current_user_from_header)):
    from crud.driver_licenses_crud import get_driver_licenses_count
    count = get_driver_licenses_count()
    return {"count": count}

@router.get("/{driver_id}", response_model=DriverLicense)
def get_driver_license_endpoint(request: Request, driver_id: int, current_user: dict = Depends(get_current_user_from_header)):
    # Проверяем, является ли пользователь администратором или запрашивает свои собственные права
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    # Если пользователь не админ и пытается получить права другого пользователя
    if not is_admin and current_user['id'] != driver_id:
        raise HTTPException(status_code=403, detail="Not authorized to access this resource")
    
    return get_driver_license(driver_id)

@router.post("/", response_model=DriverLicense)
def create_driver_license_endpoint(request: Request, license: DriverLicenseCreate, current_user: dict = Depends(get_current_user_from_header)):
    # Check if user is admin - if so, allow creating licenses
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    # For regular users, set driver_id to their own user ID
    # For admins, allow setting driver_id to any user
    if is_admin and hasattr(license, 'driver_id') and license.driver_id:
        driver_id = license.driver_id
    else:
        driver_id = current_user['id']
    
    # Проверяем, существуют ли уже права у пользователя
    try:
        existing_license = get_driver_license(driver_id)
        # Если права уже существуют, обновляем их
        update_data = DriverLicenseUpdate(
            license_number=license.license_number,
            issued_by=license.issued_by,
            expiration_date=license.expiration_date,
            document_photo_id=license.document_photo_id,
            document_photo_back_id=license.document_photo_back_id,
            status='pending'  # Сбрасываем статус в pending при обновлении
        )
        return update_driver_license(driver_id, update_data)
    except:
        # Если прав не существует, создаем новые
        return create_driver_license(license, driver_id)

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