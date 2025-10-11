from dataclasses import dataclass
from typing import List, Tuple


@dataclass(frozen=True)
class Person:
    first_name: str
    last_name: str
    age: int


class ParseError(ValueError):
    """Raised when an input line cannot be parsed into a Person."""


def parse_person_line(line: str) -> Person:
    """
    Parse a single line in the format:
        Имя Фамилия Возраст
    Age must be a non-negative integer.
    Extra surrounding spaces are allowed.
    """
    if line is None:
        raise ParseError("Пустая строка.")
    parts = line.strip().split()
    if not parts:
        raise ParseError("Пустая строка.")
    if len(parts) < 3:
        raise ParseError("Ожидались три значения: Имя Фамилия Возраст.")
    if len(parts) > 3:
        # Support compound surnames or names via joining until last token which must be age?
        # To keep spec strict and predictable, enforce exactly 3 tokens.
        raise ParseError("Строка должна содержать ровно три значения: Имя Фамилия Возраст.")
    first, last, age_str = parts
    if not first.isalpha() or not last.isalpha():
        # Allow unicode letters (isalpha supports), but disallow digits and punctuation.
        raise ParseError("Имя и фамилия должны содержать только буквы.")
    try:
        age = int(age_str)
    except ValueError as e:
        raise ParseError("Возраст должен быть целым числом.") from e
    if age < 0 or age > 150:
        raise ParseError("Возраст должен быть в диапазоне 0..150.")
    return Person(first_name=first, last_name=last, age=age)


def compute_stats(ages: List[int]) -> Tuple[int, int, float]:
    """
    Return (min_age, max_age, avg_age_float).
    Raises ValueError for empty input.
    """
    if not ages:
        raise ValueError("Список возрастов пуст.")
    mn = min(ages)
    mx = max(ages)
    avg = sum(ages) / len(ages)
    return mn, mx, avg