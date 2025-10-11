
#!/usr/bin/env python3
import argparse
import sys
from .logic import parse_nonnegative_number, compute_area, format_decimal, ParseNumberError


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="Вычисление площади прямоугольника: S = a*b.")
    parser.add_argument("length", help="Длина прямоугольника (неотрицательное число)")
    parser.add_argument("width", help="Ширина прямоугольника (неотрицательное число)")
    parser.add_argument("--precision", type=int, default=None, help="Количество знаков после запятой (0..30)")
    args = parser.parse_args(argv)

    try:
        a = parse_nonnegative_number(args.length)
        b = parse_nonnegative_number(args.width)
        area = compute_area(a, b)
        text = format_decimal(area, args.precision)
        print(text)
        return 0
    except ParseNumberError as e:
        print(f"Ошибка: {e}", file=sys.stderr)
        return 1
    except ValueError as e:
        print(f"Ошибка: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Неизвестная ошибка: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
