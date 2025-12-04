from fastapi import APIRouter, HTTPException
from schemas import *
from crud.cars_crud import *
from typing import List

router = APIRouter(prefix="/cars", tags=["Cars"])

@router.get("/", response_model=List[Car])
def get_cars_endpoint(offset: int = 0, limit: int = 10):
    return get_cars(offset, limit)

@router.get("/{car_id}", response_model=Car)
def get_car_endpoint(car_id: int):
    return get_car(car_id)

@router.post("/", response_model=Car)
def create_car_endpoint(car: CarCreate):
    return create_car(car)

@router.put("/{car_id}", response_model=Car)
def update_car_endpoint(car_id: int, car: CarUpdate):
    return update_car(car_id, car)

@router.delete("/{car_id}")
def delete_car_endpoint(car_id: int):
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
def get_all_cars_positions_endpoint():
    """Возвращает все машины с их позициями"""
    return get_all_cars_positions()