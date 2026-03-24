from pydantic import BaseModel, EmailStr, field_validator
from typing import List, Optional, Union, Any
from datetime import datetime


# =============================================================================
# Authentication schemas
# =============================================================================

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


class Token(BaseModel):
    access_token: str
    token_type: str = "bearer"


class TokenData(BaseModel):
    user_id: Optional[int] = None
    email: Optional[str] = None


# =============================================================================
# Role schemas
# =============================================================================

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


# =============================================================================
# User schemas
# =============================================================================

class UserBase(BaseModel):
    email: str
    hashed_password: str
    name: str
    surname: str
    cashback: Optional[float] = 0
    role_id: int
    status: Optional[str] = "active"


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


class UserWithRoleName(UserBase):
    id: int
    created_at: datetime
    updated_at: datetime
    role_name: Optional[str] = None

    class Config:
        from_attributes = True


# =============================================================================
# Car schemas
# =============================================================================

class CarBase(BaseModel):
    vin: str
    plate_number: str
    model: str
    status: Optional[str] = "available"
    position: Optional[str] = None  # GeoJSON format
    main_photo_id: Optional[int] = None


class CarCreate(CarBase):
    status: str = "available"  # Always set to 'available' during creation


class CarUpdate(BaseModel):
    vin: Optional[str] = None
    plate_number: Optional[str] = None
    model: Optional[str] = None
    status: Optional[str] = None
    position: Optional[str] = None
    main_photo_id: Optional[int] = None


class Car(CarBase):
    id: int
    updated_at: datetime

    class Config:
        from_attributes = True


# =============================================================================
# Photo schemas
# =============================================================================

class PhotoBase(BaseModel):
    object_type: str
    user_id: Optional[int] = None
    car_id: Optional[int] = None
    uploaded_by: Optional[int] = None


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


# =============================================================================
# Driver License schemas
# =============================================================================

class DriverLicenseBase(BaseModel):
    license_number: str
    issued_by: str
    expiration_date: datetime
    document_photo_id: int
    document_photo_back_id: Optional[int] = None
    status: Optional[str] = "pending"


class DriverLicenseCreate(DriverLicenseBase):
    driver_id: Optional[int] = None


class DriverLicenseUpdate(BaseModel):
    issued_by: Optional[str] = None
    expiration_date: Optional[datetime] = None
    document_photo_id: Optional[int] = None
    document_photo_back_id: Optional[int] = None
    status: Optional[str] = None


class DriverLicense(DriverLicenseBase):
    driver_id: int

    class Config:
        from_attributes = True


# =============================================================================
# Session schemas
# =============================================================================

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


# =============================================================================
# Car State schemas
# =============================================================================

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


# =============================================================================
# Rental schemas
# =============================================================================

class RentalBase(BaseModel):
    user_id: Optional[int] = None
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


class RentalWithUserAndCarInfo(Rental):
    email: str
    vin: str

    class Config:
        from_attributes = True


# =============================================================================
# Maintenance Request schemas
# =============================================================================

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


class MaintenanceRequestWithCarInfo(MaintenanceRequest):
    vin: Optional[str] = None
    reported_by_email: Optional[str] = None


# =============================================================================
# Payment Log schemas
# =============================================================================

class PaymentLogBase(BaseModel):
    rental_id: int
    user_id: int
    user_email: Optional[str] = None
    pay_type: str
    price: float
    card_number: Optional[str] = None


class PaymentLogCreate(PaymentLogBase):
    user_email: Optional[str] = None

    class Config:
        from_attributes = True


class PaymentLog(PaymentLogBase):
    id: int

    class Config:
        from_attributes = True


# =============================================================================
# Log schemas
# =============================================================================

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


# =============================================================================
# Trip Completion schemas
# =============================================================================

class TripCompletionBase(BaseModel):
    rental_id: int
    admin_approved: Optional[bool] = None
    admin_comment: Optional[str] = None
    admin_reviewed_by: Optional[int] = None
    admin_reviewed_at: Optional[datetime] = None
    user_email: Optional[str] = None
    car_vin: Optional[str] = None


class TripCompletionCreate(TripCompletionBase):
    rental_id: int
    completion_photo_ids: Optional[List[int]] = None


class TripCompletionUpdate(BaseModel):
    admin_approved: Optional[bool] = None
    admin_comment: Optional[str] = None
    admin_reviewed_by: Optional[int] = None
    admin_reviewed_at: Optional[datetime] = None


class TripCompletion(TripCompletionBase):
    id: int
    completion_photo_ids: Optional[List[int]] = None
    created_at: datetime

    class Config:
        from_attributes = True


# =============================================================================
# Action Log schemas
# =============================================================================

class ActionLogBase(BaseModel):
    actor_user_id: int
    action_type: str
    target_user_id: Optional[int] = None
    target_car_id: Optional[int] = None
    target_rental_id: Optional[int] = None
    description: Optional[str] = None
    old_values: Optional[dict] = None
    new_values: Optional[dict] = None
    user_agent: Optional[str] = None


class ActionLogCreate(ActionLogBase):
    pass


class ActionLog(ActionLogBase):
    id: int
    created_at: datetime

    class Config:
        from_attributes = True


class ActionLogWithEmails(ActionLog):
    actor_email: Optional[str] = None
    target_user_email: Optional[str] = None
    target_car_vin: Optional[str] = None


# =============================================================================
# MongoDB Log schemas (for action logs, DB query logs, error logs)
# =============================================================================

class ActionLogMongo(BaseModel):
    """Schema for action logs stored in MongoDB."""
    id: Optional[str] = None  # MongoDB ObjectId
    actor_user_id: Union[int, str]
    actor_email: Optional[str] = None
    action_type: str
    target_user_id: Optional[Union[int, str]] = None
    target_user_email: Optional[str] = None
    target_car_id: Optional[Union[int, str]] = None
    target_car_vin: Optional[str] = None
    target_rental_id: Optional[Union[int, str]] = None
    description: Optional[str] = None
    old_values: Optional[dict] = None
    new_values: Optional[dict] = None
    user_agent: Optional[str] = None
    ip_address: Optional[str] = None
    created_at: datetime

    class Config:
        from_attributes = True


class DbQueryLogMongo(BaseModel):
    """Schema for database query logs stored in MongoDB."""
    id: Optional[str] = None  # MongoDB ObjectId
    query: str
    query_type: str  # SELECT, INSERT, UPDATE, DELETE
    table_name: Optional[str] = None
    execution_time_ms: Optional[float] = None
    rows_affected: Optional[int] = None
    user_id: Optional[int] = None
    endpoint: Optional[str] = None
    ip_address: Optional[str] = None
    created_at: datetime

    class Config:
        from_attributes = True


class ErrorLogMongo(BaseModel):
    """Schema for error logs stored in MongoDB."""
    id: Optional[str] = None  # MongoDB ObjectId
    error_type: str  # Exception class name
    error_message: str
    stack_trace: Optional[str] = None
    endpoint: Optional[str] = None
    user_id: Optional[int] = None
    user_email: Optional[str] = None
    request_method: Optional[str] = None
    request_url: Optional[str] = None
    request_body: Optional[dict] = None
    ip_address: Optional[str] = None
    severity: str = "ERROR"  # DEBUG, INFO, WARNING, ERROR, CRITICAL
    created_at: datetime

    class Config:
        from_attributes = True


class LogFilter(BaseModel):
    """Schema for filtering logs."""
    start_date: Optional[datetime] = None
    end_date: Optional[datetime] = None
    user_id: Optional[int] = None
    user_email: Optional[str] = None
    action_type: Optional[str] = None
    event_type: Optional[str] = None  # For error logs
    severity: Optional[str] = None  # For error logs
    endpoint: Optional[str] = None
    table_name: Optional[str] = None  # For DB query logs