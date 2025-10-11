
#!/usr/bin/env python3
import argparse
import sys
from .logic import find_files, FinderError


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description="Рекурсивный поиск файлов по расширению (Ubuntu-friendly пути)."
    )
    parser.add_argument("directory", help="Стартовая папка (например, /home/user/projects)")
    parser.add_argument("extension", help="Расширение (например, txt или .txt)")
    parser.add_argument("--case-sensitive", action="store_true", help="Учитывать регистр расширения (по умолчанию нет)")
    parser.add_argument("--follow-symlinks", action="store_true", help="Следовать симлинкам (по умолчанию нет)")
    args = parser.parse_args(argv)

    try:
        files = find_files(
            args.directory,
            args.extension,
            case_sensitive=args.case_sensitive,
            follow_symlinks=args.follow_symlinks,
        )
    except FinderError as e:
        print(f"Ошибка: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Неизвестная ошибка: {e}", file=sys.stderr)
        return 1

    if not files:
        # Сообщим явно, если ничего не найдено — чтобы было понятно пользователю
        print("Ничего не найдено.", file=sys.stderr)
        return 0

    for p in files:
        # Выводим POSIX-путь (с /), как в примере; под Ubuntu это native
        print(p.as_posix())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
