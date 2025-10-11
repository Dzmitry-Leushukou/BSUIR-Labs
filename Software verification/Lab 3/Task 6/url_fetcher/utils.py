
import os
import re
import tempfile

_ILLEGAL = re.compile(r'[\\/:*?"<>|]+')

def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)

def is_writable_dir(path: str) -> bool:
    try:
        ensure_dir(path)
        testfile = tempfile.TemporaryFile(dir=path)
        testfile.close()
        return True
    except Exception:
        return False

def safe_filename(name: str, fallback: str = "download") -> str:
    name = (name or "").strip() or fallback
    name = _ILLEGAL.sub("_", name)
    if name.startswith("."):
        name = "_" + name.lstrip(".")
    return name

def dedupe_filename(dirpath: str, filename: str) -> str:
    base, ext = os.path.splitext(filename)
    i = 1
    candidate = filename
    while os.path.exists(os.path.join(dirpath, candidate)):
        candidate = f"{base} ({i}){ext}"
        i += 1
    return candidate
