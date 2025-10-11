
import os
import time
import urllib.request
import urllib.error
import urllib.parse
from typing import Optional
from .utils import ensure_dir, is_writable_dir, safe_filename, dedupe_filename


USER_AGENT = "url-fetcher/1.0 (+https://example.invalid)"
DEFAULT_TIMEOUT = 20
DEFAULT_RETRIES = 3
CHUNK_SIZE = 64 * 1024


class DownloadError(RuntimeError):
    pass


def _filename_from_cd(cd_header: Optional[str]) -> Optional[str]:
    if not cd_header:
        return None
    parts = cd_header.split(";")
    for p in parts:
        p = p.strip()
        if p.lower().startswith("filename*="):
            try:
                _, v = p.split("=", 1)
                enc, _, rest = v.split("'", 2)
                return urllib.parse.unquote(rest)
            except Exception:
                continue
        if p.lower().startswith("filename="):
            v = p.split("=", 1)[1].strip().strip('"').strip("'")
            return v or None
    return None


def _infer_name_from_url(url: str) -> Optional[str]:
    path = urllib.parse.urlparse(url).path
    if not path or path.endswith("/"):
        return None
    name = os.path.basename(path)
    return name or None


def choose_filename(url: str, headers, explicit_name: Optional[str], out_dir: str) -> str:
    name = explicit_name or _filename_from_cd(headers.get("Content-Disposition")) or _infer_name_from_url(url) or "download"
    name = safe_filename(name)
    name = dedupe_filename(out_dir, name)
    return name


def _is_transient(e: Exception) -> bool:
    if isinstance(e, urllib.error.HTTPError):
        return 500 <= e.code < 600
    if isinstance(e, urllib.error.URLError):
        return True
    return False


def fetch(url: str, out_dir: str, *, filename: Optional[str] = None, timeout: int = DEFAULT_TIMEOUT, retries: int = DEFAULT_RETRIES, chunk_size: int = CHUNK_SIZE) -> str:
    parsed = urllib.parse.urlparse(url)
    if parsed.scheme not in {"http", "https"}:
        raise DownloadError("Поддерживаются только http/https URL.")
    if not is_writable_dir(out_dir):
        raise DownloadError(f"Папка недоступна для записи: {out_dir}")
    ensure_dir(out_dir)

    req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    attempt = 0
    last_exc = None

    while attempt <= retries:
        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                final_url = resp.geturl()
                headers = resp.headers
                name = choose_filename(final_url, headers, filename, out_dir)
                tmp_path = os.path.join(out_dir, f".{name}.part")
                final_path = os.path.join(out_dir, name)

                with open(tmp_path, "wb") as f:
                    while True:
                        chunk = resp.read(chunk_size)
                        if not chunk:
                            break
                        f.write(chunk)
                os.replace(tmp_path, final_path)
                return final_path
        except Exception as e:
            last_exc = e
            if _is_transient(e) and attempt < retries:
                time.sleep(0.4 * (2 ** attempt))
                attempt += 1
                continue
            if isinstance(e, urllib.error.HTTPError):
                raise DownloadError(f"HTTP {e.code}: {e.reason}") from e
            if isinstance(e, urllib.error.URLError):
                raise DownloadError(f"Ошибка сети: {e.reason}") from e
            raise DownloadError(str(e)) from e

    raise DownloadError(f"Не удалось загрузить {url}: {last_exc}")
