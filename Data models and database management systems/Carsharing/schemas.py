from pydantic import BaseModel, field_validator
from typing import List, Optional
from datetime import datetime

# Pydantic models
class RoleBase(BaseModel):
    name: str
    description: Optional[str] = None

class RoleCreate(RoleBase):
    pass

class RoleUpdate(RoleBase):
    pass

class Role(RoleBase):
    id: int
    
    class Config:
        from_attributes = True

class UserBase(BaseModel):
    email: str
    hashed_password: str
    name: str
    surname: str
    cashback: Optional[float] = 0
    role_id: int
    status: Optional[str] = "active"

from pydantic import BaseModel, EmailStr

class UserRegistration(BaseModel):
    email: EmailStr
    password: str  # plain text password
    name: str
    surname: str
    cashback: Optional[float] = 0
    role_id: int = 2  # по умолчанию (user role)
    status: Optional[str] = "active"
    
    @field_validator('role_id')
    @classmethod
    def validate_role_id(cls, v):
        if v not in [1, 2]:  # Only allow admin (1) and user (2) roles
            raise ValueError('role_id должен быть 1 (администратор) или 2 (пользователь)')
        return v

class UserLogin(BaseModel):
    email: str
    password: str

class UserPasswordChange(BaseModel):
    current_password: str
    new_password: str

class UserCreate(UserBase):
    role_id: int = 2  # по умолчанию (user role)
    
    @field_validator('role_id')
    @classmethod
    def validate_role_id(cls, v):
        if v not in [1, 2]:  # Only allow admin (1) and user (2) roles
            raise ValueError('role_id должен быть 1 (администратор) или 2 (пользователь)')
        return v

class UserUpdate(BaseModel):
    email: Optional[str] = None
    name: Optional[str] = None
    surname: Optional[str] = None
    cashback: Optional[float] = None
    role_id: Optional[int] = None
    status: Optional[str] = None
    
    @field_validator('role_id', mode='before')
    @classmethod
    def validate_role_id(cls, v):
        if v is not None and v not in [1, 2]:  # Only allow admin (1) and user (2) roles
            raise ValueError('role_id должен быть 1 (администратор) или 2 (пользователь)')
        return v

class User(UserBase):
    id: int
    created_at: datetime
    updated_at: datetime
    
    class Config:
        from_attributes = True

class CarBase(BaseModel):
    vin: str
    plate_number: str
    model: str
    status: Optional[str] = "available"
    position: Optional[str] = None  # GeoJSON format
    main_photo_id: Optional[int] = None

class CarCreate(CarBase):
    pass

class CarUpdate(BaseModel):
    model: Optional[str] = None
    status: Optional[str] = None
    position: Optional[str] = None
    main_photo_id: Optional[int] = None

class Car(CarBase):
    id: int
    updated_at: datetime
    
    class Config:
        from_attributes = True

class PhotoBase(BaseModel):
    object_type: str
    user_id: Optional[int] = None
    car_id: Optional[int] = None
    uploaded_by: int

class PhotoCreate(PhotoBase):
    file_data: bytes
    filename: str
    content_type: str
    file_size: int

class PhotoUpdate(BaseModel):
    uploaded_by: Optional[int] = None

class Photo(PhotoBase):
    id: int
    filename: str
    content_type: str
    file_size: int
    uploaded_at: datetime
    
    class Config:
        from_attributes = True

class DriverLicenseBase(BaseModel):
    license_number: str
    issued_by: str
    expiration_date: datetime  # Changed from str to datetime to handle date objects from DB
    document_photo_id: int
    document_photo_back_id: Optional[int] = None  # ID второй фотографии (например, обратной стороны)
    status: Optional[str] = "pending"

class DriverLicenseCreate(DriverLicenseBase):
    driver_id: Optional[int] = None  # Make driver_id optional for creation

class DriverLicenseUpdate(BaseModel):
    issued_by: Optional[str] = None
    expiration_date: Optional[datetime] = None  # Changed from str to datetime
    document_photo_id: Optional[int] = None
    document_photo_back_id: Optional[int] = None
    status: Optional[str] = None

class DriverLicense(DriverLicenseBase):
    driver_id: int
    
    class Config:
        from_attributes = True

class SessionBase(BaseModel):
    user_id: int

class SessionCreate(SessionBase):
    pass

class Session(SessionBase):
    id: int
    created_at: datetime
    updated_at: datetime
    
    class Config:
        from_attributes = True

class CarStateBase(BaseModel):
    car_id: int
    checked_by: Optional[int] = None
    verified: Optional[bool] = False
    comment: Optional[str] = None

class CarStateCreate(CarStateBase):
    pass

class CarStateUpdate(BaseModel):
    checked_by: Optional[int] = None
    verified: Optional[bool] = None
    comment: Optional[str] = None

class CarState(CarStateBase):
    id: int
    checked_at: datetime
    
    class Config:
        from_attributes = True

class RentalBase(BaseModel):
    user_id: int
    car_id: int
    started_at: Optional[datetime] = None
    price: float
    status: Optional[str] = "active"

class RentalCreate(RentalBase):
    pass

class RentalUpdate(BaseModel):
    ended_at: Optional[datetime] = None
    status: Optional[str] = None
    price: Optional[float] = None

class Rental(RentalBase):
    id: int
    ended_at: Optional[datetime] = None
    
    class Config:
        from_attributes = True

class RentalWithCarInfo(Rental):
    vin: str
    plate_number: str
    model: str
    main_photo_id: Optional[int] = None
    
    class Config:
        from_attributes = True

class MaintenanceRequestBase(BaseModel):
    car_id: int
    reported_by: Optional[int] = None
    status: Optional[str] = "open"
    description: str

class MaintenanceRequestCreate(MaintenanceRequestBase):
    pass

class MaintenanceRequestUpdate(BaseModel):
    reported_by: Optional[int] = None
    resolved_at: Optional[datetime] = None
    status: Optional[str] = None
    description: Optional[str] = None

class MaintenanceRequest(MaintenanceRequestBase):
    id: int
    created_at: datetime
    
    class Config:
        from_attributes = True

class PaymentLogBase(BaseModel):
    rental_id: int
    user_id: int
    pay_type: str
    price: float

class PaymentLogCreate(PaymentLogBase):
    pass

class PaymentLog(PaymentLogBase):
    id: int
    
    class Config:
        from_attributes = True

class LogBase(BaseModel):
    actor_user_id: int
    action_type: str
    target_id: Optional[int] = None

class LogCreate(LogBase):
    pass

class Log(LogBase):
    id: int
    created_at: datetime
    
    class Config:
        from_attributes = True

class TripCompletionBase(BaseModel):
    rental_id: int
    completion_photo_id: Optional[int] = None
    admin_approved: Optional[bool] = None
    admin_comment: Optional[str] = None
    admin_reviewed_by: Optional[int] = None
    admin_reviewed_at: Optional[datetime] = None

class TripCompletionCreate(TripCompletionBase):
    rental_id: int
    completion_photo_id: int  # Required for creation

class TripCompletionUpdate(BaseModel):
    admin_approved: Optional[bool] = None
    admin_comment: Optional[str] = None
    admin_reviewed_by: Optional[int] = None
    admin_reviewed_at: Optional[datetime] = None

class TripCompletion(TripCompletionBase):
    id: int
    created_at: datetime
    
    class Config:
        from_attributes = True

class ActionLogBase(BaseModel):
    actor_user_id: int
    action_type: str
    target_user_id: Optional[int] = None
    target_car_id: Optional[int] = None
    target_rental_id: Optional[int] = None
    description: Optional[str] = None
    old_values: Optional[dict] = None  # Changed from str to dict to handle JSON objects from DB
    new_values: Optional[dict] = None  # Changed from str to dict to handle JSON objects from DB
    user_agent: Optional[str] = None

class ActionLogCreate(ActionLogBase):
    pass

class ActionLog(ActionLogBase):
    id: int
    created_at: datetime
    
    class Config:
        from_attributes = True