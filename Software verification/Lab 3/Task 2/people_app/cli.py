#!/usr/bin/env python3
import sys
from typing import List
from .logic import parse_person_line, compute_stats, ParseError, Person


def read_people(stdin) -> List[Person]:
    people: List[Person] = []
    prompt = "Введите людей (Имя Фамилия Возраст) по одной строке. Пустая строка — конец ввода.\n"
    # Печатаем подсказку только в интерактивном режиме
    if stdin is sys.stdin and sys.stdin.isatty():
        print(prompt, end="")
    while True:
        try:
            line = stdin.readline()
        except KeyboardInterrupt:
            print("\nВвод прерван пользователем.", file=sys.stderr)
            sys.exit(1)
        if line == "":
            # EOF
            break
        if line.strip() == "":
            # Пустая строка завершает ввод
            break

        # ВАЖНО: не выходим здесь, а пробрасываем ошибку выше
        person = parse_person_line(line)
        people.append(person)

    return people


def main(argv=None) -> int:
    argv = argv or sys.argv[1:]
    try:
        people = read_people(sys.stdin)
    except ParseError as e:
        print(f"Ошибка: {e}", file=sys.stderr)
        return 1

    if not people:
        print("Ошибка: не введено ни одного человека.", file=sys.stderr)
        return 1

    for p in people:
        print(f"{p.last_name} {p.first_name} {p.age}")

    min_age, max_age, avg_age = compute_stats([p.age for p in people])
    # Формат ровно как ожидает тест: одна строка "min max avg"
    print(f"{min_age} {max_age} {avg_age:.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
