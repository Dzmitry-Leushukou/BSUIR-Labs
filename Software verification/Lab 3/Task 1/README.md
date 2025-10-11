
# Task 1 — Hello, world! (и случайные восклицания)

## Запуск
```bash
python -m hello_app.cli
```
Выводит три строки:
1) `Hello, world!`
2) `And hi again!`
3) Строку из случайного числа `!` (в диапазоне 5..50).

## Тесты
```bash
python -m unittest discover -s tests -p "test_*.py" -v
```
