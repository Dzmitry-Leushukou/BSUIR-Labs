from fastapi import APIRouter, HTTPException, File, UploadFile
from schemas import *
from crud.photos_crud import *
from typing import List
import os
from datetime import datetime

router = APIRouter(prefix="/photos", tags=["Photos"])

@router.get("/", response_model=List[Photo])
def get_photos_endpoint(offset: int = 0, limit: int = 10):
    return get_photos(offset, limit)

@router.get("/{photo_id}", response_model=Photo)
def get_photo_endpoint(photo_id: int):
    return get_photo(photo_id)

@router.get("/file/{photo_id}")
async def get_photo_file_endpoint(photo_id: int):
    """
    Retrieve photo file from database
    """
    from fastapi.responses import Response
    photo = get_photo(photo_id)
    
    return Response(
        content=photo['file_data'],
        media_type=photo['content_type'],
        headers={
            "Content-Disposition": f"inline; filename={photo['filename']}"
        }
    )

@router.post("/", response_model=Photo)
def create_photo_endpoint(photo: PhotoCreate):
    return create_photo(photo)

@router.post("/upload")
async def upload_photo_endpoint(
    file: UploadFile = File(...),
    object_type: str = None,
    user_id: int = None,
    car_id: int = None,
    uploaded_by: int = None
):
    """
    Upload a photo file and create a record in the database
    """
    # Validate file type
    if not file.content_type.startswith("image/"):
        raise HTTPException(status_code=400, detail="File must be an image")
    
    # Read file content
    content = await file.read()
    
    # Create photo record in database
    photo_create = PhotoCreate(
        object_type=object_type or "general",
        user_id=user_id,
        car_id=car_id,
        file_data=content,
        filename=file.filename,
        content_type=file.content_type,
        file_size=len(content),
        uploaded_by=uploaded_by
    )
    
    created_photo = create_photo(photo_create)
    return created_photo

@router.put("/{photo_id}", response_model=Photo)
def update_photo_endpoint(photo_id: int, photo: PhotoUpdate):
    return update_photo(photo_id, photo)

@router.delete("/{photo_id}")
def delete_photo_endpoint(photo_id: int):
    return delete_photo(photo_id)