# Миграция системы журналирования на MongoDB

## Обзор

Хранилище журналов действий пользователей и системных событий перенесено из PostgreSQL в MongoDB.

## Что реализовано

### 1. MongoDB инфраструктура
- Добавлен сервис MongoDB в `compose.yml`
- Создан клиент для подключения к MongoDB (`mongo_client.py`)
- Настроены переменные окружения для MongoDB

### 2. Коллекции логов в MongoDB

#### action_logs
Хранит все операции пользователя:
- Вход/выход (user_login, user_logout)
- Регистрация (user_registration)
- Создание/обновление/удаление объектов
- Изменение статусов
- Платежи

Поля:
- `actor_user_id`: ID пользователя, выполнившего действие
- `actor_email`: Email пользователя
- `action_type`: Тип действия
- `target_user_id/car_id/rental_id`: ID целевого объекта
- `description`: Описание
- `old_values`/`new_values`: Старые/новые значения
- `user_agent`: User-Agent клиента
- `ip_address`: IP-адрес
- `created_at`: Время создания

#### db_query_logs
Хранит события запросов к реляционной БД:
- `query`: Текст запроса
- `query_type`: SELECT, INSERT, UPDATE, DELETE
- `table_name`: Имя таблицы
- `execution_time_ms`: Время выполнения
- `rows_affected`: Количество затронутых строк
- `user_id`: ID пользователя
- `endpoint`: API endpoint
- `created_at`: Время создания

#### error_logs
Хранит ошибки и исключения приложения:
- `error_type`: Тип исключения
- `error_message`: Сообщение об ошибке
- `stack_trace`: Трассировка стека
- `endpoint`: API endpoint
- `user_id`/`user_email`: Пользователь
- `request_method`/`request_url`/`request_body`: Данные запроса
- `severity`: Уровень (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- `created_at`: Время создания

### 3. TTL-индексы для автоудаления

Автоматическое удаление старых логов настроено через TTL-индексы:

| Коллекция | TTL | Переменная окружения |
|-----------|-----|---------------------|
| action_logs | 90 дней | ACTION_LOGS_TTL_DAYS |
| db_query_logs | 30 дней | DB_QUERY_LOGS_TTL_DAYS |
| error_logs | 60 дней | ERROR_LOGS_TTL_DAYS |

### 4. API endpoints

### Action Logs
```
GET    /action_logs/              # Список логов с фильтрацией
GET    /action_logs/count         # Количество логов
GET    /action_logs/{log_id}      # Конкретный лог
POST   /action_logs/              # Создать лог
DELETE /action_logs/{log_id}      # Удалить лог
```

Параметры фильтрации:
- `start_date` / `end_date`: Временной интервал (ISO 8601)
- `user_id`: ID пользователя
- `action_type`: Тип действия
- `target_user_id` / `target_car_id`: Целевой объект
- `offset` / `limit`: Пагинация

### Analytics Reports (MongoDB Aggregation)

#### Статистика активности по периодам
```
GET /analytics/user-activity?period=day|week|month&start_date=...&end_date=...
```

Возвращает:
- `period`: Период (дата/неделя/месяц)
- `total_actions`: Общее количество действий
- `unique_users_count`: Количество уникальных пользователей
- `actions_by_type`: Распределение по типам действий

#### ТОП-10 самых активных пользователей
```
GET /analytics/top-users?limit=10&start_date=...&end_date=...
```

Возвращает:
- `user_id`: ID пользователя
- `email`: Email пользователя
- `total_actions`: Общее количество действий
- `actions_by_type`: Распределение по типам действий
- `last_action`: Время последнего действия
- `first_action`: Время первого действия

#### Распределение операций по типам (CRUD)
```
GET /analytics/operations-distribution?start_date=...&end_date=...
```

Возвращает:
- `total_operations`: Общее количество операций
- `by_type`: Распределение по типам действий (с процентами)
- `crud_distribution`: Распределение по CRUD операциям

#### Временные тренды (Time Series Analysis)
```
GET /analytics/time-series?period=hour|day_of_week|hour_of_day&start_date=...&end_date=...
```

Возвращает:
- `total_periods`: Количество периодов
- `avg_actions`: Среднее количество действий за период
- `max_actions`: Максимальное количество действий
- `min_actions`: Минимальное количество действий
- `trend_data`: Детальные данные по периодам

#### Аномалии в поведении пользователей
```
GET /analytics/anomalies?std_threshold=2.0&start_date=...&end_date=...
```

Возвращает:
- `anomalies`: Список аномалий с деталями
  - `user_id`, `email`: Данные пользователя
  - `action_count`: Количество действий
  - `anomaly_types`: Типы аномалий (high_activity, low_activity, diverse_actions)
  - `deviation`: Отклонение от среднего
- `statistics`: Статистика анализа

#### Экспорт отчётов
```
# JSON экспорт
GET /analytics/export/user-activity/json?period=day
GET /analytics/export/top-users/json?limit=10

# CSV экспорт
GET /analytics/export/user-activity/csv?period=day
GET /analytics/export/top-users/csv?limit=10
GET /analytics/export/operations-distribution/json
GET /analytics/export/anomalies/json
```

#### DB Query Logs
```
GET /logs/db_queries       # Список запросов к БД
GET /logs/db_queries/count # Количество запросов
```

Параметры фильтрации:
- `start_date` / `end_date`
- `table_name`
- `query_type`
- `user_id`
- `endpoint`

#### Error Logs
```
GET /logs/errors       # Список ошибок
GET /logs/errors/count # Количество ошибок
```

Параметры фильтрации:
- `start_date` / `end_date`
- `error_type`
- `severity`
- `user_id`
- `endpoint`

### 5. Frontend

#### Action Logs
Обновлена страница просмотра action logs (`/admin/action_logs`):

- Фильтры по временному интервалу (дата начала/окончания)
- Фильтр по типу действия (dropdown)
- Фильтр по ID пользователя
- Текстовые фильтры по полям таблицы
- Пагинация

#### Analytics Dashboard
Добавлена новая страница аналитики (`/admin/analytics`):

**Визуализация данных (Chart.js):**
- 📊 Активность по периодам (бар-чарт)
- 📈 Распределение CRUD операций (doughnut-чарт)
- ⏰ Временные тренды (line-чарт)
- 👥 ТОП-10 пользователей (horizontal bar-чарт)

**Таблицы:**
- ТОП-10 самых активных пользователей
- Аномалии в поведении пользователей

**Функционал:**
- Фильтры по датам и периодам
- Кнопка экспорта всех отчётов (JSON)
- Автоматическая загрузка при открытии
- Responsive дизайн

### 6. Middleware

#### MongoDBLoggingMiddleware
Автоматически логирует:
- HTTP 5xx ошибки
- Необработанные исключения

#### DatabaseQueryLoggingMiddleware
Класс для логирования SQL-запросов (требует интеграции с database.py)

## Запуск

```bash
docker compose up --build -d
```

Сервисы:
- PostgreSQL: localhost:5432
- MongoDB: localhost:27017
- Redis: localhost:6379
- Web API: localhost:8000

## Проверка работы

### Через API
```bash
# Логин (создаст запись в action_logs)
curl -X POST http://localhost:8000/users/login \
  -H "Content-Type: application/json" \
  -d '{"email":"admin@example.com","password":"admin123"}'

# Получить логи действий
curl -H "Authorization: Bearer <TOKEN>" \
  "http://localhost:8000/action_logs/?offset=0&limit=10"

# Фильтрация по времени
curl -H "Authorization: Bearer <TOKEN>" \
  "http://localhost:8000/action_logs/?start_date=2026-03-18T00:00:00Z&end_date=2026-03-19T00:00:00Z&action_type=user_login"
```

### Через MongoDB shell
```bash
docker exec carsharing_mongodb mongosh carsharing_logs

# Просмотр логов
db.action_logs.find().limit(10)

# Проверка индексов
db.action_logs.getIndexes()

# Просмотр коллекции
show collections
```

## Конфигурация

Переменные окружения (`.env`):

```env
MONGODB_HOST=mongodb
MONGODB_PORT=27017
MONGODB_DB=carsharing_logs

ACTION_LOGS_TTL_DAYS=90
DB_QUERY_LOGS_TTL_DAYS=30
ERROR_LOGS_TTL_DAYS=60
```

## Структура файлов

```
Carsharing/
├── mongo_client.py              # Клиент MongoDB
├── middleware/
│   └── mongo_logging.py         # Middleware для логирования
├── crud/
│   └── mongo_logs_crud.py       # CRUD операции для логов
├── routers/
│   ├── action_logs_router.py    # API для action logs
│   └── logs_router.py           # API для DB query и error logs
├── schemas.py                   # Pydantic модели (добавлены MongoDB схемы)
├── frontend/
│   ├── admin_action_logs.html   # Страница просмотра логов
│   └── admin_action_logs.js     # JavaScript для фильтров
└── compose.yml                  # Docker Compose с MongoDB
```

## Примечания

1. Старая таблица `action_logs` в PostgreSQL сохраняется для обратной совместимости
2. Все новые записи пишутся только в MongoDB
3. Для полноценного логирования SQL-запросов требуется доработка `database.py`
