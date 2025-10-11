
from typing import List


def grayscale_hex(value: int) -> str:
    """Return hex color like #RRGGBB for a grayscale 0..255 value."""
    if not (0 <= value <= 255):
        raise ValueError("grayscale value must be in 0..255")
    return f"#{value:02x}{value:02x}{value:02x}"


def generate_grayscale_values(rows: int) -> List[int]:
    """
    Generate a list of grayscale channel values from white to black.
    If rows == 256, step == 1 (минимально возможный шаг).
    If rows < 2, raise ValueError.
    """
    if rows < 2:
        raise ValueError("rows must be >= 2")
    result = []
    for i in range(rows):
        t = i / (rows - 1)
        v = round(255 * (1 - t))
        result.append(v)
    return result


def generate_html_table(rows: int = 256, cols: int = 3, cell_text: str = "{hex}") -> str:
    """
    Build an HTML document with a table of given rows and cols.
    Each row has uniform grayscale background from white->black.
    cell_text can contain {hex} and {value} placeholders.
    """
    values = generate_grayscale_values(rows)
    colors = [grayscale_hex(v) for v in values]
    parts = []
    parts.append("<!doctype html>")
    parts.append("<html lang='ru'>")
    parts.append("<head>")
    parts.append("<meta charset='utf-8'>")
    parts.append("<meta name='viewport' content='width=device-width, initial-scale=1'>")
    parts.append("<title>Градиентная таблица: белый → чёрный</title>")
    parts.append("""<style>
        body { font-family: Arial, sans-serif; margin: 16px; }
        table { border-collapse: collapse; width: 100%; }
        td { border: 1px solid #ccc; padding: 8px; text-align: center; }
        .row-label { width: 140px; text-align: left; }
    </style>""")
    parts.append("</head><body>")
    parts.append("<h1>Градиентная таблица (от белого к чёрному)</h1>")
    parts.append(f"<p>Строк: {rows}, столбцов: {cols}. Шаг по яркости минимальный при 256 строках.</p>")
    parts.append("<table>")
    for i, (v, hexcode) in enumerate(zip(values, colors), start=1):
        parts.append(f"<tr style='background:{hexcode}'>")
        text_color = '#000000' if v > 128 else '#ffffff'
        label = f"#{i}: {hexcode} (v={v})"
        parts.append(f"<td class='row-label' style='color:{text_color}; font-weight:bold'>{label}</td>")
        for _ in range(cols-1):
            txt = cell_text.format(hex=hexcode, value=v)
            parts.append(f"<td style='color:{text_color}'>{txt}</td>")
        parts.append("</tr>")
    parts.append("</table>")
    parts.append("</body></html>")
    return "\n".join(parts)
