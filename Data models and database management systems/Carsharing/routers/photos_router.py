from fastapi import APIRouter, HTTPException
from schemas import *
from crud.photos_crud import *
from typing import List

router = APIRouter(prefix="/photos", tags=["Photos"])

@router.get("/", response_model=List[Photo])
def get_photos_endpoint(offset: int = 0, limit: int = 10):
    return get_photos(offset, limit)

@router.get("/{photo_id}", response_model=Photo)
def get_photo_endpoint(photo_id: int):
    return get_photo(photo_id)

@router.post("/", response_model=Photo)
def create_photo_endpoint(photo: PhotoCreate):
    return create_photo(photo)

@router.put("/{photo_id}", response_model=Photo)
def update_photo_endpoint(photo_id: int, photo: PhotoUpdate):
    return update_photo(photo_id, photo)

@router.delete("/{photo_id}")
def delete_photo_endpoint(photo_id: int):
    return delete_photo(photo_id)