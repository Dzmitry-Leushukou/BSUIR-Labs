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
PORT = int(os.getenv("PORT", "8003"))
SERVICE_ID = os.getenv("SERVICE_ID", "fileserver")  

class ResourceRequest(BaseModel):
    ticket: str
    authenticator: str
    request_data: str = ""  

app = FastAPI(title="Service Server")

def encrypt_aes_gcm(key: bytes, plaintext: bytes) -> bytes:
    aesgcm = AESGCM(key)
    nonce = os.urandom(12)
    ciphertext = aesgcm.encrypt(nonce, plaintext, None)
    return nonce + ciphertext

def decrypt_aes_gcm(key: bytes, encrypted_data: bytes) -> bytes:
    if len(encrypted_data) < 12:
        raise ValueError("Too short encrypted data")
    nonce = encrypted_data[:12]
    ciphertext = encrypted_data[12:]
    aesgcm = AESGCM(key)
    return aesgcm.decrypt(nonce, ciphertext, None)

def dict_to_bytes(d: Dict) -> bytes:
    return json.dumps(d).encode('utf-8')

def bytes_to_dict(b: bytes) -> Dict:
    return json.loads(b.decode('utf-8'))

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

@app.post("/resource")
async def access_resource(request: ResourceRequest):
    if not request.ticket or not request.authenticator:
        raise HTTPException(status_code=400, detail="Missing ticket or authenticator")

    logger.info(f"Resource request for service {SERVICE_ID}")

    try:
        service_key = fetch_key(SERVICE_ID)
    except HTTPException as e:
        logger.error(f"Failed to fetch own key for {SERVICE_ID}")
        raise HTTPException(status_code=500, detail="Service key missing")

    try:
        ticket_enc = b64_decode(request.ticket)
        ticket_bytes = decrypt_aes_gcm(service_key, ticket_enc)
        ticket = bytes_to_dict(ticket_bytes)
    except Exception as e:
        logger.warning(f"Ticket decryption failed: {e}")
        raise HTTPException(status_code=401, detail="Invalid service ticket")

    current_time = int(time.time())
    if ticket["expiry_time"] < current_time:
        logger.info("Service ticket expired")
        raise HTTPException(status_code=401, detail="Ticket expired")

    try:
        sk_service = b64_decode(ticket["session_key"])
    except Exception:
        raise HTTPException(status_code=401, detail="Invalid ticket format")

    try:
        auth_enc = b64_decode(request.authenticator)
        auth_bytes = decrypt_aes_gcm(sk_service, auth_enc)
        auth = bytes_to_dict(auth_bytes)
    except Exception as e:
        logger.warning(f"Authenticator decryption failed: {e}")
        raise HTTPException(status_code=401, detail="Invalid authenticator")

    if auth["user_id"] != ticket["user_id"]:
        logger.warning("User ID mismatch")
        raise HTTPException(status_code=401, detail="User ID mismatch")

    timestamp = auth.get("timestamp")
    if not timestamp or abs(current_time - timestamp) > 300:
        logger.warning("Timestamp too old or missing")
        raise HTTPException(status_code=401, detail="Timestamp invalid or expired")

    response_text = f"Hello, {ticket['user_id']}! You requested: {request.request_data}"
    
    return {"response": response_text}

@app.get("/")
async def root():
    return {"service": "Service Server", "service_id": SERVICE_ID}

if __name__ == "__main__":
    logger.info(f"Starting SS on port {PORT}, KSS URL: {KSS_URL}, SERVICE_ID: {SERVICE_ID}")
    uvicorn.run(app, host="0.0.0.0", port=PORT)