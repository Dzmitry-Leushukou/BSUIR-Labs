from fastapi import APIRouter, HTTPException
from schemas import *
from crud.rentals_crud import *
from typing import List

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
def create_rental_endpoint(rental: RentalCreate):
    return create_rental(rental)

@router.put("/{rental_id}", response_model=Rental)
def update_rental_endpoint(rental_id: int, rental: RentalUpdate):
    # Проверяем, что статус не является 'paused', так как это недопустимое значение
    if rental.status == "paused":
        raise HTTPException(status_code=400, detail="Invalid status: 'paused' is not allowed")
    
    return update_rental(rental_id, rental)
@router.delete("/{rental_id}")
def delete_rental_endpoint(rental_id: int):
    return delete_rental(rental_id)