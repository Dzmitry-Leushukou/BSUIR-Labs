
import os
import re
import sys
from pathlib import Path
from typing import List


class FinderError(ValueError):
    pass


_EXT_INVALID = re.compile(r"[\\/\s]")

def normalize_extension(ext: str, *, case_sensitive: bool) -> str:
    """
    Return normalized extension with leading dot.
    Accepts 'txt' or '.txt'. Raises on empty/invalid.
    """
    if ext is None:
        raise FinderError("Расширение не задано.")
    e = ext.strip()
    if not e:
        raise FinderError("Расширение пустое.")
    if _EXT_INVALID.search(e):
        raise FinderError("Расширение не должно содержать пробелов или / \\.")
    if e.startswith('.'):
        e = e[1:]
    if not e:
        raise FinderError("Расширение пустое.")
    e = '.' + (e if case_sensitive else e.lower())
    return e


def find_files(root: str | os.PathLike, ext: str, *, case_sensitive: bool=False, follow_symlinks: bool=False) -> List[Path]:
    """
    Recursively walk starting at 'root' and return a list of absolute Paths
    whose filename ends with the given extension (case-insensitive by default).
    Walk order is preserved (os.walk).
    """
    root_path = Path(root)
    if not root_path.exists():
        raise FinderError(f"Папка не существует: {root_path}")
    if not root_path.is_dir():
        raise FinderError(f"Ожидалась папка, а не файл: {root_path}")

    norm_ext = normalize_extension(ext, case_sensitive=case_sensitive)
    results: List[Path] = []

    def onerror(err):
        print(f"Предупреждение: {err}", file=sys.stderr)

    for dirpath, dirnames, filenames in os.walk(root_path, followlinks=follow_symlinks, onerror=onerror):
        for name in filenames:
            candidate = name if case_sensitive else name.lower()
            if candidate.endswith(norm_ext):
                p = Path(dirpath) / name
                try:
                    abs_path = p.resolve()
                except Exception:
                    abs_path = p.absolute()
                results.append(abs_path)

    return results
