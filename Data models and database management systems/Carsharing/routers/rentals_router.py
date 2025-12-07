from fastapi import APIRouter, HTTPException, Depends, Request
from schemas import *
from crud.rentals_crud import *
from typing import List
from .users_router import get_current_user_from_header

router = APIRouter(prefix="/rentals", tags=["Rentals"])

@router.get("/", response_model=List[Rental])
def get_rentals_endpoint(offset: int = 0, limit: int = 10):
    return get_rentals(offset, limit)

@router.get("/user/{user_id}", response_model=List[Rental])
def get_rentals_by_user_endpoint(user_id: int, offset: int = 0, limit: int = 10):
    return get_rentals_by_user_id(user_id, offset, limit)

@router.get("/{rental_id}", response_model=Rental)
def get_rental_endpoint(rental_id: int):
    return get_rental(rental_id)

@router.post("/", response_model=Rental)
def create_rental_endpoint(request: Request, rental: RentalCreate, current_user: dict = Depends(get_current_user_from_header)):
    # Use the user_id from the header instead of the rental object for security
    rental.user_id = current_user['id']
    return create_rental(rental)

@router.put("/{rental_id}", response_model=Rental)
def update_rental_endpoint(request: Request, rental_id: int, rental: RentalUpdate, current_user: dict = Depends(get_current_user_from_header)):
    # Проверяем, что статус не является 'paused', так как это недопустимое значение
    if rental.status == "paused":
        raise HTTPException(status_code=400, detail="Invalid status: 'paused' is not allowed")
    
    # Check if user is admin - if so, allow updating any rental
    # Otherwise, ensure user can only update their own rentals
    from crud.users_crud import get_user
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    if not is_admin:
        # First get the rental to check if it belongs to the user
        from crud.rentals_crud import get_rental
        rental_obj = get_rental(rental_id)
        if rental_obj['user_id'] != current_user['id']:
            raise HTTPException(status_code=403, detail="Not authorized to update this rental")
    
    return update_rental(rental_id, rental)

@router.delete("/{rental_id}")
def delete_rental_endpoint(request: Request, rental_id: int, current_user: dict = Depends(get_current_user_from_header)):
    # Check if user is admin - if so, allow deleting any rental
    # Otherwise, ensure user can only delete their own rentals
    from crud.users_crud import get_user
    from crud.rentals_crud import get_rental
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    if not is_admin:
        # First get the rental to check if it belongs to the user
        rental_obj = get_rental(rental_id)
        if rental_obj['user_id'] != current_user['id']:
            raise HTTPException(status_code=403, detail="Not authorized to delete this rental")
    
    return delete_rental(rental_id)