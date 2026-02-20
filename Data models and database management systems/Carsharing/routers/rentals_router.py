from fastapi import APIRouter, HTTPException, Depends, Request
from schemas import *
from crud.rentals_crud import *
from typing import List
from .users_router import get_current_user

router = APIRouter(prefix="/rentals", tags=["Аренды"])

@router.get("/", response_model=List[Rental])
def get_rentals_endpoint(offset: int = 0, limit: int = 10):
    return get_rentals(offset, limit)

@router.get("/user/{user_id}", response_model=List[Rental])
def get_rentals_by_user_endpoint(user_id: int, offset: int = 0, limit: int = 10):
    return get_rentals_by_user_id(user_id, offset, limit)

@router.get("/user/{user_id}/with-car-info", response_model=List[RentalWithCarInfo])
def get_rentals_with_car_info_by_user_endpoint(user_id: int, offset: int = 0, limit: int = 10):
    return get_rentals_with_car_info_by_user_id(user_id, offset, limit)

@router.get("/with-user-and-car-info", response_model=List[RentalWithUserAndCarInfo])
def get_rentals_with_user_and_car_info_endpoint(offset: int = 0, limit: int = 10):
    return get_rentals_with_user_and_car_info(offset, limit)

@router.get("/count", response_model=dict)
def get_rentals_count_endpoint(current_user: dict = Depends(get_current_user)):
    from crud.rentals_crud import get_rentals_count
    count = get_rentals_count()
    return {"count": count}

@router.get("/user/{user_id}/count", response_model=dict)
def get_rentals_count_by_user_endpoint(user_id: int, current_user: dict = Depends(get_current_user)):
    from crud.rentals_crud import get_rentals_count_by_user_id
    count = get_rentals_count_by_user_id(user_id)
    return {"count": count}

@router.get("/{rental_id}", response_model=Rental)
def get_rental_endpoint(rental_id: int):
    return get_rental(rental_id)

@router.post("/", response_model=Rental)
def create_rental_endpoint(request: Request, rental: RentalCreate, current_user: dict = Depends(get_current_user)):
    rental.user_id = current_user['id']

    from crud.driver_licenses_crud import get_driver_license
    from datetime import datetime
    try:
        license = get_driver_license(rental.user_id)
        if license['status'] != 'approved':
            raise HTTPException(status_code=403, detail="Водительские права не одобрены")
        if license['expiration_date'] < datetime.now().date():
            raise HTTPException(status_code=403, detail="Срок действия водительских прав истек")
    except HTTPException:
        raise HTTPException(status_code=403, detail="Пользователь не имеет действующих водительских прав")

    return create_rental(rental)

@router.put("/{rental_id}", response_model=Rental)
def update_rental_endpoint(request: Request, rental_id: int, rental: RentalUpdate, current_user: dict = Depends(get_current_user)):
    if rental.status == "paused":
        raise HTTPException(status_code=400, detail="Недопустимый статус: 'paused' не разрешен")

    from crud.users_crud import get_user
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    if not is_admin:
        from crud.rentals_crud import get_rental
        rental_obj = get_rental(rental_id)
        if rental_obj['user_id'] != current_user['id']:
            raise HTTPException(status_code=403, detail="Нет прав для обновления этой аренды")

    return update_rental(rental_id, rental)

@router.delete("/{rental_id}")
def delete_rental_endpoint(request: Request, rental_id: int, current_user: dict = Depends(get_current_user)):
    from crud.users_crud import get_user
    from crud.rentals_crud import get_rental
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    if not is_admin:
        rental_obj = get_rental(rental_id)
        if rental_obj['user_id'] != current_user['id']:
            raise HTTPException(status_code=403, detail="Нет прав для удаления этой аренды")

    return delete_rental(rental_id)