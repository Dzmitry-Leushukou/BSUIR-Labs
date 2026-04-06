from fastapi import APIRouter, HTTPException, Request
from schemas import *
from crud.sessions_crud import *
from session_manager import session_manager
from typing import List

router = APIRouter(prefix="/sessions", tags=["Sessions"])


@router.get("/", response_model=List[Session])
def get_sessions_endpoint(offset: int = 0, limit: int = 10):
    return get_sessions(offset, limit)


# ==========================================
# Redis Session Management endpoints (должны быть выше /{session_id})
# ==========================================

@router.get("/stats")
def get_session_stats():
    """
    Получить статистику активных сессий.
    """
    try:
        return session_manager.get_active_sessions_count()
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to get session stats: {str(e)}")


@router.post("/validate")
def validate_session(session_data: dict):
    """
    Проверить валидность сессии по токену.
    """
    session_id = session_data.get("session_id")
    if not session_id:
        raise HTTPException(status_code=400, detail="session_id is required")
    
    session = session_manager.validate_session(session_id)
    if session is None:
        raise HTTPException(status_code=401, detail="Invalid or expired session")
    
    return {"valid": True, "session": session}


@router.get("/user/{user_id}/active")
def get_user_active_sessions(user_id: int):
    """
    Получить все активные сессии пользователя из Redis.
    """
    try:
        sessions = session_manager.get_user_sessions(user_id)
        return {"user_id": user_id, "sessions": sessions, "count": len(sessions)}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to get user sessions: {str(e)}")


@router.delete("/user/{user_id}/all")
def delete_all_user_sessions(user_id: int):
    """
    Удалить все сессии пользователя из Redis.
    """
    try:
        count = session_manager.delete_all_user_sessions(user_id)
        return {"user_id": user_id, "deleted_sessions": count}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to delete user sessions: {str(e)}")


@router.get("/{session_id}", response_model=Session)
def get_session_endpoint(session_id: int):
    return get_session(session_id)


@router.post("/", response_model=Session)
def create_session_endpoint(session: SessionCreate):
    return create_session(session)


@router.put("/{session_id}", response_model=Session)
def update_session_endpoint(session_id: int, session: SessionBase):
    return update_session(session_id, session)


@router.delete("/{session_id}")
def delete_session_endpoint(session_id: int):
    return delete_session(session_id)