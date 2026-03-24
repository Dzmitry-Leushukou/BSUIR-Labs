#!/usr/bin/env python3
"""
Скрипт для генерации тестовых данных в MongoDB коллекцию action_logs.
Создает нормальных пользователей и пользователей с аномальным поведением.
"""

import sys
import random
from datetime import datetime, timedelta
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from crud.analytics_crud import get_action_logs_collection
from bson import ObjectId


# Реалистичные данные для генерации
ACTION_TYPES = [
    "user_login",
    "user_logout",
    "user_registration",
    "profile_update",
    "car_create",
    "car_update",
    "car_delete",
    "car_rental_start",
    "car_rental_end",
    "car_rental_cancel",
    "car_rental_pending_completion",
    "driver_license_upload",
    "driver_license_approved",
    "driver_license_rejected",
    "trip_completion_create",
    "trip_completion_approved",
    "trip_completion_rejected",
    "user_ban",
    "user_unban",
    "payment_success",
    "payment_failed",
    "maintenance_request",
]

CAR_VINS = [
    "WBA3A5C55CF123456",
    "WVWZZZ3CZWE123456",
    "XTA219000J0123456",
    "JF2SJADC5JH123456",
    "5XYZUDLB8JG123456",
    "1HGBH41JXMN123456",
    "WBADT43452G123456",
    "WDDGF4HB1EA123456",
    "YV1MS682962123456",
    "SAJWA0HC8JL123456",
]

USER_AGENTS = [
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 14_6 like Mac OS X) AppleWebKit/605.1.15",
    "Mozilla/5.0 (Android 11; Mobile; rv:89.0) Gecko/89.0 Firefox/89.0",
]


def generate_anomalies():
    """Генерация тестовых данных с нормальными и аномальными пользователями."""

    collection = get_action_logs_collection()

    print("🔍 Очистка старых тестовых данных...")
    collection.delete_many({
        "actor_email": {"$regex": "^(normal_user_|anomaly_test_|admin_|user_)"}
    })

    now = datetime.utcnow()

    # =========================================================================
    # Нормальные пользователи
    # =========================================================================
    print("\n👥 Создание 10 нормальных пользователей (по 5-8 действий каждый)...")

    normal_users = []
    for i in range(10):
        user_id = ObjectId()
        email = f"normal_user_{i}@example.com"
        normal_users.append({"user_id": user_id, "email": email})

        # Генерируем 5-8 случайных действий за последние 30 дней
        num_actions = random.randint(5, 8)
        for j in range(num_actions):
            action_type = random.choice(ACTION_TYPES[:10])  # Только обычные действия
            days_ago = random.randint(1, 30)
            hours_ago = random.randint(0, 23)
            action_time = now - timedelta(days=days_ago, hours=hours_ago)

            log_entry = {
                "actor_user_id": user_id,
                "actor_email": email,
                "action_type": action_type,
                "description": f"Пользователь выполнил {action_type}",
                "target_user_id": None,
                "target_car_id": None,
                "target_rental_id": None,
                "target_car_vin": None,
                "target_user_email": None,
                "old_values": None,
                "new_values": None,
                "user_agent": random.choice(USER_AGENTS),
                "ip_address": f"192.168.1.{random.randint(1, 254)}",
                "created_at": action_time,
            }

            # Добавляем target данные для некоторых действий
            if action_type in ["car_create", "car_update", "car_delete"]:
                log_entry["target_car_vin"] = random.choice(CAR_VINS)
            elif action_type in ["user_ban", "user_unban", "profile_update"]:
                log_entry["target_user_id"] = user_id
                log_entry["target_user_email"] = email

            collection.insert_one(log_entry)

    print(f"   ✅ Создано {len(normal_users)} нормальных пользователей")

    # =========================================================================
    # Пользователь с ВЫСОКОЙ активностью (50+ действий)
    # =========================================================================
    print("\n🚀 Создание пользователя с ВЫСОКОЙ активностью (50 действий)...")

    high_activity_user_id = ObjectId()
    high_activity_email = "anomaly_test_high_activity@spam.com"

    for i in range(50):
        action_time = now - timedelta(minutes=i * 10)
        collection.insert_one({
            "actor_user_id": high_activity_user_id,
            "actor_email": high_activity_email,
            "action_type": "user_login",
            "description": "Вход пользователя",
            "target_user_id": None,
            "target_car_id": None,
            "target_rental_id": None,
            "target_car_vin": None,
            "target_user_email": None,
            "old_values": None,
            "new_values": None,
            "user_agent": random.choice(USER_AGENTS),
            "ip_address": f"10.0.0.{random.randint(1, 254)}",
            "created_at": action_time,
        })

    print(f"   ✅ Создан пользователь {high_activity_email} с 50 действиями")

    # =========================================================================
    # Пользователь с РАЗНООБРАЗНЫМИ действиями (>10 типов)
    # =========================================================================
    print("\n🎭 Создание пользователя с РАЗНООБРАЗНЫМИ действиями (>10 типов)...")

    diverse_user_id = ObjectId()
    diverse_email = "anomaly_test_diverse_actions@spam.com"

    # Используем более 10 различных типов действий
    diverse_action_types = random.sample(ACTION_TYPES, min(14, len(ACTION_TYPES)))

    for i, action_type in enumerate(diverse_action_types):
        action_time = now - timedelta(hours=i)
        log_entry = {
            "actor_user_id": diverse_user_id,
            "actor_email": diverse_email,
            "action_type": action_type,
            "description": f"Действие: {action_type}",
            "target_user_id": None,
            "target_car_id": None,
            "target_rental_id": None,
            "target_car_vin": None,
            "target_user_email": None,
            "old_values": None,
            "new_values": None,
            "user_agent": random.choice(USER_AGENTS),
            "ip_address": f"172.16.0.{random.randint(1, 254)}",
            "created_at": action_time,
        }

        # Добавляем контекст для некоторых действий
        if "car" in action_type:
            log_entry["target_car_vin"] = random.choice(CAR_VINS)
        if "payment" in action_type:
            log_entry["new_values"] = {"amount": random.uniform(100, 5000), "status": "success"}

        collection.insert_one(log_entry)

    print(f"   ✅ Создан пользователь {diverse_email} с {len(diverse_action_types)} типами действий")

    # =========================================================================
    # Пользователь с КОМБИНИРОВАННОЙ аномалией (высокая активность + разнообразие)
    # =========================================================================
    print("\n🔥 Создание пользователя с КОМБИНИРОВАННОЙ аномалией...")

    combo_user_id = ObjectId()
    combo_email = "anomaly_test_combo@spam.com"

    for i in range(40):
        action_type = ACTION_TYPES[i % len(ACTION_TYPES)]
        action_time = now - timedelta(minutes=i * 15)
        log_entry = {
            "actor_user_id": combo_user_id,
            "actor_email": combo_email,
            "action_type": action_type,
            "description": f"Комбинированное действие: {action_type}",
            "target_user_id": None,
            "target_car_id": None,
            "target_rental_id": None,
            "target_car_vin": random.choice(CAR_VINS) if "car" in action_type else None,
            "target_user_email": None,
            "old_values": None,
            "new_values": None,
            "user_agent": random.choice(USER_AGENTS),
            "ip_address": f"192.168.100.{random.randint(1, 254)}",
            "created_at": action_time,
        }
        collection.insert_one(log_entry)

    print(f"   ✅ Создан пользователь {combo_email} с 40 действиями и {len(ACTION_TYPES)} типами")

    # =========================================================================
    # Администратор с подозрительной активностью (много банов/разбанов)
    # =========================================================================
    print("\n👮 Создание администратора с подозрительной активностью...")

    admin_user_id = ObjectId()
    admin_email = "anomaly_test_suspicious_admin@spam.com"

    target_users = [ObjectId() for _ in range(20)]

    for i in range(30):
        action_type = random.choice(["user_ban", "user_unban"])
        action_time = now - timedelta(hours=i * 2)
        target_user = random.choice(target_users)

        collection.insert_one({
            "actor_user_id": admin_user_id,
            "actor_email": admin_email,
            "action_type": action_type,
            "description": f"Администратор выполнил {action_type}",
            "target_user_id": target_user,
            "target_user_email": f"user_{i}@example.com",
            "target_car_id": None,
            "target_rental_id": None,
            "target_car_vin": None,
            "old_values": {"status": "active"} if action_type == "user_ban" else {"status": "banned"},
            "new_values": {"status": "banned"} if action_type == "user_ban" else {"status": "active"},
            "user_agent": random.choice(USER_AGENTS),
            "ip_address": "10.10.10.10",
            "created_at": action_time,
        })

    print(f"   ✅ Создан администратор {admin_email} с 30 банами/разбанами")

    # =========================================================================
    # Пользователь с частыми платежами (возможный фрод)
    # =========================================================================
    print("\n💰 Создание пользователя с подозрительными платежами...")

    fraud_user_id = ObjectId()
    fraud_email = "anomaly_test_payment_fraud@spam.com"

    for i in range(25):
        action_time = now - timedelta(hours=i)
        amount = random.uniform(5000, 50000)  # Подозрительно большие суммы

        collection.insert_one({
            "actor_user_id": fraud_user_id,
            "actor_email": fraud_email,
            "action_type": "payment_success",
            "description": "Успешный платеж",
            "target_user_id": None,
            "target_car_id": None,
            "target_rental_id": i + 1,
            "target_car_vin": None,
            "target_user_email": None,
            "old_values": None,
            "new_values": {"amount": round(amount, 2), "status": "completed"},
            "user_agent": random.choice(USER_AGENTS),
            "ip_address": f"203.0.113.{random.randint(1, 254)}",
            "created_at": action_time,
        })

    print(f"   ✅ Создан пользователь {fraud_email} с 25 платежами")

    # =========================================================================
    # ИТОГИ
    # =========================================================================
    print("\n" + "=" * 60)
    print("📊 ИТОГИ:")
    print("=" * 60)
    print(f"   Нормальных пользователей:     10 (по 5-8 действий)")
    print(f"   High activity:                1 (50 действий)")
    print(f"   Diverse actions:              1 ({len(diverse_action_types)} типов действий)")
    print(f"   Combo (high + diverse):       1 (40 действий, {len(ACTION_TYPES)} типов)")
    print(f"   Suspicious admin:             1 (30 банов/разбанов)")
    print(f"   Payment fraud:                1 (25 платежей)")
    print("=" * 60)

    total_logs = collection.count_documents({})
    print(f"\n📈 Всего записей в коллекции: {total_logs}")

    print("\n🔗 Проверка через API:")
    print("   curl -H 'Authorization: Bearer <TOKEN>' 'http://localhost:8000/analytics/anomalies?std_threshold=2.0'")
    print("\n   Или откройте frontend и войдите как admin:")
    print("   Email: admin@example.com")
    print("   Password: admin123")
    print("   URL: http://localhost:8000/admin/analytics")
    print("=" * 60)


if __name__ == "__main__":
    try:
        generate_anomalies()
        print("\n✅ Готово!\n")
    except Exception as e:
        print(f"\n❌ Ошибка: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
