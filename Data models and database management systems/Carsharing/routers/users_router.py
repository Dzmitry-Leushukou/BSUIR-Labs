from fastapi import APIRouter, HTTPException, Depends, Request, status
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from schemas import (
    User, UserWithRoleName, UserCreate, UserUpdate, UserLogin, UserRegistration,
    UserPasswordChange, Token, TokenData
)
from crud.users_crud import get_user, get_users, create_user, update_user, delete_user, get_user_by_email
from typing import List, Optional
import bcrypt
from database import get_db_connection
from psycopg2.extras import RealDictCursor
from jwt_utils import create_access_token, decode_access_token
from redis_client import redis_client

security = HTTPBearer(auto_error=False)

def verify_password(plain_password: str, hashed_password: str) -> bool:
    """Проверяет пароль, сравнивая его с хешем"""
    try:
        if isinstance(hashed_password, bytes):
            return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password)
        else:
            return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password.encode('utf-8'))
    except ValueError:
        return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password if isinstance(hashed_password, bytes) else hashed_password.encode('utf-8'))

def hash_password(password: str) -> str:
    """Хеширует пароль с использованием bcrypt"""
    return bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')

router = APIRouter(prefix="/users", tags=["Пользователи"])


async def get_current_user(request: Request, credentials: HTTPAuthorizationCredentials = Depends(security)) -> dict:
    """
    Dependency to get current user from JWT token.
    Checks blacklist before allowing access.
    """
    # Check if Redis is available
    if not redis_client.is_connected():
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Сервис аутентификации временно недоступен"
        )

    # Get token from Authorization header
    if credentials is None:
        # Try to get token from Authorization header manually
        auth_header = request.headers.get("Authorization")
        if auth_header and auth_header.startswith("Bearer "):
            token = auth_header.split(" ")[1]
        else:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Токен аутентификации не предоставлен",
                headers={"WWW-Authenticate": "Bearer"},
            )
    else:
        token = credentials.credentials

    # Decode and validate token
    payload = decode_access_token(token)
    
    if payload is None:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Недействительный или истекший токен",
            headers={"WWW-Authenticate": "Bearer"},
        )

    user_id: Optional[int] = payload.get("sub")
    email: Optional[str] = payload.get("email")
    
    # Convert user_id to int (it's stored as string in JWT)
    if user_id is not None and isinstance(user_id, str):
        try:
            user_id = int(user_id)
        except ValueError:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Недействительный токен: некорректный user_id",
                headers={"WWW-Authenticate": "Bearer"},
            )

    if user_id is None:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Недействительный токен: отсутствует user_id",
            headers={"WWW-Authenticate": "Bearer"},
        )

    # Check if user is blacklisted
    if redis_client.check_blacklist(email):
        ttl = redis_client.get_blacklist_ttl(email)
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail=f"Пользователь заблокирован. Попробуйте через {ttl // 60} мин."
        )

    # Verify user exists
    user = get_user(user_id)
    if not user:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Пользователь не найден"
        )

    # Remove sensitive data
    user_data = dict(user)
    if 'hashed_password' in user_data:
        del user_data['hashed_password']

    return user_data


def get_current_user_from_header(request: Request):
    """
    Legacy function - gets current user from X-User-ID header.
    Kept for backward compatibility during transition.
    """
    user_id = request.headers.get('X-User-ID')

    if not user_id:
        raise HTTPException(status_code=401, detail="Пользователь не аутентифицирован")

    try:
        user_id = int(user_id)
    except ValueError:
        raise HTTPException(status_code=401, detail="Неверный формат ID пользователя")

    user = get_user(user_id)
    if not user:
        raise HTTPException(status_code=404, detail="Пользователь не найден")

    user_data = dict(user)
    if 'hashed_password' in user_data:
        del user_data['hashed_password']
    return user_data


@router.get("/", response_model=List[UserWithRoleName])
def get_users_endpoint(offset: int = 0, limit: int = 100):
    return get_users(offset, limit)


@router.get("/profile")
def get_user_profile(current_user: dict = Depends(get_current_user)):
    return current_user


@router.put("/profile")
def update_user_profile(request: Request, user_update: UserUpdate, current_user: dict = Depends(get_current_user)):
    updated_user = update_user(current_user['id'], user_update)

    user_data = dict(updated_user)
    if 'hashed_password' in user_data:
        del user_data['hashed_password']

    return user_data


@router.get("/count", response_model=dict)
def get_users_count_endpoint(current_user: dict = Depends(get_current_user)):
    from crud.users_crud import get_users_count
    count = get_users_count()
    return {"count": count}


@router.get("/{user_id}", response_model=User)
def get_user_endpoint(user_id: int):
    if user_id <= 0:
        raise HTTPException(status_code=400, detail="ID пользователя должен быть положительным целым числом")
    return get_user(user_id)


@router.post("/", response_model=User)
def create_user_endpoint(user: UserCreate):
    return create_user(user)


@router.put("/change-password")
def change_password_endpoint(
    request: Request, 
    password_change: UserPasswordChange, 
    current_user: dict = Depends(get_current_user)
):
    user = get_user(current_user['id'])
    if not verify_password(password_change.current_password, user['hashed_password']):
        raise HTTPException(status_code=400, detail="Текущий пароль неверен")

    if verify_password(password_change.new_password, user['hashed_password']):
        raise HTTPException(status_code=400, detail="Новый пароль должен отличаться от текущего")

    hashed_new_password = hash_password(password_change.new_password)

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
        raise HTTPException(status_code=404, detail="Пользователь не найден")

    user_data = dict(updated_user)
    if 'hashed_password' in user_data:
        del user_data['hashed_password']

    return {"message": "Пароль успешно изменен"}


@router.put("/{user_id}", response_model=User)
def update_user_endpoint(user_id: int, user: UserUpdate):
    if user_id <= 0:
        raise HTTPException(status_code=400, detail="ID пользователя должен быть положительным целым числом")
    return update_user(user_id, user)


@router.delete("/{user_id}")
def delete_user_endpoint(user_id: int):
    if user_id <= 0:
        raise HTTPException(status_code=400, detail="User ID must be a positive integer")
    return delete_user(user_id)


@router.post("/login")
def login_user_endpoint(request: Request, user_login: UserLogin):
    """
    Authenticate user and return JWT token.
    Implements Redis-based blacklist for failed login attempts.
    """
    email = user_login.email
    
    # Check if Redis is available
    if not redis_client.is_connected():
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Сервис аутентификации временно недоступен"
        )
    
    # Check if user is blacklisted
    if redis_client.check_blacklist(email):
        ttl = redis_client.get_blacklist_ttl(email)
        minutes = ttl // 60
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail=f"Пользователь заблокирован из-за многократных неудачных попыток входа. Попробуйте через {minutes} мин."
        )
    
    # Get user from database
    user = get_user_by_email(email)
    if not user:
        # Still increment failed login to prevent email enumeration
        redis_client.increment_failed_login(email)
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Неверный email или пароль"
        )

    # Verify password
    if not verify_password(user_login.password, user['hashed_password']):
        # Increment failed login counter
        failed_count = redis_client.increment_failed_login(email)
        
        if failed_count >= 3:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail=f"Пользователь заблокирован из-за многократных неудачных попыток входа. Попробуйте через {BLACKLIST_TTL_MINUTES} мин."
            )
        
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail=f"Неверный пароль. Осталось попыток: {3 - failed_count}"
        )

    # Successful login - reset failed counter and add to blacklist check
    redis_client.reset_failed_login(email)
    
    # Check if user is banned
    if user.get('status') == 'banned':
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Пользователь заблокирован администратором"
        )

    # Log successful login
    from crud.action_logs_crud import create_action_log
    from schemas import ActionLogCreate

    user_agent = request.headers.get('User-Agent', 'Unknown')

    log_entry = ActionLogCreate(
        actor_user_id=user['id'],
        action_type='user_login',
        target_user_id=user['id'],
        description='Успешный вход пользователя',
        old_values=None,
        new_values=None,
        user_agent=user_agent
    )

    try:
        create_action_log(log_entry)
    except Exception as e:
        print(f"Failed to log user login: {str(e)}")

    # Create JWT token
    access_token = create_access_token(
        data={
            "sub": user['id'],
            "email": user['email']
        }
    )

    # Prepare user data without sensitive information
    # Convert RealDictCursor to regular dict explicitly
    user_data = {
        "id": user['id'],
        "email": user['email'],
        "name": user['name'],
        "surname": user['surname'],
        "cashback": float(user['cashback']) if user['cashback'] else 0,
        "role_id": user['role_id'],
        "status": user['status'],
        "created_at": user['created_at'].isoformat() if user['created_at'] else None,
        "updated_at": user['updated_at'].isoformat() if user['updated_at'] else None
    }

    # Return token and user data
    return {
        "access_token": access_token,
        "token_type": "bearer",
        **user_data
    }


@router.post("/register")
async def register_user_endpoint(request: Request, user: UserRegistration):
    print(f"Registration attempt with email: {user.email}")

    existing_user = get_user_by_email(user.email)
    if existing_user:
        print(f"User with email {user.email} already exists")
        raise HTTPException(status_code=400, detail="Пользователь с этим email уже существует")

    print(f"User with email {user.email} does not exist, proceeding with registration")

    hashed_password = hash_password(user.password)

    user_data = user.model_dump()
    user_data['hashed_password'] = hashed_password
    del user_data['password']

    print(f"Attempting to create user with data: {user_data}")

    try:
        created_user = create_user(UserCreate(**user_data))
        print(f"User created successfully with ID: {created_user['id']}")
    except Exception as e:
        print(f"Error creating user: {str(e)}")
        existing_user = get_user_by_email(user.email)
        if existing_user:
            print(f"User with email {user.email} exists after attempt")
            raise HTTPException(status_code=400, detail="User with this email already exists")
        else:
            print(f"Other error occurred: {str(e)}")
            raise HTTPException(status_code=500, detail=f"Ошибка при создании пользователя: {str(e)}")

    from crud.action_logs_crud import create_action_log
    from schemas import ActionLogCreate

    user_agent = request.headers.get('User-Agent', 'Unknown')

    log_entry = ActionLogCreate(
        actor_user_id=created_user['id'],
        action_type='user_registration',
        target_user_id=created_user['id'],
        description='Успешная регистрация пользователя',
        old_values=None,
        new_values={
            'email': created_user['email'],
            'name': created_user['name'],
            'surname': created_user['surname'],
            'role_id': created_user['role_id']
        },
        user_agent=user_agent
    )

    try:
        create_action_log(log_entry)
    except Exception as e:
        print(f"Failed to log user registration: {str(e)}")

    user_response = dict(created_user)
    if 'hashed_password' in user_response:
        del user_response['hashed_password']

    print(f"Registration successful for user ID: {user_response['id']}")
    return user_response


@router.post("/logout")
def logout_user_endpoint(request: Request, current_user: dict = Depends(get_current_user)):
    from crud.action_logs_crud import create_action_log
    from schemas import ActionLogCreate

    user_agent = request.headers.get('User-Agent', 'Unknown')

    log_entry = ActionLogCreate(
        actor_user_id=current_user['id'],
        action_type='user_logout',
        target_user_id=current_user['id'],
        description='Успешный выход пользователя',
        old_values=None,
        new_values=None,
        user_agent=user_agent
    )

    try:
        create_action_log(log_entry)
    except Exception as e:
        print(f"Failed to log user logout: {str(e)}")

    return {"message": "Выход из системы выполнен успешно"}


# Import for error message
from redis_client import BLACKLIST_TTL_MINUTES
