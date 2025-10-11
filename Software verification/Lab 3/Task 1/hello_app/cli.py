
#!/usr/bin/env python3
import random
import sys
from typing import List, Optional


def build_lines(n_exclamations: Optional[int] = None) -> List[str]:
    if n_exclamations is None:
        n_exclamations = random.randint(5, 50)
    if not (5 <= n_exclamations <= 50):
        raise ValueError("Количество восклицательных знаков должно быть в диапазоне 5..50")
    return [
        "Hello, world!",
        "And hi again!",
        "!" * n_exclamations,
    ]


def main(argv=None) -> int:
    try:
        for line in build_lines():
            print(line)
    except Exception as e:
        print(f"Ошибка: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
