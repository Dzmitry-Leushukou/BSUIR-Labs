#!/usr/bin/env python3

import requests
import json

BASE_URL = "http://localhost:8000"

ADMIN_EMAIL = "admin@example.com"
ADMIN_PASSWORD = "admin123"


def get_auth_token():
    response = requests.post(
        f"{BASE_URL}/users/login",
        json={"email": ADMIN_EMAIL, "password": ADMIN_PASSWORD}
    )

    if response.status_code == 200:
        data = response.json()
        return data.get("access_token") or data.get("token")
    else:
        print(f"❌ Ошибка аутентификации: {response.status_code}")
        print(response.text)
        return None


def check_anomalies(std_threshold=2.0):
    token = get_auth_token()

    if not token:
        print("❌ Не удалось получить токен")
        return

    headers = {"Authorization": f"Bearer {token}"}

    response = requests.get(
        f"{BASE_URL}/analytics/anomalies",
        params={"std_threshold": std_threshold},
        headers=headers
    )

    print(f"\n📊 Статус запроса: {response.status_code}")

    if response.status_code == 200:
        data = response.json()
        print("\n✅ Аномалии обнаружены:")
        print(json.dumps(data, indent=2, ensure_ascii=False))

        anomalies = data.get("anomalies", [])
        statistics = data.get("statistics", {})

        print("\n" + "=" * 60)
        print("📈 СТАТИСТИКА:")
        print(f"   Среднее действий/пользователя: {statistics.get('mean_actions_per_user', 'N/A')}")
        print(f"   Стандартное отклонение: {statistics.get('std_deviation', 'N/A')}")
        print(f"   Порог (threshold): {statistics.get('threshold', 'N/A')}")
        print(f"   Всего пользователей проанализировано: {statistics.get('total_users_analyzed', 'N/A')}")
        print(f"   Аномалий обнаружено: {statistics.get('anomalies_detected', 'N/A')}")
        print("=" * 60)

        if anomalies:
            print("\n⚠️ СПИСОК АНОМАЛИЙ:")
            for i, anomaly in enumerate(anomalies, 1):
                print(f"\n{i}. {anomaly.get('email', 'N/A')}")
                print(f"   Действий: {anomaly.get('action_count', 'N/A')}")
                print(f"   Уникальных типов: {anomaly.get('unique_action_types_count', 'N/A')}")
                print(f"   Отклонение: {anomaly.get('deviation', 'N/A')}σ")
                print(f"   Типы: {', '.join(anomaly.get('anomaly_types', []))}")
        else:
            print("\n❌ Аномалий не обнаружено")
    else:
        print(f"❌ Ошибка API: {response.status_code}")
        print(response.text)


if __name__ == "__main__":
    try:
        check_anomalies(std_threshold=2.0)
    except Exception as e:
        print(f"❌ Ошибка: {e}")
        import traceback
        traceback.print_exc()
