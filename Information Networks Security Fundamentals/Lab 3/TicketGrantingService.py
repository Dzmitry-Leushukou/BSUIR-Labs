
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
PORT = int(os.getenv("PORT", "8002"))

class TicketRequest(BaseModel):
    tgt: str
    service_id: str
    authenticator: str

app = FastAPI(title="Ticket Granting Service")

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

@app.post("/ticket")
async def get_ticket(request: TicketRequest):
    if not request.tgt or not request.service_id or not request.authenticator:
        raise HTTPException(status_code=400, detail="Missing required fields")

    logger.info(f"Ticket request for service: {request.service_id}")

    try:
        tgs_key = fetch_key("tgs")
    except HTTPException as e:
        logger.error("Failed to fetch TGS key")
        raise HTTPException(status_code=500, detail="TGS key missing")

    try:
        service_key = fetch_key(request.service_id)
    except HTTPException as e:
        raise e

    try:
        tgt_enc = b64_decode(request.tgt)
        tgt_bytes = decrypt_aes_gcm(tgs_key, tgt_enc)
        tgt = bytes_to_dict(tgt_bytes)
    except Exception as e:
        logger.warning(f"TGT decryption failed: {e}")
        raise HTTPException(status_code=401, detail="Invalid TGT")

    current_time = int(time.time())
    if tgt["expiry_time"] < current_time:
        logger.info("TGT expired")
        raise HTTPException(status_code=401, detail="TGT expired")

    try:
        sk_tgs = b64_decode(tgt["session_key"])
    except Exception:
        raise HTTPException(status_code=401, detail="Invalid TGT format")

    try:
        auth_enc = b64_decode(request.authenticator)
        auth_bytes = decrypt_aes_gcm(sk_tgs, auth_enc)
        auth = bytes_to_dict(auth_bytes)
    except Exception as e:
        logger.warning(f"Authenticator decryption failed: {e}")
        raise HTTPException(status_code=401, detail="Invalid authenticator")

    if auth["user_id"] != tgt["user_id"]:
        logger.warning("User ID mismatch")
        raise HTTPException(status_code=401, detail="User ID mismatch")

    timestamp = auth.get("timestamp")
    if not timestamp or abs(current_time - timestamp) > 300:
        logger.warning("Timestamp too old or missing")
        raise HTTPException(status_code=401, detail="Timestamp invalid or expired")

    sk_service = os.urandom(32)

    issue_time = current_time
    expiry_time = current_time + 3600  
    ticket_payload = {
        "user_id": tgt["user_id"],
        "session_key": b64_encode(sk_service),
        "issue_time": issue_time,
        "expiry_time": expiry_time
    }
    ticket_bytes = dict_to_bytes(ticket_payload)
    ticket_encrypted = encrypt_aes_gcm(service_key, ticket_bytes)
    service_ticket_b64 = b64_encode(ticket_encrypted)

    client_payload = {
        "session_key": b64_encode(sk_service),
        "service_ticket": service_ticket_b64
    }
    client_bytes = dict_to_bytes(client_payload)
    client_encrypted = encrypt_aes_gcm(sk_tgs, client_bytes)
    client_block_b64 = b64_encode(client_encrypted)

    return {
        "encrypted_block": client_block_b64,
        "service_ticket": service_ticket_b64
    }

@app.get("/")
async def root():
    return {"service": "Ticket Granting Service"}

if __name__ == "__main__":
    logger.info(f"Starting TGS on port {PORT}, KSS URL: {KSS_URL}")
    uvicorn.run(app, host="0.0.0.0", port=PORT)