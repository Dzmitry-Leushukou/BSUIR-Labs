
# Поиск файлов по расширению (Ubuntu)

**Задача:** найти в указанной папке (и всех подпапках) файлы с заданным расширением
и вывести их абсолютные пути в формате `dir1/dir2/file.ext` (POSIX).

## Запуск (Ubuntu)
> **Важно:** если в пути есть пробелы — обязательно используйте кавычки.

```bash
python -m file_finder.cli "/home/dzmitry_leushukou/Programming/BSUIR/BSUIR-Labs/Software verification/Lab 3/Task 5" "txt"
# или из самой папки:
cd "/home/dzmitry_leushukou/Programming/BSUIR/BSUIR-Labs/Software verification/Lab 3/Task 5"
python -m file_finder.cli "." "txt"
```

Опции:
- `--case-sensitive` — учитывать регистр расширения (по умолчанию нет, найдёт *.TXT тоже).
- `--follow-symlinks` — следовать симлинкам.

## Пример вывода (Ubuntu)
```
/home/dzmitry_leushukou/projects/a/file1.txt
/home/dzmitry_leushukou/projects/a/b/file2.TXT
```

## Поведение и надёжность
- Валидация: путь существует и это директория; расширение корректно.
- Рекурсивный обход `os.walk`, вывод **абсолютных** POSIX‑путей.
- Порядок — как обходит `os.walk` (сверху вниз).
- Если ничего не найдено, печатается сообщение в `stderr`: `Ничего не найдено.`

## Тесты
```bash
python -m unittest discover -s tests -p "test_*.py" -v
# или
PYTHONPATH=. pytest -vv --color=yes
```
