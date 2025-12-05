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
    
    # Create uploads directory if it doesn't exist
    upload_dir = "frontend/uploads"
    os.makedirs(upload_dir, exist_ok=True)
    
    # Generate unique filename
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    filename = f"{timestamp}_{file.filename}"
    filepath = os.path.join(upload_dir, filename)
    
    # Save file
    with open(filepath, "wb") as f:
        content = await file.read()
        f.write(content)
    
    # Create photo record in database
    photo_create = PhotoCreate(
        object_type=object_type or "general",
        user_id=user_id,
        car_id=car_id,
        url=f"/uploads/{filename}",
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