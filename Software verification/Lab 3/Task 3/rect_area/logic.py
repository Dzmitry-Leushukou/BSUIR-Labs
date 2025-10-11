
from decimal import Decimal, getcontext, InvalidOperation
import re

getcontext().prec = 50  # High precision for computations

_NUM_RE = re.compile(r"^[+]?((\d+(\.\d*)?)|(\.\d+))([eE][+-]?\d+)?$")


class ParseNumberError(ValueError):
    pass


def parse_nonnegative_number(text: str) -> Decimal:
    """
    Parse non-negative number from string.
    Accepts dot or comma as decimal separator and scientific notation.
    Raises ParseNumberError for invalid or negative values.
    """
    if text is None:
        raise ParseNumberError("Пустое значение.")
    s = text.strip()
    if not s:
        raise ParseNumberError("Пустое значение.")
    s = s.replace(",", ".")
    # Normalize leading '+'
    if s.startswith("+"):
        s = s[1:]
    if not _NUM_RE.match(s):
        raise ParseNumberError(f"Некорректное число: '{text}'")
    try:
        val = Decimal(s)
    except InvalidOperation as e:
        raise ParseNumberError(f"Некорректное число: '{text}'") from e
    if val.is_nan() or val.is_infinite():
        raise ParseNumberError("Число не должно быть бесконечным или NaN.")
    if val < 0:
        raise ParseNumberError("Длина/ширина не может быть отрицательной.")
    return +val  # unary plus -> apply context precision


def compute_area(a: Decimal, b: Decimal) -> Decimal:
    """
    Compute rectangle area S = a*b.
    """
    return +(a * b)


def format_decimal(val: Decimal, places: int | None = None) -> str:
    """
    Return a human-friendly decimal string without scientific notation.
    If 'places' is provided, the number will be rounded to that many fraction digits.
    """
    if places is not None:
        if places < 0 or places > 30:
            raise ValueError("Точность должна быть от 0 до 30.")
        q = Decimal(1).scaleb(-places)  # 10^-places
        val = val.quantize(q)
    s = format(val, 'f')
    # trim trailing zeros and possible trailing dot
    if '.' in s:
        s = s.rstrip('0').rstrip('.')
    return s or "0"
