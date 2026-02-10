from fastapi import APIRouter, HTTPException
from schemas import *
from crud.sessions_crud import *
from typing import List

router = APIRouter(prefix="/sessions", tags=["Sessions"])

@router.get("/", response_model=List[Session])
def get_sessions_endpoint(offset: int = 0, limit: int = 10):
    return get_sessions(offset, limit)

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