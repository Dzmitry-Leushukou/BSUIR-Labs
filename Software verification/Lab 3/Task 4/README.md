
# Task 4 — HTML таблица с градиентом (белый → чёрный)

## Запуск
```bash
python3 -m gradient_table.cli gradient_table.html --rows 256 --cols 3 --text "{hex}"
```
- `--rows 256` даёт минимально возможный шаг (1 уровень яркости на строку).
- `--text` может использовать `{hex}` и `{value}`.

## Тесты
```bash
python3 -m unittest discover -s tests -p "test_*.py" -v
```
