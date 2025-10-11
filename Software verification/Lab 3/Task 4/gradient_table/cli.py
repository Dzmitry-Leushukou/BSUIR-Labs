
#!/usr/bin/env python3
import argparse
import sys
from .logic import generate_html_table


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description="Сгенерировать HTML-файл с таблицей, у которой фон строк меняется от белого к чёрному."
    )
    parser.add_argument("output", nargs="?", default="gradient_table.html", help="Путь к выходному HTML-файлу")
    parser.add_argument("--rows", type=int, default=256, help="Количество строк (256 = минимальный шаг)")
    parser.add_argument("--cols", type=int, default=3, help="Количество столбцов")
    parser.add_argument("--text", default="{hex}", help="Текст в ячейках, можно использовать {hex} и {value}")
    args = parser.parse_args(argv)

    try:
        html = generate_html_table(rows=args.rows, cols=args.cols, cell_text=args.text)
    except Exception as e:
        print(f"Ошибка: {e}", file=sys.stderr)
        return 1

    with open(args.output, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"Готово: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
