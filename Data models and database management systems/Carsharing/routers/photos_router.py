from fastapi import APIRouter, HTTPException, File, UploadFile
from schemas import *
from crud.photos_crud import *
from typing import List
import os
from datetime import datetime

router = APIRouter(prefix="/photos", tags=["Photos"])

@router.get("/", response_model=List[Photo])
def get_photos_endpoint(offset: int = 0, limit: int = 10):
    photos = get_photos(offset, limit)
    # Convert dict-like objects to Photo models if needed
    if photos and isinstance(photos[0], dict):
        return [Photo.model_validate(photo) for photo in photos]
    return photos

@router.get("/{photo_id}", response_model=Photo)
def get_photo_endpoint(photo_id: int):
    photo = get_photo(photo_id)
    # Convert dict-like object to Photo model if needed
    if isinstance(photo, dict):
        return Photo.model_validate(photo)
    return photo

@router.get("/file/{photo_id}")
async def get_photo_file_endpoint(photo_id: int):
    """
    Retrieve photo file from database
    """
    from fastapi.responses import StreamingResponse
    import io
    import urllib.parse
    
    photo = get_photo(photo_id)
    
    # Handle both dict-like objects and Pydantic models
    if isinstance(photo, dict):
        file_data = photo['file_data']
        content_type = photo['content_type']
        filename = photo['filename']
    else:
        file_data = photo.file_data
        content_type = photo.content_type
        filename = photo.filename
    
    # Properly encode the filename for the Content-Disposition header
    encoded_filename = urllib.parse.quote(filename, safe='')
    
    # Create a BytesIO stream from the file data
    def iterfile():
        yield file_data
    
    return StreamingResponse(
        iterfile(),
        media_type=content_type,
        headers={
            "Content-Disposition": f"inline; filename*=UTF-8''{encoded_filename}"
        }
    )

@router.post("/", response_model=Photo)
def create_photo_endpoint(photo: PhotoCreate):
    created_photo = create_photo(photo)
    # Convert dict-like object to Photo model if needed
    if isinstance(created_photo, dict):
        return Photo.model_validate(created_photo)
    return created_photo

@router.post("/upload", response_model=Photo)
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
    
    # Create photo record in database ensuring constraint compliance
    # Ensure that when object_type is 'document' or 'driver', user_id is not null to satisfy the constraint
    effective_user_id = user_id or uploaded_by or 1
    effective_object_type = object_type or "document"
    
    if effective_object_type in ['document', 'driver']:
        # For document/driver photos, ensure user_id is set
        effective_user_id = effective_user_id
        effective_car_id = None
    elif effective_object_type == 'car':
        # For car photos, ensure car_id is set and user_id is null
        if car_id is None:
            raise HTTPException(status_code=400, detail="Car photos must have a car_id")
        effective_car_id = car_id
        effective_user_id = None
    else:
        # For other types, use provided values or defaults
        effective_car_id = car_id
    
    photo_create = PhotoCreate(
        object_type=effective_object_type,
        user_id=effective_user_id,
        car_id=effective_car_id,
        file_data=content,
        filename=file.filename,
        content_type=file.content_type,
        file_size=len(content),
        uploaded_by=uploaded_by or 1
    )
    
    created_photo = create_photo(photo_create)
    return created_photo

@router.put("/{photo_id}", response_model=Photo)
def update_photo_endpoint(photo_id: int, photo: PhotoUpdate):
    updated_photo = update_photo(photo_id, photo)
    # Convert dict-like object to Photo model if needed
    if isinstance(updated_photo, dict):
        return Photo.model_validate(updated_photo)
    return updated_photo

@router.delete("/{photo_id}")
def delete_photo_endpoint(photo_id: int):
    return delete_photo(photo_id)

@router.get("/trip-completion/{trip_completion_id}", response_model=List[Photo])
def get_photos_by_trip_completion_endpoint(trip_completion_id: int):
    """
    Get all photos associated with a trip completion
    """
    photos = get_photos_by_trip_completion_id(trip_completion_id)
    # Convert dict-like objects to Photo models if needed
    if photos and isinstance(photos[0], dict):
        return [Photo.model_validate(photo) for photo in photos]
    return photos