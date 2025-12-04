from fastapi import APIRouter, HTTPException, Depends
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from schemas import (
    User, UserCreate, UserUpdate, UserLogin, UserRegistration,
    UserPasswordChange
)
from crud.users_crud import get_user, get_users, create_user, update_user, delete_user, get_user_by_email
from typing import List
import bcrypt
from database import get_db_connection
from psycopg2.extras import RealDictCursor

security = HTTPBearer()

def verify_password(plain_password: str, hashed_password: str) -> bool:
    """Проверяет пароль, сравнивая его с хешем"""
    # Convert the hashed password string back to bytes for bcrypt
    try:
        # Check if the hashed_password is already bytes
        if isinstance(hashed_password, bytes):
            return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password)
        else:
            return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password.encode('utf-8'))
    except ValueError:
        # Handle case where hashed_password is already bytes or invalid
        return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password if isinstance(hashed_password, bytes) else hashed_password.encode('utf-8'))

def hash_password(password: str) -> str:
    """Хеширует пароль с использованием bcrypt"""
    return bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')

router = APIRouter(prefix="/users", tags=["Users"])

def get_current_user(credentials: HTTPAuthorizationCredentials = Depends(security)):
    """
    Получает текущего пользователя по токену.
    Для упрощения в демо-версии используем фиксированный токен.
    """
    token = credentials.credentials
    
    # В реальном приложении тут будет проверка токена и извлечение ID пользователя
    # Например, через JWT-токен или сессию
    
    # Для демонстрации проверим наличие токена
    if not token:
        raise HTTPException(status_code=401, detail="Not authenticated")
    
    # Временная логика - извлекаем ID пользователя из токена
    # В реальном приложении токен должен содержать ID пользователя
    # Для демонстрации будем использовать фиксированный ID из токена
    # В реальности токен может быть в формате "user_id:token", где user_id - это ID пользователя
    user_id = 1  # В реальном приложении токен должен содержать ID пользователя
    
    # Попробуем извлечь ID из токена, если токен имеет формат "user_id:token"
    if ':' in token:
        try:
            user_id_str, _ = token.split(':', 1)
            user_id = int(user_id_str)
        except ValueError:
            # Если не удалось извлечь ID, используем значение по умолчанию
            user_id = 1
    
    # Проверим, есть ли пользователь с таким ID
    user = get_user(user_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    
    # Удаляем hashed_password из ответа для безопасности
    user_data = dict(user)
    if 'hashed_password' in user_data:
        del user_data['hashed_password']
    return user_data

@router.get("/", response_model=List[User])
def get_users_endpoint(offset: int = 0, limit: int = 100):
    return get_users(offset, limit)

@router.get("/profile")
def get_user_profile(current_user: dict = Depends(get_current_user)):
    return current_user

@router.put("/profile")
def update_user_profile(user_update: UserUpdate, current_user: dict = Depends(get_current_user)):
    # Обновляем пользователя с ID, извлеченным из токена
    updated_user = update_user(current_user['id'], user_update)
    
    # Удаляем hashed_password из ответа для безопасности
    user_data = dict(updated_user)
    if 'hashed_password' in user_data:
        del user_data['hashed_password']
    
    # Возвращаем обновленные данные с токеном для согласованности
    user_data['token'] = f"{user_data['id']}:dummy_token"
    
    return user_data

@router.get("/{user_id}", response_model=User)
def get_user_endpoint(user_id: int):
    return get_user(user_id)

@router.post("/", response_model=User)
def create_user_endpoint(user: UserCreate):
    return create_user(user)

@router.put("/change-password")
def change_password_endpoint(password_change: UserPasswordChange, current_user: dict = Depends(get_current_user)):
    # Получаем полную информацию о пользователе по ID, чтобы получить хешированный пароль
    user = get_user(current_user['id'])
    if not verify_password(password_change.current_password, user['hashed_password']):
        raise HTTPException(status_code=400, detail="Current password is incorrect")
    
    # Проверяем, совпадает ли новый пароль с текущим
    if verify_password(password_change.new_password, user['hashed_password']):
        raise HTTPException(status_code=400, detail="New password must be different from current password")
    
    # Хешируем новый пароль
    hashed_new_password = hash_password(password_change.new_password)
    
    # Обновляем пароль в базе данных
    conn = get_db_connection()
    cur = conn.cursor(cursor_factory=RealDictCursor)
    cur.execute(
        "UPDATE users SET hashed_password = %s WHERE id = %s RETURNING *",
        (hashed_new_password, current_user['id'])
    )
    updated_user = cur.fetchone()
    conn.commit()
    cur.close()
    conn.close()
    
    if not updated_user:
        raise HTTPException(status_code=404, detail="User not found")
    
    # Удаляем hashed_password из ответа для безопасности
    user_data = dict(updated_user)
    if 'hashed_password' in user_data:
        del user_data['hashed_password']
    
    return {"message": "Password changed successfully"}

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
    
    # Возвращаем информацию о пользователе без пароля, но с токеном
    user_data = dict(user)
    del user_data['hashed_password']
    
    # Добавляем временный токен для демонстрации
    # В реальном приложении тут должен быть JWT-токен
    # Для демонстрации создадим токен в формате "user_id:token"
    user_data['token'] = f"{user_data['id']}:dummy_token"  # В реальном приложении токен должен быть сгенерирован
    
    return user_data

@router.post("/register")
async def register_user_endpoint(user: UserRegistration):
    print(f"Registration attempt with email: {user.email}")
    
    # Проверяем, существует ли уже пользователь с таким email
    existing_user = get_user_by_email(user.email)
    if existing_user:
        print(f"User with email {user.email} already exists")
        raise HTTPException(status_code=400, detail="User with this email already exists")
    
    print(f"User with email {user.email} does not exist, proceeding with registration")
    
    # Хешируем пароль
    hashed_password = hash_password(user.password)
    
    # Создаем пользователя с хешированным паролем
    user_data = user.model_dump()
    user_data['hashed_password'] = hashed_password
    # Удаляем plain text пароль из данных пользователя
    del user_data['password']
    
    print(f"Attempting to create user with data: {user_data}")
    
    try:
        # Создаем пользователя в базе данных
        created_user = create_user(UserCreate(**user_data))
        print(f"User created successfully with ID: {created_user['id']}")
    except Exception as e:
        print(f"Error creating user: {str(e)}")
        # Проверяем, возможно ли это связано с уникальным ограничением
        # Дополнительная проверка на случай гонки
        existing_user = get_user_by_email(user.email)
        if existing_user:
            print(f"User with email {user.email} exists after attempt")
            raise HTTPException(status_code=400, detail="User with this email already exists")
        else:
            # Если ошибка другая, пробрасываем её
            print(f"Other error occurred: {str(e)}")
            raise HTTPException(status_code=500, detail=f"Error creating user: {str(e)}")
    
    # Возвращаем информацию о пользователе без пароля, но с токеном
    user_response = dict(created_user)
    if 'hashed_password' in user_response:
        del user_response['hashed_password']
    
    # Добавляем временный токен для демонстрации
    # В реальном приложении токен должен быть JWT-токеном
    # Для демонстрации создадим токен в формате "user_id:token"
    user_response['token'] = f"{user_response['id']}:dummy_token"  # В реальном приложении токен должен быть сгенерирован
    
    print(f"Registration successful for user ID: {user_response['id']}")
    return user_response