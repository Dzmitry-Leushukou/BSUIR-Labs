from fastapi import APIRouter, HTTPException
from schemas import *
from crud.car_states_crud import *
from typing import List

router = APIRouter(prefix="/car_states", tags=["Car States"])

@router.get("/", response_model=List[CarState])
def get_car_states_endpoint(offset: int = 0, limit: int = 10):
    return get_car_states(offset, limit)

@router.get("/{state_id}", response_model=CarState)
def get_car_state_endpoint(state_id: int):
    return get_car_state(state_id)

@router.post("/", response_model=CarState)
def create_car_state_endpoint(state: CarStateCreate):
    return create_car_state(state)

@router.put("/{state_id}", response_model=CarState)
def update_car_state_endpoint(state_id: int, state: CarStateUpdate):
    return update_car_state(state_id, state)

@router.delete("/{state_id}")
def delete_car_state_endpoint(state_id: int):
    return delete_car_state(state_id)