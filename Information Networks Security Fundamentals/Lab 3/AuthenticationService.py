import os
import base64
import json
import time
import logging
from typing import Dict
import uvicorn
import requests
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from dotenv import load_dotenv

load_dotenv()
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

KSS_URL = os.getenv("KSS_URL", "http://localhost:8000")
PORT = int(os.getenv("PORT", "8001"))

class AuthRequest(BaseModel):
    user_id: str

app = FastAPI(title="Authentication Service")

def encrypt_aes_gcm(key: bytes, plaintext: bytes) -> bytes:
    aesgcm = AESGCM(key)
    nonce = os.urandom(12)
    ciphertext = aesgcm.encrypt(nonce, plaintext, None)
    return nonce + ciphertext

def dict_to_bytes(d: Dict) -> bytes:
    return json.dumps(d).encode('utf-8')

def b64_encode(data: bytes) -> str:
    return base64.b64encode(data).decode('ascii')

def b64_decode(data: str) -> bytes:
    return base64.b64decode(data)

def fetch_key(entity_id: str) -> bytes:
    url = f"{KSS_URL}/keys/{entity_id}"
    logger.info(f"Fetching key for {entity_id} from {url}")
    try:
        resp = requests.get(url, timeout=5)
    except requests.exceptions.RequestException as e:
        logger.error(f"KSS connection error: {e}")
        raise HTTPException(status_code=500, detail="KSS unavailable")
    if resp.status_code == 404:
        raise HTTPException(status_code=404, detail=f"Entity '{entity_id}' not found")
    elif resp.status_code != 200:
        logger.error(f"KSS returned {resp.status_code}: {resp.text}")
        raise HTTPException(status_code=500, detail="KSS error")
    data = resp.json()
    key_b64 = data.get("key")
    if not key_b64:
        raise HTTPException(status_code=500, detail="Invalid KSS response")
    return b64_decode(key_b64)

@app.post("/auth")
async def auth(request: AuthRequest):
    user_id = request.user_id
    if not user_id:
        raise HTTPException(status_code=400, detail="user_id is required")
    logger.info(f"Auth request for user: {user_id}")
    try:
        user_key = fetch_key(user_id)
    except HTTPException as e:
        raise e
    try:
        tgs_key = fetch_key("tgs")
    except HTTPException as e:
        logger.error("TGS key not found in KSS")
        raise HTTPException(status_code=500, detail="TGS key missing")
    sk_tgs = os.urandom(32)
    issue_time = int(time.time())
    expiry_time = issue_time + 8 * 3600
    tgt_payload = {
        "user_id": user_id,
        "session_key": b64_encode(sk_tgs),
        "issue_time": issue_time,
        "expiry_time": expiry_time
    }
    tgt_bytes = dict_to_bytes(tgt_payload)
    tgt_encrypted = encrypt_aes_gcm(tgs_key, tgt_bytes)
    tgt_b64 = b64_encode(tgt_encrypted)
    client_payload = {"session_key": b64_encode(sk_tgs)}
    client_bytes = dict_to_bytes(client_payload)
    client_encrypted = encrypt_aes_gcm(user_key, client_bytes)
    client_block_b64 = b64_encode(client_encrypted)
    return {"encrypted_block": client_block_b64, "tgt": tgt_b64}

@app.get("/")
async def root():
    return {"service": "Authentication Service"}

if __name__ == "__main__":
    logger.info(f"Starting AS on port {PORT}, KSS URL: {KSS_URL}")
    uvicorn.run(app, host="0.0.0.0", port=PORT)