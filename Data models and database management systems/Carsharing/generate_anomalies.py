#!/usr/bin/env python3
"""
Скрипт для генерации тестовых данных с аномалиями в поведении пользователей.

Создаёт пользователей с подозрительным поведением:
1. Пользователь с высокой активностью (high_activity)
2. Пользователь с разнообразными действиями (diverse_actions)
"""

import sys
from datetime import datetime, timedelta
from pathlib import Path

# Добавляем корень проекта в path
sys.path.insert(0, str(Path(__file__).parent))

from crud.analytics_crud import get_action_logs_collection
from bson import ObjectId


def generate_anomalies():
    """Генерация аномальных данных пользователей."""
    
    collection = get_action_logs_collection()
    
    print("🔍 Очистка старых тестовых данных...")
    # Удаляем старые тестовые записи
    collection.delete_many({
        "actor_email": {"$regex": "^anomaly_test_"}
    })
    
    now = datetime.utcnow()
    
    # ========================================================================
    # 1. Создаём "нормальных" пользователей (для статистики)
    # ========================================================================
    print("\n👥 Создание 10 нормальных пользователей (по 5 действий каждый)...")
    
    normal_users = []
    for i in range(10):
        user_id = ObjectId()
        email = f"normal_user_{i}@example.com"
        normal_users.append({"user_id": user_id, "email": email})
        
        # Каждый пользователь делает 3-7 действий (нормальная активность)
        for j in range(3, 8):
            collection.insert_one({
                "actor_user_id": user_id,
                "actor_email": email,
                "action_type": "user_login" if j % 2 == 0 else "profile_update",
                "entity_type": "user",
                "timestamp": now - timedelta(days=j, hours=i),
                "details": {"test_data": True, "user_type": "normal"},
                "created_at": now - timedelta(days=j, hours=i)
            })
    
    print(f"   ✅ Создано {len(normal_users)} нормальных пользователей")
    
    # ========================================================================
    # 2. Пользователь с ВЫСОКОЙ активностью (high_activity)
    # ========================================================================
    print("\n🚀 Создание пользователя с ВЫСОКОЙ активностью (50 действий)...")
    
    high_activity_user_id = ObjectId()
    high_activity_email = "anomaly_test_high_activity@spam.com"
    
    for i in range(50):
        collection.insert_one({
            "actor_user_id": high_activity_user_id,
            "actor_email": high_activity_email,
            "action_type": "user_login",
            "entity_type": "user",
            "timestamp": now - timedelta(minutes=i * 10),
            "details": {"test_data": True, "anomaly_type": "high_activity"},
            "created_at": now - timedelta(minutes=i * 10)
        })
    
    print(f"   ✅ Создан пользователь {high_activity_email} с 50 действиями")
    
    # ========================================================================
    # 3. Пользователь с РАЗНООБРАЗНЫМИ действиями (diverse_actions)
    # ========================================================================
    print("\n🎭 Создание пользователя с РАЗНООБРАЗНЫМИ действиями (>10 типов)...")
    
    diverse_user_id = ObjectId()
    diverse_email = "anomaly_test_diverse_actions@spam.com"
    
    # Все доступные типы действий в системе
    action_types = [
        "user_login",
        "user_registration",
        "user_logout",
        "car_rental_start",
        "rental_return_completed",
        "rental_cancelled_by_admin",
        "rental_cancelled_by_user",
        "trip_completion_create",
        "trip_completion_approved",
        "trip_completion_rejected",
        "driver_license_upload",
        "driver_license_approved",
        "driver_license_rejected",
        "profile_update"
    ]
    
    # Пользователь выполняет все 14 типов действий
    for i, action_type in enumerate(action_types):
        collection.insert_one({
            "actor_user_id": diverse_user_id,
            "actor_email": diverse_email,
            "action_type": action_type,
            "entity_type": "user",
            "timestamp": now - timedelta(hours=i),
            "details": {"test_data": True, "anomaly_type": "diverse_actions"},
            "created_at": now - timedelta(hours=i)
        })
    
    print(f"   ✅ Создан пользователь {diverse_email} с {len(action_types)} типами действий")
    
    # ========================================================================
    # 4. Пользователь с КОМБИНИРОВАННОЙ аномалией (high_activity + diverse_actions)
    # ========================================================================
    print("\n🔥 Создание пользователя с КОМБИНИРОВАННОЙ аномалией...")
    
    combo_user_id = ObjectId()
    combo_email = "anomaly_test_combo@spam.com"
    
    # 40 действий с 12 различными типами
    for i in range(40):
        action_type = action_types[i % len(action_types)]
        collection.insert_one({
            "actor_user_id": combo_user_id,
            "actor_email": combo_email,
            "action_type": action_type,
            "entity_type": "user",
            "timestamp": now - timedelta(minutes=i * 15),
            "details": {"test_data": True, "anomaly_type": "combo"},
            "created_at": now - timedelta(minutes=i * 15)
        })
    
    print(f"   ✅ Создан пользователь {combo_email} с 40 действиями и 12 типами")
    
    # ========================================================================
    # Итоги
    # ========================================================================
    print("\n" + "=" * 60)
    print("📊 ИТОГИ:")
    print("=" * 60)
    print(f"   Нормальных пользователей:     10 (по 5-7 действий)")
    print(f"   High activity:                1 (50 действий)")
    print(f"   Diverse actions:              1 (14 типов действий)")
    print(f"   Combo (high + diverse):       1 (40 действий, 12 типов)")
    print("=" * 60)
    
    print("\n🔗 Проверка через API:")
    print("   curl -H 'Authorization: Bearer <TOKEN>' 'http://localhost:8000/analytics/anomalies?std_threshold=2.0'")
    print("\n   Или откройте frontend и войдите как admin:")
    print("   Email: admin@example.com")
    print("   Password: admin123")
    print("   URL: http://localhost:8000/frontend/admin_analytics.html")
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
