from fastapi import FastAPI, HTTPException, Depends
from pydantic import BaseModel, field_validator
from typing import List, Optional
import psycopg2
from psycopg2.extras import RealDictCursor
from datetime import datetime
import os
from dotenv import load_dotenv
import bcrypt

load_dotenv()

app = FastAPI(title="Carsharing API", description="API for carsharing application", version="1.0.0")

# Database connection
def get_db_connection():
    conn = psycopg2.connect(
        host=os.getenv("DB_HOST", "localhost"),
        database=os.getenv("DB_NAME", "carsharing_db"),
        user=os.getenv("DB_USER", "postgres"),
        password=os.getenv("DB_PASSWORD", "postgres"),
        port=os.getenv("DB_PORT", 5432)
    )
    return conn

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

class UserCreate(UserBase):
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

class CarCreate(CarBase):
    pass

class CarUpdate(BaseModel):
    model: Optional[str] = None
    status: Optional[str] = None
    position: Optional[str] = None

class Car(CarBase):
    id: int
    updated_at: datetime
    
    class Config:
        from_attributes = True

class PhotoBase(BaseModel):
    object_type: str
    user_id: Optional[int] = None
    car_id: Optional[int] = None
    url: str
    uploaded_by: int

class PhotoCreate(PhotoBase):
    pass

class PhotoUpdate(BaseModel):
    url: Optional[str] = None
    uploaded_by: Optional[int] = None

class Photo(PhotoBase):
    id: int
    uploaded_at: datetime
    
    class Config:
        from_attributes = True

class DriverLicenseBase(BaseModel):
    license_number: str
    issued_by: str
    expiration_date: str
    document_photo_id: int
    status: Optional[str] = "pending"

class DriverLicenseCreate(DriverLicenseBase):
    pass

class DriverLicenseUpdate(BaseModel):
    issued_by: Optional[str] = None
    expiration_date: Optional[str] = None
    document_photo_id: Optional[int] = None
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
    started_at: datetime
    price: float
    status: Optional[str] = "active"

class RentalCreate(RentalBase):
    pass

class RentalUpdate(BaseModel):
    ended_at: Optional[datetime] = None
    status: Optional[str] = None

class Rental(RentalBase):
    id: int
    
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
    card_number: Optional[str] = None

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

class ActionLogBase(BaseModel):
    actor_user_id: int
    action_type: str
    target_user_id: Optional[int] = None
    target_car_id: Optional[int] = None
    target_rental_id: Optional[int] = None
    description: Optional[str] = None
    old_values: Optional[str] = None
    new_values: Optional[str] = None
    user_agent: Optional[str] = None

class ActionLogCreate(ActionLogBase):
    pass

class ActionLog(ActionLogBase):
    id: int
    created_at: datetime
    
    class Config:
        from_attributes = True

# Roles CRUD
@app.get("/roles/", response_model=List[Role])
def get_roles(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM roles ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    roles = cur.fetchall()
    cur.close()
    conn.close()
    return roles

@app.get("/roles/{role_id}", response_model=Role)
def get_role(role_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM roles WHERE id = %s", (role_id,))
    role = cur.fetchone()
    cur.close()
    conn.close()
    if not role:
        raise HTTPException(status_code=404, detail="Роль не найдена")
    return role

@app.post("/roles/", response_model=Role)
def create_role(role: RoleCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        "INSERT INTO roles (name, description) VALUES (%s, %s) RETURNING *",
        (role.name, role.description)
    )
    new_role = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_role

@app.put("/roles/{role_id}", response_model=Role)
def update_role(role_id: int, role: RoleUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        "UPDATE roles SET name = %s, description = %s WHERE id = %s RETURNING *",
        (role.name, role.description, role_id)
    )
    updated_role = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_role:
        raise HTTPException(status_code=404, detail="Роль не найдена")
    return updated_role

@app.delete("/roles/{role_id}")
def delete_role(role_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM roles WHERE id = %s", (role_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Роль не найдена")
    return {"message": "Роль успешно удалена"}

# Users CRUD
@app.get("/users/", response_model=List[User])
def get_users(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM users ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    users = cur.fetchall()
    cur.close()
    conn.close()
    return users

@app.get("/users/{user_id}", response_model=User)
def get_user(user_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM users WHERE id = %s", (user_id,))
    user = cur.fetchone()
    cur.close()
    conn.close()
    if not user:
        raise HTTPException(status_code=404, detail="Пользователь не найден")
    return user

@app.post("/users/", response_model=User)
def create_user(user: UserCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO users (email, hashed_password, name, surname, cashback, role_id, status) 
           VALUES (%s, %s, %s, %s, %s, %s, %s) RETURNING *""",
        (user.email, user.hashed_password, user.name, user.surname, user.cashback, user.role_id, user.status)
    )
    new_user = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_user

@app.put("/users/{user_id}", response_model=User)
def update_user(user_id: int, user: UserUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if user.email is not None:
        update_fields.append("email = %s")
        values.append(user.email)
    if user.name is not None:
        update_fields.append("name = %s")
        values.append(user.name)
    if user.surname is not None:
        update_fields.append("surname = %s")
        values.append(user.surname)
    if user.cashback is not None:
        update_fields.append("cashback = %s")
        values.append(user.cashback)
    if user.role_id is not None:
        update_fields.append("role_id = %s")
        values.append(user.role_id)
    if user.status is not None:
        update_fields.append("status = %s")
        values.append(user.status)
    
    update_fields.append("updated_at = CURRENT_TIMESTAMP")
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE users SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(user_id)
    
    cur.execute(query, values)
    updated_user = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_user:
        raise HTTPException(status_code=404, detail="Пользователь не найден")
    return updated_user

@app.delete("/users/{user_id}")
def delete_user(user_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM users WHERE id = %s", (user_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Пользователь не найден")
    return {"message": "Пользователь успешно удален"}

# Cars CRUD
@app.get("/cars/", response_model=List[Car])
def get_cars(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM cars ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    cars = cur.fetchall()
    cur.close()
    conn.close()
    return cars

# The cars router is now in the separate file, so we remove this endpoint from main app
# @app.get("/cars/all/positions", response_model=List[dict])
# def get_all_cars_positions(user_id: int):
#     """
#     Returns all cars with their positions and information about whether the car is rented by the user
#     """
#     from crud.cars_crud import get_all_cars_positions as get_cars_positions_crud
#     return get_cars_positions_crud(user_id)

@app.get("/cars/{car_id}", response_model=Car)
def get_car(car_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM cars WHERE id = %s", (car_id,))
    car = cur.fetchone()
    cur.close()
    conn.close()
    if not car:
        raise HTTPException(status_code=404, detail="Автомобиль не найден")
    return car

@app.post("/cars/", response_model=Car)
def create_car(car: CarCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    if car.position:
        cur.execute(
            """INSERT INTO cars (vin, plate_number, model, status, position) 
               VALUES (%s, %s, %s, %s, ST_GeomFromText(%s, 4326)) RETURNING *""",
            (car.vin, car.plate_number, car.model, car.status, car.position)
        )
    else:
        cur.execute(
            """INSERT INTO cars (vin, plate_number, model, status) 
               VALUES (%s, %s, %s, %s) RETURNING *""",
            (car.vin, car.plate_number, car.model, car.status)
        )
    new_car = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_car

@app.put("/cars/{car_id}", response_model=Car)
def update_car(car_id: int, car: CarUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if car.model is not None:
        update_fields.append("model = %s")
        values.append(car.model)
    if car.status is not None:
        update_fields.append("status = %s")
        values.append(car.status)
    if car.position is not None:
        update_fields.append("position = ST_GeomFromText(%s, 4326)")
        values.append(car.position)
    
    update_fields.append("updated_at = CURRENT_TIMESTAMP")
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE cars SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(car_id)
    
    cur.execute(query, values)
    updated_car = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_car:
        raise HTTPException(status_code=404, detail="Автомобиль не найден")
    return updated_car

@app.delete("/cars/{car_id}")
def delete_car(car_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM cars WHERE id = %s", (car_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Автомобиль не найден")
    return {"message": "Автомобиль успешно удален"}

# Photos CRUD
@app.get("/photos/", response_model=List[Photo])
def get_photos(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM photos ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    photos = cur.fetchall()
    cur.close()
    conn.close()
    return photos

@app.get("/photos/{photo_id}", response_model=Photo)
def get_photo(photo_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM photos WHERE id = %s", (photo_id,))
    photo = cur.fetchone()
    cur.close()
    conn.close()
    if not photo:
        raise HTTPException(status_code=404, detail="Фото не найдено")
    return photo

@app.post("/photos/", response_model=Photo)
def create_photo(photo: PhotoCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO photos (object_type, user_id, car_id, url, uploaded_by) 
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (photo.object_type, photo.user_id, photo.car_id, photo.url, photo.uploaded_by)
    )
    new_photo = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_photo

@app.put("/photos/{photo_id}", response_model=Photo)
def update_photo(photo_id: int, photo: PhotoUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if photo.url is not None:
        update_fields.append("url = %s")
        values.append(photo.url)
    if photo.uploaded_by is not None:
        update_fields.append("uploaded_by = %s")
        values.append(photo.uploaded_by)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE photos SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(photo_id)
    
    cur.execute(query, values)
    updated_photo = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_photo:
        raise HTTPException(status_code=404, detail="Фото не найдено")
    return updated_photo

@app.delete("/photos/{photo_id}")
def delete_photo(photo_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM photos WHERE id = %s", (photo_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Фото не найдено")
    return {"message": "Фото успешно удалено"}

# Driver Licenses CRUD
@app.get("/driver_licenses/", response_model=List[DriverLicense])
def get_driver_licenses(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM driver_licenses ORDER BY driver_id LIMIT %s OFFSET %s", (limit, skip))
    licenses = cur.fetchall()
    cur.close()
    conn.close()
    return licenses

@app.get("/driver_licenses/{driver_id}", response_model=DriverLicense)
def get_driver_license(driver_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM driver_licenses WHERE driver_id = %s", (driver_id,))
    license = cur.fetchone()
    cur.close()
    conn.close()
    if not license:
        raise HTTPException(status_code=404, detail="Водительское удостоверение не найдено")
    return license

@app.post("/driver_licenses/", response_model=DriverLicense)
def create_driver_license(license: DriverLicenseCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO driver_licenses (license_number, issued_by, expiration_date, document_photo_id, status) 
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (license.license_number, license.issued_by, license.expiration_date, license.document_photo_id, license.status)
    )
    new_license = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_license

@app.put("/driver_licenses/{driver_id}", response_model=DriverLicense)
def update_driver_license(driver_id: int, license: DriverLicenseUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if license.issued_by is not None:
        update_fields.append("issued_by = %s")
        values.append(license.issued_by)
    if license.expiration_date is not None:
        update_fields.append("expiration_date = %s")
        values.append(license.expiration_date)
    if license.document_photo_id is not None:
        update_fields.append("document_photo_id = %s")
        values.append(license.document_photo_id)
    if license.status is not None:
        update_fields.append("status = %s")
        values.append(license.status)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE driver_licenses SET {', '.join(update_fields)} WHERE driver_id = %s RETURNING *"
    values.append(driver_id)
    
    cur.execute(query, values)
    updated_license = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_license:
        raise HTTPException(status_code=404, detail="Водительское удостоверение не найдено")
    return updated_license

@app.delete("/driver_licenses/{driver_id}")
def delete_driver_license(driver_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM driver_licenses WHERE driver_id = %s", (driver_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Водительское удостоверение не найдено")
    return {"message": "Водительское удостоверение успешно удалено"}

# Sessions CRUD
@app.get("/sessions/", response_model=List[Session])
def get_sessions(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM sessions ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    sessions = cur.fetchall()
    cur.close()
    conn.close()
    return sessions

@app.get("/sessions/{session_id}", response_model=Session)
def get_session(session_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM sessions WHERE id = %s", (session_id,))
    session = cur.fetchone()
    cur.close()
    conn.close()
    if not session:
        raise HTTPException(status_code=404, detail="Сессия не найдена")
    return session

@app.post("/sessions/", response_model=Session)
def create_session(session: SessionCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO sessions (user_id)
           VALUES (%s) RETURNING *""",
        (session.user_id,)
    )
    new_session = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_session

@app.put("/sessions/{session_id}", response_model=Session)
def update_session(session_id: int, session: SessionBase):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        "UPDATE sessions SET user_id = %s, updated_at = CURRENT_TIMESTAMP WHERE id = %s RETURNING *",
        (session.user_id, session_id)
    )
    updated_session = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_session:
        raise HTTPException(status_code=404, detail="Сессия не найдена")
    return updated_session

@app.delete("/sessions/{session_id}")
def delete_session(session_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM sessions WHERE id = %s", (session_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Сессия не найдена")
    return {"message": "Сессия успешно удалена"}

# Car States CRUD
@app.get("/car_states/", response_model=List[CarState])
def get_car_states(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM car_states ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    states = cur.fetchall()
    cur.close()
    conn.close()
    return states

@app.get("/car_states/{state_id}", response_model=CarState)
def get_car_state(state_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM car_states WHERE id = %s", (state_id,))
    state = cur.fetchone()
    cur.close()
    conn.close()
    if not state:
        raise HTTPException(status_code=404, detail="Состояние автомобиля не найдено")
    return state

@app.post("/car_states/", response_model=CarState)
def create_car_state(state: CarStateCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO car_states (car_id, checked_by, verified, comment) 
           VALUES (%s, %s, %s, %s) RETURNING *""",
        (state.car_id, state.checked_by, state.verified, state.comment)
    )
    new_state = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_state

@app.put("/car_states/{state_id}", response_model=CarState)
def update_car_state(state_id: int, state: CarStateUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if state.checked_by is not None:
        update_fields.append("checked_by = %s")
        values.append(state.checked_by)
    if state.verified is not None:
        update_fields.append("verified = %s")
        values.append(state.verified)
    if state.comment is not None:
        update_fields.append("comment = %s")
        values.append(state.comment)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE car_states SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(state_id)
    
    cur.execute(query, values)
    updated_state = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_state:
        raise HTTPException(status_code=404, detail="Состояние автомобиля не найдено")
    return updated_state

@app.delete("/car_states/{state_id}")
def delete_car_state(state_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM car_states WHERE id = %s", (state_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Состояние автомобиля не найдено")
    return {"message": "Состояние автомобиля успешно удалено"}

# Rentals CRUD
@app.get("/rentals/", response_model=List[Rental])
def get_rentals(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM rentals ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    rentals = cur.fetchall()
    cur.close()
    conn.close()
    return rentals

@app.get("/rentals/{rental_id}", response_model=Rental)
def get_rental(rental_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM rentals WHERE id = %s", (rental_id,))
    rental = cur.fetchone()
    cur.close()
    conn.close()
    if not rental:
        raise HTTPException(status_code=404, detail="Аренда не найдена")
    return rental

@app.post("/rentals/", response_model=Rental)
def create_rental(rental: RentalCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO rentals (user_id, car_id, started_at, price, status) 
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (rental.user_id, rental.car_id, rental.started_at, rental.price, rental.status)
    )
    new_rental = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_rental

@app.put("/rentals/{rental_id}", response_model=Rental)
def update_rental(rental_id: int, rental: RentalUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if rental.ended_at is not None:
        update_fields.append("ended_at = %s")
        values.append(rental.ended_at)
    if rental.status is not None:
        update_fields.append("status = %s")
        values.append(rental.status)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE rentals SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(rental_id)
    
    cur.execute(query, values)
    updated_rental = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_rental:
        raise HTTPException(status_code=404, detail="Аренда не найдена")
    return updated_rental

@app.delete("/rentals/{rental_id}")
def delete_rental(rental_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM rentals WHERE id = %s", (rental_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Аренда не найдена")
    return {"message": "Аренда успешно удалена"}

# Maintenance Requests CRUD
@app.get("/maintenance_requests/", response_model=List[MaintenanceRequest])
def get_maintenance_requests(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM maintenance_requests ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    requests = cur.fetchall()
    cur.close()
    conn.close()
    return requests

@app.get("/maintenance_requests/{request_id}", response_model=MaintenanceRequest)
def get_maintenance_request(request_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM maintenance_requests WHERE id = %s", (request_id,))
    request = cur.fetchone()
    cur.close()
    conn.close()
    if not request:
        raise HTTPException(status_code=404, detail="Запрос на обслуживание не найден")
    return request

@app.post("/maintenance_requests/", response_model=MaintenanceRequest)
def create_maintenance_request(request: MaintenanceRequestCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO maintenance_requests (car_id, reported_by, status, description) 
           VALUES (%s, %s, %s, %s) RETURNING *""",
        (request.car_id, request.reported_by, request.status, request.description)
    )
    new_request = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_request

@app.put("/maintenance_requests/{request_id}", response_model=MaintenanceRequest)
def update_maintenance_request(request_id: int, request: MaintenanceRequestUpdate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    
    # Build dynamic update query
    update_fields = []
    values = []
    
    if request.reported_by is not None:
        update_fields.append("reported_by = %s")
        values.append(request.reported_by)
    if request.resolved_at is not None:
        update_fields.append("resolved_at = %s")
        values.append(request.resolved_at)
    if request.status is not None:
        update_fields.append("status = %s")
        values.append(request.status)
    if request.description is not None:
        update_fields.append("description = %s")
        values.append(request.description)
    
    if not update_fields:
        raise HTTPException(status_code=400, detail="Нет полей для обновления")
    
    query = f"UPDATE maintenance_requests SET {', '.join(update_fields)} WHERE id = %s RETURNING *"
    values.append(request_id)
    
    cur.execute(query, values)
    updated_request = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    if not updated_request:
        raise HTTPException(status_code=404, detail="Запрос на обслуживание не найден")
    return updated_request

@app.delete("/maintenance_requests/{request_id}")
def delete_maintenance_request(request_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM maintenance_requests WHERE id = %s", (request_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Запрос на обслуживание не найден")
    return {"message": "Запрос на обслуживание успешно удален"}

# Payment Logs CRUD
@app.get("/payment_logs/", response_model=List[PaymentLog])
def get_payment_logs(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM payment_logs ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    logs = cur.fetchall()
    cur.close()
    conn.close()
    return logs

@app.get("/payment_logs/{log_id}", response_model=PaymentLog)
def get_payment_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM payment_logs WHERE id = %s", (log_id,))
    log = cur.fetchone()
    cur.close()
    conn.close()
    if not log:
        raise HTTPException(status_code=404, detail="Лог платежа не найден")
    return log

@app.post("/payment_logs/", response_model=PaymentLog)
def create_payment_log(log: PaymentLogCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO payment_logs (rental_id, user_id, pay_type, card_number, price)
           VALUES (%s, %s, %s, %s, %s) RETURNING *""",
        (log.rental_id, log.user_id, log.pay_type, log.card_number, log.price)
    )
    new_log = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_log

@app.delete("/payment_logs/{log_id}")
def delete_payment_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM payment_logs WHERE id = %s", (log_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Лог платежа не найден")
    return {"message": "Лог платежа успешно удален"}

# Logs CRUD
@app.get("/logs/", response_model=List[Log])
def get_logs(skip: int = 0, limit: int = 100):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM logs ORDER BY id LIMIT %s OFFSET %s", (limit, skip))
    logs = cur.fetchall()
    cur.close()
    conn.close()
    return logs

@app.get("/logs/{log_id}", response_model=Log)
def get_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute("SELECT * FROM logs WHERE id = %s", (log_id,))
    log = cur.fetchone()
    cur.close()
    conn.close()
    if not log:
        raise HTTPException(status_code=404, detail="Лог не найден")
    return log

@app.post("/logs/", response_model=Log)
def create_log(log: LogCreate):
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        """INSERT INTO logs (actor_user_id, action_type, target_id)
           VALUES (%s, %s, %s) RETURNING *""",
        (log.actor_user_id, log.action_type, log.target_id)
    )
    new_log = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    return new_log

@app.delete("/logs/{log_id}")
def delete_log(log_id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("DELETE FROM logs WHERE id = %s", (log_id,))
    conn.commit()
    deleted_count = cur.rowcount
    cur.close()
    conn.close()
    if deleted_count == 0:
        raise HTTPException(status_code=404, detail="Лог не найден")
    return {"message": "Лог успешно удален"}


# Utility function for password hashing
def hash_password(password: str) -> str:
    """Хеширует пароль с использованием bcrypt"""
    return bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')

def verify_password(plain_password: str, hashed_password: str) -> bool:
    """Проверяет пароль, сравнивая его с хешем"""
    return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password.encode('utf-8'))

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)