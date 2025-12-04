from fastapi import APIRouter, HTTPException
from schemas import *
from crud.users_crud import *
from typing import List
import bcrypt

def verify_password(plain_password: str, hashed_password: str) -> bool:
    """Проверяет пароль, сравнивая его с хешем"""
    return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password.encode('utf-8'))

def hash_password(password: str) -> str:
    """Хеширует пароль с использованием bcrypt"""
    return bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')

router = APIRouter(prefix="/users", tags=["Users"])

@router.get("/", response_model=List[User])
def get_users_endpoint(offset: int = 0, limit: int = 100):
    return get_users(offset, limit)

@router.get("/{user_id}", response_model=User)
def get_user_endpoint(user_id: int):
    return get_user(user_id)

@router.post("/", response_model=User)
def create_user_endpoint(user: UserCreate):
    return create_user(user)

@router.put("/{user_id}", response_model=User)
def update_user_endpoint(user_id: int, user: UserUpdate):
    return update_user(user_id, user)

@router.delete("/{user_id}")
def delete_user_endpoint(user_id: int):
    return delete_user(user_id)

@router.post("/login")
def login_user_endpoint(user_login: UserLogin):
    user = get_user_by_email(user_login.email)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    
    # Проверка хешированного пароля
    if not verify_password(user_login.password, user['hashed_password']):
        raise HTTPException(status_code=401, detail="Incorrect password")
    
    # Возвращаем информацию о пользователе без пароля
    user_data = dict(user)
    del user_data['hashed_password']
    return user_data

@router.post("/register")
def register_user_endpoint(user: UserRegistration):
    # Проверяем, существует ли уже пользователь с таким email
    existing_user = get_user_by_email(user.email)
    if existing_user:
        raise HTTPException(status_code=400, detail="User with this email already exists")
    
    # Хешируем пароль
    hashed_password = hash_password(user.password)
    
    # Создаем пользователя с хешированным паролем
    user_data = user.model_dump()
    user_data['hashed_password'] = hashed_password
    # Удаляем plain text пароль из данных пользователя
    del user_data['password']
    
    # Создаем пользователя в базе данных
    created_user = create_user(UserCreate(**user_data))
    
    # Возвращаем информацию о пользователе без пароля
    user_response = dict(created_user)
    del user_response['hashed_password']
    return user_response

@router.get("/profile")
def get_user_profile():
    # В реальном приложении здесь будет проверка токена
    # Для демонстрации просто возвращаем фиксированные данные
    # В реальном приложении нужно будет извлекать ID пользователя из токена
    raise HTTPException(status_code=501, detail="Not implemented yet - requires token authentication")