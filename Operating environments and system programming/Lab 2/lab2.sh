#!/bin/bash

error() {
    echo "Ошибка: $1" >&2
    exit 1
}

warning() {
    echo "Предупреждение: $1" >&2
}

if [ $# -eq 0 ]; then
    echo "Ошибка: Не указан входной файл" >&2
    echo "Использование: $0 файл" >&2
    exit 1
fi

[ $# -gt 1 ] && warning "Лишние аргументы игнорируются. Используется только первый файл: $1"

[ ! -f "$1" ] && error "Файл '$1' не найден"
[ ! -r "$1" ] && error "Нет прав на чтение файла '$1'"

[ ! -s "$1" ] && warning "Файл '$1' пуст" && cat "$1" && exit 0

if ! command -v sed >/dev/null 2>&1; then
    error "sed не найден в системе"
fi

TEMP_FILE="/tmp/lab2_$$.txt"
touch "$TEMP_FILE" 2>/dev/null || TEMP_FILE="./lab2_$$.tmp"
touch "$TEMP_FILE" 2>/dev/null || error "Не удалось создать временный файл"

cat "$1" | tr '\n' '\r' > "$TEMP_FILE"

sed -i.tmp -E '
    # Защита десятичных точек
    s/([0-9])\.([0-9])/\1@DOT@\2/g
    s/(^|[[:space:]])\.([0-9])/\1@DOT@\2/g
    
    # Защита сокращений (простая версия)
    s/т\.д\./@TD@/g
    s/т\.п\./@TP@/g
    s/т\.е\./@TE@/g
    s/и\.т\.д\./@ITD@/g
    s/и\.т\.п\./@ITP@/g
    
    # Защита инициалов
    s/([А-ЯA-Z])\.([А-ЯA-Z])\./\1@INIT@\2@INIT@/g
    
    # Заглавная в начале документа
    s/^([[:space:]]*)([a-zа-яё])/\1\U\2/
    
    # Заглавные после . ! ?
    s/([.!?])([[:space:]]*)([a-zа-яё])/\1\2\U\3/g
    
    # Восстановление защищённых точек
    s/@DOT@/./g
    s/@TD@/т.д./g
    s/@TP@/т.п./g
    s/@TE@/т.е./g
    s/@ITD@/и т.д./g
    s/@ITP@/и т.п./g
    s/@INIT@/./g
' "$TEMP_FILE" && mv "$TEMP_FILE.tmp" "$TEMP_FILE"

if [ $? -ne 0 ]; then
    rm -f "$TEMP_FILE" "$TEMP_FILE.tmp"
    error "Ошибка при обработке файла sed"
fi

cat "$TEMP_FILE" | tr '\r' '\n'

rm -f "$TEMP_FILE" "$TEMP_FILE.tmp"

exit 0
