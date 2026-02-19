import os
import base64
import csv
import logging
from typing import Dict
from io import StringIO

import uvicorn
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, validator
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from dotenv import load_dotenv

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

CSV_FILE = "keys.enc"

load_dotenv()

MASTER_KEY_B64 = os.getenv("KSS_MASTER_KEY")
if not MASTER_KEY_B64:
    raise RuntimeError("Переменная окружения KSS_MASTER_KEY не установлена")
try:
    MASTER_KEY = base64.b64decode(MASTER_KEY_B64)
    if len(MASTER_KEY) != 32:
        raise ValueError("Мастер-ключ должен быть 32 байта")
except Exception as e:
    raise RuntimeError(f"Ошибка загрузки мастер-ключа: {e}")

keys_db: Dict[str, bytes] = {}

app = FastAPI(title="Key Storage Service (encrypted)")

class KeyEntry(BaseModel):
    entity_id: str
    key: str

    @validator('key')
    def validate_key_length(cls, v):
        try:
            decoded = base64.b64decode(v)
        except Exception:
            raise ValueError('Некорректная base64 строка')
        if len(decoded) != 32:
            raise ValueError('Ключ должен быть 32 байта')
        return v

class RegisterRequest(BaseModel):
    entity_id: str

def encrypt_data(plaintext: bytes) -> bytes:
    aesgcm = AESGCM(MASTER_KEY)
    nonce = os.urandom(12)
    ciphertext = aesgcm.encrypt(nonce, plaintext, None)
    return nonce + ciphertext

def decrypt_data(encrypted: bytes) -> bytes:
    if len(encrypted) < 12:
        raise ValueError("Слишком короткие зашифрованные данные")
    nonce = encrypted[:12]
    ciphertext = encrypted[12:]
    aesgcm = AESGCM(MASTER_KEY)
    return aesgcm.decrypt(nonce, ciphertext, None)

def load_keys_from_encrypted():
    global keys_db
    if not os.path.exists(CSV_FILE):
        logger.info(f"Файл {CSV_FILE} не найден, начинаем с пустым хранилищем")
        keys_db = {}
        return
    try:
        with open(CSV_FILE, 'rb') as f:
            encrypted_data = f.read()
        plaintext = decrypt_data(encrypted_data)
        csv_text = plaintext.decode('utf-8')
        reader = csv.DictReader(StringIO(csv_text))
        loaded = {}
        for row in reader:
            entity_id = row['entity_id']
            key_b64 = row['key']
            try:
                key_bytes = base64.b64decode(key_b64)
                if len(key_bytes) != 32:
                    logger.warning(f"Ключ для {entity_id} имеет неверную длину, пропускаем")
                    continue
                loaded[entity_id] = key_bytes
            except Exception:
                logger.warning(f"Ошибка декодирования ключа для {entity_id}, пропускаем")
        keys_db = loaded
        logger.info(f"Загружено {len(keys_db)} ключей из {CSV_FILE}")
    except Exception as e:
        logger.error(f"Ошибка чтения {CSV_FILE}: {e}, начинаем с пустым хранилищем")
        keys_db = {}

def save_keys_to_encrypted():
    try:
        output = StringIO()
        writer = csv.writer(output)
        writer.writerow(['entity_id', 'key'])
        for entity_id, key_bytes in keys_db.items():
            key_b64 = base64.b64encode(key_bytes).decode('ascii')
            writer.writerow([entity_id, key_b64])
        csv_text = output.getvalue()
        plaintext = csv_text.encode('utf-8')
        encrypted_data = encrypt_data(plaintext)
        with open(CSV_FILE, 'wb') as f:
            f.write(encrypted_data)
        logger.info(f"Сохранено {len(keys_db)} ключей в {CSV_FILE}")
    except Exception as e:
        logger.error(f"Ошибка записи в {CSV_FILE}: {e}")

def ensure_default_keys():
    defaults = ["tgs"] 
    created = False
    for entity_id in defaults:
        if entity_id not in keys_db:
            new_key = os.urandom(32)
            keys_db[entity_id] = new_key
            logger.info(f"Auto-created key for {entity_id}")
            created = True
    if created:
        save_keys_to_encrypted()

load_keys_from_encrypted()
ensure_default_keys()

@app.get("/keys/{entity_id}")
async def get_key(entity_id: str):
    logger.info(f"GET /keys/{entity_id}")
    if entity_id not in keys_db:
        raise HTTPException(status_code=404, detail="Entity not found")
    key_bytes = keys_db[entity_id]
    key_b64 = base64.b64encode(key_bytes).decode('ascii')
    return {"key": key_b64}

@app.post("/keys", status_code=201)
async def create_key(entry: KeyEntry):
    logger.info(f"POST /keys entity_id={entry.entity_id}")
    key_bytes = base64.b64decode(entry.key)
    keys_db[entry.entity_id] = key_bytes
    save_keys_to_encrypted()
    return {"message": "Key stored successfully"}

@app.post("/register", status_code=201)
async def register(req: RegisterRequest):
    entity_id = req.entity_id
    if not entity_id:
        raise HTTPException(status_code=400, detail="entity_id required")
    
    if entity_id in keys_db:
        raise HTTPException(status_code=409, detail="Entity already exists")
    
    new_key = os.urandom(32)
    keys_db[entity_id] = new_key
    save_keys_to_encrypted()
    
    key_b64 = base64.b64encode(new_key).decode('ascii')
    return {"entity_id": entity_id, "key": key_b64}

@app.get("/")
async def root():
    return {"service": "Key Storage Service (encrypted)"}

if __name__ == "__main__":
    port = int(os.getenv("PORT", "8000"))
    logger.info(f"Starting KSS on port {port}")
    uvicorn.run(app, host="0.0.0.0", port=port)