
#!/usr/bin/env python3
import argparse
import sys
from .downloader import fetch, DownloadError


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="Скачать документ по URL в указанную папку (надёжно).")
    parser.add_argument("url", help="HTTP/HTTPS URL")
    parser.add_argument("out_dir", help="Папка для сохранения файла")
    parser.add_argument("--name", help="Явное имя файла (без пути)")
    parser.add_argument("--timeout", type=int, default=20, help="Таймаут запроса (сек)")
    parser.add_argument("--retries", type=int, default=3, help="Количество повторов при временных ошибках")
    parser.add_argument("--chunk", type=int, default=65536, help="Размер блока чтения (байт)")
    args = parser.parse_args(argv)

    try:
        path = fetch(args.url, args.out_dir, filename=args.name, timeout=args.timeout, retries=args.retries, chunk_size=args.chunk)
    except DownloadError as e:
        print(f"Ошибка: {e}", file=sys.stderr)
        return 1

    print(f"Сохранено: {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
