from fastapi import APIRouter, HTTPException, Depends, Request
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

router = APIRouter(prefix="/users", tags=["Пользователи"])

def get_current_user_from_header(request: Request):
    """
    Получает текущего пользователя по user_id из заголовка.
    """
    user_id = request.headers.get('X-User-ID')
    
    if not user_id:
        raise HTTPException(status_code=401, detail="Not authenticated")
    
    try:
        user_id = int(user_id)
    except ValueError:
        raise HTTPException(status_code=401, detail="Invalid user ID format")
    
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
def get_user_profile(current_user: dict = Depends(get_current_user_from_header)):
    return current_user

@router.put("/profile")
def update_user_profile(request: Request, user_update: UserUpdate, current_user: dict = Depends(get_current_user_from_header)):
    # Обновляем пользователя с ID, извлеченным из заголовка
    updated_user = update_user(current_user['id'], user_update)
    
    # Удаляем hashed_password из ответа для безопасности
    user_data = dict(updated_user)
    if 'hashed_password' in user_data:
        del user_data['hashed_password']
    
    # Возвращаем обновленные данные без токена
    return user_data

@router.get("/{user_id}", response_model=User)
def get_user_endpoint(user_id: int):
    return get_user(user_id)

@router.post("/", response_model=User)
def create_user_endpoint(user: UserCreate):
    return create_user(user)

@router.put("/change-password")
def change_password_endpoint(request: Request, password_change: UserPasswordChange, current_user: dict = Depends(get_current_user_from_header)):
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
def login_user_endpoint(request: Request, user_login: UserLogin):
    user = get_user_by_email(user_login.email)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    
    # Проверка хешированного пароля
    if not verify_password(user_login.password, user['hashed_password']):
        raise HTTPException(status_code=401, detail="Incorrect password")
    
    # Логируем успешный вход пользователя
    from crud.action_logs_crud import create_action_log
    from schemas import ActionLogCreate
    import json
    
    user_agent = request.headers.get('User-Agent', 'Unknown')
    
    log_entry = ActionLogCreate(
        actor_user_id=user['id'],
        action_type='user_login',
        target_user_id=user['id'],
        description='User login successful',
        old_values=None,
        new_values=None,
        user_agent=user_agent
    )
    
    try:
        create_action_log(log_entry)
    except Exception as e:
        # Если логирование не удалось, не прерываем основной процесс
        print(f"Failed to log user login: {str(e)}")
    
    # Возвращаем информацию о пользователе без пароля
    user_data = dict(user)
    del user_data['hashed_password']
    
    # Возвращаем информацию о пользователе без токена
    return user_data

@router.post("/register")
async def register_user_endpoint(request: Request, user: UserRegistration):
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
    
    # Логируем успешную регистрацию пользователя
    from crud.action_logs_crud import create_action_log
    from schemas import ActionLogCreate
    import json
    
    user_agent = request.headers.get('User-Agent', 'Unknown')
    
    log_entry = ActionLogCreate(
        actor_user_id=created_user['id'],
        action_type='user_registration',
        target_user_id=created_user['id'],
        description='User registration successful',
        old_values=None,
        new_values=json.dumps({
            'email': created_user['email'],
            'name': created_user['name'],
            'surname': created_user['surname'],
            'role_id': created_user['role_id']
        }),
        user_agent=user_agent
    )
    
    try:
        create_action_log(log_entry)
    except Exception as e:
        # Если логирование не удалось, не прерываем основной процесс
        print(f"Failed to log user registration: {str(e)}")
    
    # Возвращаем информацию о пользователе без пароля
    user_response = dict(created_user)
    if 'hashed_password' in user_response:
        del user_response['hashed_password']
    
    # Возвращаем информацию о пользователе без токена
    print(f"Registration successful for user ID: {user_response['id']}")
    return user_response

@router.post("/logout")
def logout_user_endpoint(request: Request, current_user: dict = Depends(get_current_user_from_header)):
    # Логируем выход пользователя
    from crud.action_logs_crud import create_action_log
    from schemas import ActionLogCreate
    
    user_agent = request.headers.get('User-Agent', 'Unknown')
    
    log_entry = ActionLogCreate(
        actor_user_id=current_user['id'],
        action_type='user_logout',
        target_user_id=current_user['id'],
        description='User logout successful',
        old_values=None,
        new_values=None,
        user_agent=user_agent
    )
    
    try:
        create_action_log(log_entry)
    except Exception as e:
        # Если логирование не удалось, не прерываем основной процесс
        print(f"Failed to log user logout: {str(e)}")
    
    return {"message": "Logout successful"}