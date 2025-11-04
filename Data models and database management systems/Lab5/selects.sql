\c carsharing_db;
SET TIME ZONE 'UTC';

-- -----------------------------------------------------------------------------
-- 1) Активные пользователи с их ролью (INNER JOIN) + сложные условия
--    Функция ЛР1: просмотр каталога пользователей
SELECT u.id, u.email, u.name, u.surname, r.name AS role_name, u.cashback
FROM users u
JOIN roles r ON r.id = u.role_id
WHERE u.status = 'active'
  AND (u.cashback >= 100 OR u.created_at >= NOW() - INTERVAL '30 days')
ORDER BY u.cashback DESC, u.created_at DESC;

-- -----------------------------------------------------------------------------
-- 2) Доступные сейчас машины (NOT EXISTS активной аренды) — [СЛОЖНЫЙ]
--    Функция ЛР1: найти свободные авто для старта аренды
SELECT c.id, c.plate_number, c.model, c.status
FROM cars c
WHERE c.status = 'available'
  AND NOT EXISTS (
      SELECT 1
      FROM rentals r
      WHERE r.car_id = c.id
        AND r.status = 'active'
  )
ORDER BY c.updated_at DESC;

-- -----------------------------------------------------------------------------
-- 3) Текущие активные аренды с пользователем и авто (множественные JOIN)
--    Функция ЛР1: мониторинг активных аренд
SELECT r.id AS rental_id, r.started_at, u.email, CONCAT(u.name,' ',u.surname) AS user_fullname,
       c.plate_number, c.model
FROM rentals r
JOIN users   u ON u.id = r.user_id
JOIN cars    c ON c.id = r.car_id
WHERE r.status = 'active'
ORDER BY r.started_at DESC;

-- -----------------------------------------------------------------------------
-- 4) Кол-во аренд по модели авто (GROUP BY + агрегат)
--    Функция ЛР1: аналитика использования автопарка
SELECT c.model, COUNT(*) AS rentals_cnt
FROM rentals r
JOIN cars c ON c.id = r.car_id
GROUP BY c.model
ORDER BY rentals_cnt DESC, c.model;

-- -----------------------------------------------------------------------------
-- 5) Модели с более чем 10 аренд (HAVING)
SELECT c.model, COUNT(*) AS rentals_cnt
FROM rentals r
JOIN cars c ON c.id = r.car_id
GROUP BY c.model
HAVING COUNT(*) > 10
ORDER BY rentals_cnt DESC;

-- -----------------------------------------------------------------------------
-- 6) Выручка по пользователям + ранжирование (SUM() OVER, RANK) — [СЛОЖНЫЙ]
--    Функция ЛР1: лидеры по оплатам
WITH revenue AS (
  SELECT pl.user_id, SUM(pl.price) AS total_revenue
  FROM payment_logs pl
  GROUP BY pl.user_id
)
SELECT u.id, u.email, COALESCE(rv.total_revenue,0) AS total_revenue,
       RANK() OVER (ORDER BY COALESCE(rv.total_revenue,0) DESC) AS revenue_rank
FROM users u
LEFT JOIN revenue rv ON rv.user_id = u.id
ORDER BY revenue_rank, u.id;

-- -----------------------------------------------------------------------------
-- 7) История аренд с разницей между поездками (LAG) — [СЛОЖНЫЙ]
--    Функция ЛР1: поведение пользователя
SELECT r.user_id, r.id AS rental_id, r.started_at,
       LAG(r.started_at) OVER (PARTITION BY r.user_id ORDER BY r.started_at) AS prev_started_at,
       EXTRACT(EPOCH FROM (r.started_at - LAG(r.started_at) OVER (PARTITION BY r.user_id ORDER BY r.started_at))) / 3600.0
         AS hours_since_prev
FROM rentals r
ORDER BY r.user_id, r.started_at;

-- -----------------------------------------------------------------------------
-- 8) Каскадные платежи в рамках аренды + накопительный итог (SUM OVER) — [СЛОЖНЫЙ]
SELECT pl.rental_id, pl.id AS payment_id, pl.price,
       SUM(pl.price) OVER (PARTITION BY pl.rental_id ORDER BY pl.id
                           ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS cum_paid
FROM payment_logs pl
ORDER BY pl.rental_id, pl.id;

-- -----------------------------------------------------------------------------
-- 9) Уникальный список участников экосистемы (пользователи + заявители ТО) (UNION)
--    Функция ЛР1: коммуникации
SELECT DISTINCT u.email AS contact
FROM users u
UNION
SELECT DISTINCT u2.email AS contact
FROM maintenance_requests mr
JOIN users u2 ON u2.id = mr.reported_by
ORDER BY contact;

-- -----------------------------------------------------------------------------
-- 10) Пользователи с максимальным кэшбэком (скалярный подзапрос)
SELECT id, email, cashback
FROM users
WHERE cashback = (SELECT MAX(u2.cashback) FROM users u2)
ORDER BY id;

-- -----------------------------------------------------------------------------
-- 11) Пользователи без единой аренды (NOT EXISTS)
SELECT u.id, u.email
FROM users u
WHERE NOT EXISTS (
  SELECT 1 FROM rentals r WHERE r.user_id = u.id
)
ORDER BY u.id;

-- -----------------------------------------------------------------------------
-- 12) Роли и пользователи (RIGHT JOIN) — показать роли даже без пользователей
SELECT r.name AS role, u.id AS user_id, u.email
FROM users u
RIGHT JOIN roles r ON r.id = u.role_id
ORDER BY r.name, u.id NULLS LAST;

-- -----------------------------------------------------------------------------
-- 13) Машины и последняя заявка на ТО (FULL OUTER JOIN + агрегат) — [СЛОЖНЫЙ]
WITH last_m AS (
  SELECT car_id, MAX(created_at) AS last_created
  FROM maintenance_requests
  GROUP BY car_id
)
SELECT c.id AS car_id, c.plate_number, lm.last_created
FROM cars c
FULL OUTER JOIN last_m lm ON lm.car_id = c.id
ORDER BY car_id NULLS LAST;

-- -----------------------------------------------------------------------------
-- 14) Ежедневное число аренд за последние 7 дней (CROSS JOIN generate_series) — [СЛОЖНЫЙ]
WITH days AS (
  SELECT generate_series::date AS d
  FROM generate_series((CURRENT_DATE - INTERVAL '6 days')::date, CURRENT_DATE, INTERVAL '1 day')
)
SELECT d.d AS day,
       COUNT(r.id) AS rentals_cnt
FROM days d
LEFT JOIN rentals r ON r.started_at::date = d.d
GROUP BY d.d
ORDER BY d.d;

-- -----------------------------------------------------------------------------
-- 15) Для каждой машины — последний техосмотр/заявка (LATERAL) — [СЛОЖНЫЙ]
SELECT c.id AS car_id, c.plate_number, x.last_kind, x.last_time
FROM cars c
LEFT JOIN LATERAL (
  SELECT 'maintenance_request' AS last_kind, mr.created_at AS last_time
  FROM maintenance_requests mr
  WHERE mr.car_id = c.id
  ORDER BY mr.created_at DESC
  LIMIT 1
) x ON TRUE
ORDER BY c.id;

-- -----------------------------------------------------------------------------
-- 16) Однофамильцы среди пользователей (SELF JOIN)
SELECT u1.id AS user1_id, u2.id AS user2_id, u1.surname
FROM users u1
JOIN users u2 ON u2.surname = u1.surname AND u2.id > u1.id
ORDER BY u1.surname, u1.id, u2.id;

-- -----------------------------------------------------------------------------
-- 17) Классификация длительности аренды (CASE) — [СЛОЖНЫЙ]
SELECT r.id AS rental_id,
       EXTRACT(EPOCH FROM (COALESCE(r.ended_at, NOW()) - r.started_at))/3600.0 AS hours,
       CASE
         WHEN COALESCE(r.ended_at, NOW()) - r.started_at < INTERVAL '1 hour' THEN 'short'
         WHEN COALESCE(r.ended_at, NOW()) - r.started_at < INTERVAL '6 hours' THEN 'medium'
         ELSE 'long'
       END AS duration_bucket
FROM rentals r
ORDER BY rental_id;

-- -----------------------------------------------------------------------------
-- 18) Выручка по пользователям с разбивкой по типам оплаты (SUM CASE)
SELECT u.id, u.email,
       SUM(pl.price) FILTER (WHERE pl.pay_type = 'card')    AS pay_card,
       SUM(pl.price) FILTER (WHERE pl.pay_type = 'cashback') AS pay_cashback,
       SUM(pl.price) AS total
FROM users u
LEFT JOIN payment_logs pl ON pl.user_id = u.id
GROUP BY u.id, u.email
ORDER BY total DESC NULLS LAST;

-- -----------------------------------------------------------------------------
-- 19) Водительские удостоверения и фото документа (JOIN chain)
--    Функция ЛР1: модерация документов
SELECT dl.driver_id, dl.license_number, dl.status,
       p.url AS document_photo_url,
       u.email AS uploader_email
FROM driver_licenses dl
JOIN photos p ON p.id = dl.document_photo_id
JOIN users u  ON u.id = p.uploaded_by
ORDER BY dl.driver_id;

-- -----------------------------------------------------------------------------
-- 20) Ближайшая доступная машина к точке (гео) — [СЛОЖНЫЙ]
--    Используем SRID 4326; замените на свои координаты долготы/широты
SELECT c.id, c.plate_number, c.model,
       ST_DistanceSphere(c.position, ST_SetSRID(ST_MakePoint(37.6173,55.7558),4326)) AS dist_m
FROM cars c
WHERE c.status = 'available' AND c.position IS NOT NULL
ORDER BY dist_m
LIMIT 1;

-- -----------------------------------------------------------------------------
-- 21) Перцентили/квартильные группы по цене аренды в разрезе модели (NTILE) — [СЛОЖНЫЙ]
SELECT c.model, r.id AS rental_id, r.price,
       NTILE(4) OVER (PARTITION BY c.model ORDER BY r.price) AS price_quartile
FROM rentals r
JOIN cars c ON c.id = r.car_id
ORDER BY c.model, r.price;

-- -----------------------------------------------------------------------------
-- 22) Сводка выручки: по пользователю, по модели и общий итог (GROUPING SETS) — [СЛОЖНЫЙ]
WITH fees AS (
  SELECT r.id AS rental_id, r.user_id, r.car_id, COALESCE(r.price,0) AS price
  FROM rentals r
)
SELECT
  u.email,
  c.model,
  SUM(f.price) AS revenue
FROM fees f
LEFT JOIN users u ON u.id = f.user_id
LEFT JOIN cars  c ON c.id = f.car_id
GROUP BY GROUPING SETS ((u.email, c.model), (u.email), (c.model), ())
ORDER BY (u.email IS NULL), u.email, (c.model IS NULL), c.model;

-- -----------------------------------------------------------------------------
-- 23) Последняя сессия пользователя (DISTINCT ON) — [СЛОЖНЫЙ]
SELECT DISTINCT ON (s.user_id)
       s.user_id, s.id AS session_id, s.ip, s.created_at
FROM sessions s
ORDER BY s.user_id, s.created_at DESC;

-- -----------------------------------------------------------------------------
-- 24) Минуты аренды по пользователю (коррелированный подзапрос)
SELECT u.id, u.email,
       (
         SELECT COALESCE(SUM(EXTRACT(EPOCH FROM (COALESCE(r.ended_at, NOW()) - r.started_at)))/60.0,0)
         FROM rentals r WHERE r.user_id = u.id
       ) AS minutes_total
FROM users u
ORDER BY minutes_total DESC;

-- -----------------------------------------------------------------------------
-- 25) Счётчики фотографий по типам (CASE в агрегатах)
SELECT u.id, u.email,
       SUM(CASE WHEN p.object_type = 'driver'   THEN 1 ELSE 0 END) AS driver_photos,
       SUM(CASE WHEN p.object_type = 'document' THEN 1 ELSE 0 END) AS document_photos,
       SUM(CASE WHEN p.object_type = 'car'      THEN 1 ELSE 0 END) AS car_photos
FROM users u
LEFT JOIN photos p ON p.uploaded_by = u.id
GROUP BY u.id, u.email
ORDER BY u.id;

-- -----------------------------------------------------------------------------
-- 26) Недоплаты: аренды, где сумма платежей < price (LEFT JOIN + агрегат) — [СЛОЖНЫЙ]
--    Функция ЛР1: сверка оплат
WITH paid AS (
  SELECT rental_id, SUM(price) AS paid
  FROM payment_logs
  GROUP BY rental_id
)
SELECT r.id AS rental_id, r.user_id, r.price, COALESCE(p.paid,0) AS paid,
       (r.price - COALESCE(p.paid,0)) AS underpaid
FROM rentals r
LEFT JOIN paid p ON p.rental_id = r.id
WHERE COALESCE(p.paid,0) < r.price
ORDER BY underpaid DESC;

-- -----------------------------------------------------------------------------
-- 27) Скользящее среднее по дневным арендам (7-дневное окно) — [СЛОЖНЫЙ]
WITH days AS (
  SELECT generate_series::date AS d
  FROM generate_series((CURRENT_DATE - INTERVAL '27 days')::date, CURRENT_DATE, INTERVAL '1 day')
),
per_day AS (
  SELECT d.d AS day, COUNT(r.id) AS rentals_cnt
  FROM days d
  LEFT JOIN rentals r ON r.started_at::date = d.d
  GROUP BY d.d
)
SELECT day,
       rentals_cnt,
       AVG(rentals_cnt) OVER (ORDER BY day ROWS BETWEEN 6 PRECEDING AND CURRENT ROW) AS mov_avg_7
FROM per_day
ORDER BY day;

-- -----------------------------------------------------------------------------
-- 28) Аккаунты, ожидающие верификации прав и имеющие загруженные документы (EXISTS)
--    Функция ЛР1: очередь модерации
SELECT u.id, u.email
FROM users u
WHERE u.driver_id IS NOT NULL
  AND EXISTS (
      SELECT 1 FROM driver_licenses dl
      WHERE dl.driver_id = u.driver_id AND dl.status = 'pending'
  )
  AND EXISTS (
      SELECT 1 FROM photos p
      WHERE p.object_type IN ('driver','document') AND p.user_id = u.id
  )
ORDER BY u.id;

-- -----------------------------------------------------------------------------
-- 29) Распределение пользователей по ролям с нулями (RIGHT JOIN + COALESCE)
SELECT r.name AS role_name, COALESCE(cnt.cnt,0) AS users_cnt
FROM roles r
LEFT JOIN (
  SELECT role_id, COUNT(*) AS cnt
  FROM users
  GROUP BY role_id
) cnt ON cnt.role_id = r.id
ORDER BY role_name;

-- -----------------------------------------------------------------------------
-- 30) Активность по IP: число сессий на пользователя и последний визит (GROUP + MAX)
SELECT u.id, u.email,
       COUNT(s.id) AS sessions_cnt,
       MAX(s.created_at) AS last_seen
FROM users u
LEFT JOIN sessions s ON s.user_id = u.id
GROUP BY u.id, u.email
ORDER BY last_seen DESC NULLS LAST;

