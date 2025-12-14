from fastapi import APIRouter, HTTPException, Depends, Request
from schemas import *
from crud.cars_crud import *
from typing import List
from .users_router import get_current_user_from_header

router = APIRouter(prefix="/cars", tags=["Автомобили"])

@router.get("/", response_model=List[Car])
def get_cars_endpoint(offset: int = 0, limit: int = 1000):
    return get_cars(offset, limit)

@router.get("/{car_id}", response_model=Car)
def get_car_endpoint(car_id: int):
    if car_id <= 0:
        raise HTTPException(status_code=400, detail="Car ID must be a positive integer")
    return get_car(car_id)

@router.post("/", response_model=Car)
def create_car_endpoint(car: CarCreate):
    return create_car(car)

@router.put("/{car_id}", response_model=Car)
def update_car_endpoint(car_id: int, car: CarUpdate):
    if car_id <= 0:
        raise HTTPException(status_code=400, detail="Car ID must be a positive integer")
    return update_car(car_id, car)

@router.delete("/{car_id}")
def delete_car_endpoint(car_id: int):
    if car_id <= 0:
        raise HTTPException(status_code=400, detail="Car ID must be a positive integer")
    return delete_car(car_id)

class CarPosition(BaseModel):
    id: int
    vin: str
    plate_number: str
    model: str
    status: str
    position_text: Optional[str] = None
    longitude: Optional[float] = None
    latitude: Optional[float] = None

@router.get("/all/positions", response_model=List[CarPosition])
def get_cars_positions_with_user_rental_status_endpoint(request: Request, current_user: dict = Depends(get_current_user_from_header)):
    """Возвращает все машины с информацией о том, арендована ли машина пользователем"""
    user_id = current_user['id']
    return get_cars_positions_with_user_rental_status(user_id)