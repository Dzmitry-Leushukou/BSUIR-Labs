"""Генерация тестовых данных для MongoDB (большие объёмы)."""
import csv
import os
import random
from datetime import datetime, timedelta

DATA_DIR = "Data_csv"
os.makedirs(DATA_DIR, exist_ok=True)

NUM_CATEGORIES = 50
NUM_USERS = 1000
NUM_PRODUCTS = 2000
NUM_ORDERS = 10000
MAX_ITEMS_PER_ORDER = 5

category_names_ru = [
    "Электроника", "Смартфоны", "Ноутбуки", "Планшеты", "Телевизоры",
    "Наушники", "Клавиатуры", "Мышки", "Мониторы", "Принтеры",
    "Одежда мужская", "Одежда женская", "Обувь", "Аксессуары", "Сумки",
    "Книги художественные", "Книги учебные", "Книги научные", "Журналы", "Комиксы",
    "Мебель", "Освещение", "Текстиль", "Декор", "Посуда",
    "Спорт", "Фитнес", "Велосипеды", "Туризм", "Рыбалка",
    "Красота", "Здоровье", "Парфюмерия", "Косметика", "Уход за кожей",
    "Продукты", "Напитки", "Сладости", "Специи", "Консервы",
    "Автозапчасти", "Автохимия", "Автоэлектроника", "Шины", "Аккумуляторы",
    "Игрушки", "Настольные игры", "Головоломки", "Конструкторы", "Куклы"
]

product_prefixes = {
    1: "iPhone", 2: "Galaxy", 3: "ThinkPad", 4: "iPad", 5: "Bravia",
    6: "Sony", 7: "Cherry", 8: "Logitech", 9: "Dell", 10: "HP",
    11: "Куртка", 12: "Платье", 13: "Кроссовки", 14: "Часы", 15: "Рюкзак",
    16: "Роман", 17: "Учебник", 18: "Монография", 19: "Подписка", 20: "Альбом",
    21: "Диван", 22: "Люстра", 23: "Плед", 24: "Ваза", 25: "Сервиз",
    26: "Гантели", 27: "Коврик", 28: "Велосипед", 29: "Палатка", 30: "Удочка",
    31: "Крем", 32: "Витамины", 33: "Духи", 34: "Помада", 35: "Сыворотка",
    36: "Кофе", 37: "Сок", 38: "Шоколад", 39: "Перец", 40: "Тушёнка",
    41: "Фильтр", 42: "Шампунь", 43: "Видеорегистратор", 44: "Зимняя шина", 45: "Батарея",
    46: "LEGO", 47: "Монополия", 48: "Кубик Рубика", 49: "Техник", 50: "Barbie"
}

random.seed(42)

# ===================== CATEGORIES =====================
categories_file = os.path.join(DATA_DIR, "categoryid-name.csv")
with open(categories_file, "w", encoding="utf-8", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["category_id", "name"])
    for i in range(1, NUM_CATEGORIES + 1):
        name = category_names_ru[i - 1] if i <= len(category_names_ru) else f"Категория {i}"
        writer.writerow([i, name])
print(f"Categories: {NUM_CATEGORIES}")

# ===================== USERS =====================
users_file = os.path.join(DATA_DIR, "userid-name-email-createdat.csv")
first_names = ["Александр", "Мария", "Дмитрий", "Елена", "Андрей", "Ольга", "Сергей", "Наталья",
               "Иван", "Анна", "Михаил", "Екатерина", "Артём", "Татьяна", "Николай", "Ирина",
               "Алексей", "Светлана", "Владимир", "Юлия", "Павел", "Марина", "Роман", "Виктория"]
last_names = ["Петров", "Иванова", "Сидоров", "Козлова", "Новиков", "Смирнова", "Волков", "Павлова",
              "Соколов", "Морозова", "Лебедев", "Виноградова", "Попов", "Богданова", "Кузнецов", "Орлова"]
with open(users_file, "w", encoding="utf-8", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["user_id", "name", "email", "created_at"])
    for i in range(1, NUM_USERS + 1):
        fn = random.choice(first_names)
        ln = random.choice(last_names)
        created = datetime(2023, 1, 1) + timedelta(days=random.randint(0, 364))  # 2023 год
        writer.writerow([i, f"{ln} {fn}", f"user{i}@example.com", created.strftime("%Y-%m-%d %H:%M:%S")])
print(f"Users: {NUM_USERS}")

# ===================== PRODUCTS =====================
products_file = os.path.join(DATA_DIR, "productid-name-categoryid-price.csv")
with open(products_file, "w", encoding="utf-8", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["product_id", "name", "category_id", "price"])
    for i in range(1, NUM_PRODUCTS + 1):
        cat_id = (i - 1) % NUM_CATEGORIES + 1
        cat_idx = (cat_id - 1) % 50
        prefix = product_prefixes.get(cat_idx + 1, "Товар")
        name = f"{prefix} {i}"
        price = round(random.uniform(50, 5000), 2)
        writer.writerow([i, name, cat_id, price])
print(f"Products: {NUM_PRODUCTS}")

# ===================== ORDERS =====================
orders_file = os.path.join(DATA_DIR, "orderid-userid-createdat-status.csv")
statuses = ["pending", "processing", "shipped", "delivered", "cancelled"]
status_weights = [0.1, 0.15, 0.2, 0.45, 0.1]
with open(orders_file, "w", encoding="utf-8", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["order_id", "user_id", "created_at", "status"])
    for i in range(1, NUM_ORDERS + 1):
        uid = random.randint(1, NUM_USERS)
        created = datetime(2024, 1, 1) + timedelta(
            days=random.randint(0, 730),  # 2024-2025 годы
            hours=random.randint(0, 23),
            minutes=random.randint(0, 59)
        )
        status = random.choices(statuses, weights=status_weights, k=1)[0]
        writer.writerow([i, uid, created.strftime("%Y-%m-%d %H:%M:%S"), status])
print(f"Orders: {NUM_ORDERS}")

# ===================== ORDER ITEMS =====================
# Генерируем коррелированные покупки: товары из одной категории чаще покупаются вместе
product_categories = {}
for i in range(1, NUM_PRODUCTS + 1):
    cat_id = (i - 1) % NUM_CATEGORIES + 1
    product_categories[i] = cat_id

# Группируем продукты по категориям
category_products = {}
for pid, cid in product_categories.items():
    category_products.setdefault(cid, []).append(pid)

items_file = os.path.join(DATA_DIR, "orderitemid-orderid-productid-quantity-price.csv")
order_item_id = 1
with open(items_file, "w", encoding="utf-8", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["order_item_id", "order_id", "product_id", "quantity", "price"])
    for oid in range(1, NUM_ORDERS + 1):
        n_items = random.randint(1, MAX_ITEMS_PER_ORDER)
        # Выбираем "базовую" категорию для заказа (70% шанс)
        if random.random() < 0.7:
            base_cat = random.randint(1, NUM_CATEGORIES)
            # Все товары заказа из одной категории (создаёт корреляции)
            cat_prods = category_products.get(base_cat, list(range(1, NUM_PRODUCTS + 1)))
            chosen = random.choices(cat_prods, k=n_items)
        else:
            chosen = [random.randint(1, NUM_PRODUCTS) for _ in range(n_items)]
        for pid in chosen:
            qty = random.randint(1, 10)
            price = round(random.uniform(50, 5000), 2)
            writer.writerow([order_item_id, oid, pid, qty, price])
            order_item_id += 1
print(f"Order items: {order_item_id - 1}")
print(f"\nAll files saved to {DATA_DIR}/")
