\c carsharing_db;
SET TIME ZONE 'Europe/Minsk';

-- -----------------------------------------------------------------------------
-- 1) Просмотр каталога пользователей
SELECT u.id, u.email, u.name, u.surname, r.name AS role_name, u.cashback
FROM users u
JOIN roles r ON r.id = u.role_id
WHERE u.status = 'active'
  AND (u.cashback >= 100 OR u.created_at >= NOW() - INTERVAL '30 days')
ORDER BY u.cashback DESC, u.created_at DESC;

-- -----------------------------------------------------------------------------
-- 2) Доступные сейчас машины
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
-- 3) Текущие активные аренды с пользователем и авто
SELECT r.id AS rental_id, r.started_at, u.email, u.name || ' ' || u.surname AS user_fullname,
       c.plate_number, c.model
FROM rentals r
JOIN users   u ON u.id = r.user_id
JOIN cars    c ON c.id = r.car_id
WHERE r.status = 'active'
ORDER BY r.started_at DESC;

-- -----------------------------------------------------------------------------
-- 4) Кол-во аренд по модели авто
SELECT c.model, COUNT(*) AS rentals_cnt
FROM rentals r
JOIN cars c ON c.id = r.car_id
GROUP BY c.model
ORDER BY rentals_cnt DESC, c.model;

-- -----------------------------------------------------------------------------
-- 5) Модели с более чем 10 аренд
SELECT c.model, COUNT(*) AS rentals_cnt
FROM rentals r
JOIN cars c ON c.id = r.car_id
GROUP BY c.model
HAVING COUNT(*) > 10
ORDER BY rentals_cnt DESC;

-- -----------------------------------------------------------------------------
-- 6) Выручка по пользователям
SELECT u.id, u.email, 
       CASE WHEN rv.total_revenue IS NOT NULL THEN rv.total_revenue ELSE 0 END AS total_revenue
FROM users u
LEFT JOIN (
  SELECT user_id, SUM(price) AS total_revenue
  FROM payment_logs 
  GROUP BY user_id
) rv ON rv.user_id = u.id
ORDER BY total_revenue DESC, u.id;

-- -----------------------------------------------------------------------------
-- 7) История аренд с разницей между поездками
SELECT user_id, id AS rental_id, started_at,
       (SELECT started_at FROM rentals r2 
        WHERE r2.user_id = r1.user_id AND r2.started_at < r1.started_at 
        ORDER BY r2.started_at DESC LIMIT 1) AS prev_started_at
FROM rentals r1
ORDER BY user_id, started_at;

-- -----------------------------------------------------------------------------
-- 8) Каскадные платежи в рамках аренды
SELECT rental_id, id AS payment_id, price,
       (SELECT SUM(price) FROM payment_logs pl2 
        WHERE pl2.rental_id = pl1.rental_id AND pl2.id <= pl1.id) AS cum_paid
FROM payment_logs pl1
ORDER BY rental_id, id;

-- -----------------------------------------------------------------------------
-- 9) Уникальный список участников экосистемы (пользователи + заявители ТО)
SELECT email AS contact FROM users
UNION
SELECT u2.email AS contact
FROM maintenance_requests mr
JOIN users u2 ON u2.id = mr.reported_by
ORDER BY contact;

-- -----------------------------------------------------------------------------
-- 10) Пользователи с максимальным кэшбэком
SELECT id, email, cashback
FROM users
WHERE cashback = (SELECT MAX(cashback) FROM users)
ORDER BY id;

-- -----------------------------------------------------------------------------
-- 11) Пользователи без единой аренды 
SELECT u.id, u.email
FROM users u
WHERE NOT EXISTS (
  SELECT 1 FROM rentals r WHERE r.user_id = u.id
)
ORDER BY u.id;

-- -----------------------------------------------------------------------------
-- 12) Роли и пользователи — показать роли даже без пользователей
SELECT r.name AS role, u.id AS user_id, u.email
FROM users u
RIGHT JOIN roles r ON r.id = u.role_id
ORDER BY r.name, u.id;

-- -----------------------------------------------------------------------------
-- 13) Машины и последняя заявка на ТО 
SELECT c.id AS car_id, c.plate_number, 
       (SELECT MAX(created_at) FROM maintenance_requests WHERE car_id = c.id) AS last_created
FROM cars c
ORDER BY c.id;

-- -----------------------------------------------------------------------------
-- 14) Ежедневное число аренд за последние 7 дней
SELECT DATE(started_at) AS day, COUNT(*) AS rentals_cnt
FROM rentals 
WHERE started_at >= CURRENT_DATE - INTERVAL '6 days'
GROUP BY DATE(started_at)
ORDER BY day;

-- -----------------------------------------------------------------------------
-- 15) Для каждой машины — последний техосмотр/заявка
SELECT c.id AS car_id, c.plate_number,
       mr.description AS last_maintenance_description,
       mr.created_at AS last_maintenance_date
FROM cars c
LEFT JOIN maintenance_requests mr ON mr.car_id = c.id
WHERE mr.created_at = (
    SELECT MAX(created_at) 
    FROM maintenance_requests 
    WHERE car_id = c.id
) OR mr.created_at IS NULL
ORDER BY c.id;
-- -----------------------------------------------------------------------------
-- 16) Однофамильцы среди пользователей
SELECT u1.id AS user1_id, u2.id AS user2_id, u1.surname
FROM users u1
JOIN users u2 ON u2.surname = u1.surname AND u2.id > u1.id
ORDER BY u1.surname, u1.id, u2.id;

-- -----------------------------------------------------------------------------
-- 17) Классификация длительности аренды
SELECT id AS rental_id,
       CASE
         WHEN ended_at - started_at < INTERVAL '1 hour' THEN 'short'
         WHEN ended_at - started_at < INTERVAL '6 hours' THEN 'medium'
         ELSE 'long'
       END AS duration_bucket
FROM rentals 
WHERE ended_at IS NOT NULL
ORDER BY id;

-- -----------------------------------------------------------------------------
-- 18) Выручка по пользователям с разбивкой по типам оплаты
SELECT u.id, u.email,
       SUM(CASE WHEN pl.pay_type = 'card' THEN pl.price ELSE 0 END) AS pay_card,
       SUM(CASE WHEN pl.pay_type = 'cashback' THEN pl.price ELSE 0 END) AS pay_cashback,
       SUM(pl.price) AS total
FROM users u
LEFT JOIN payment_logs pl ON pl.user_id = u.id
GROUP BY u.id, u.email
ORDER BY total DESC;

-- -----------------------------------------------------------------------------
-- 19) Водительские удостоверения и фото документа
SELECT dl.driver_id, dl.license_number, dl.status,
       p.url AS document_photo_url,
       u.email AS uploader_email
FROM driver_licenses dl
JOIN photos p ON p.id = dl.document_photo_id
JOIN users u  ON u.id = p.uploaded_by
ORDER BY dl.driver_id;

-- -----------------------------------------------------------------------------
-- 20) Первая доступная машина
SELECT c.id, c.plate_number, c.model
FROM cars c
WHERE c.status = 'available'
  AND NOT EXISTS (
      SELECT 1 
      FROM rentals r 
      WHERE r.car_id = c.id AND r.status = 'active'
  )
ORDER BY c.id
LIMIT 1;

-- -----------------------------------------------------------------------------
-- 21) Группы по цене аренды в разрезе модели 
SELECT model, rental_id, price,
       CASE 
         WHEN price_percent <= 25 THEN 1
         WHEN price_percent <= 50 THEN 2  
         WHEN price_percent <= 75 THEN 3
         ELSE 4
       END AS price_quartile
FROM (
  SELECT c.model, r.id AS rental_id, r.price,
         (SELECT COUNT(*) FROM rentals r2 JOIN cars c2 ON c2.id = r2.car_id 
          WHERE c2.model = c.model AND r2.price <= r.price) * 100.0 / 
         (SELECT COUNT(*) FROM rentals r3 JOIN cars c3 ON c3.id = r3.car_id WHERE c3.model = c.model) AS price_percent
  FROM rentals r
  JOIN cars c ON c.id = r.car_id
) t
ORDER BY model, price;

-- -----------------------------------------------------------------------------
-- 22) Сводка выручки
SELECT email, model, SUM(revenue) AS revenue
FROM (
  SELECT u.email, c.model, r.price AS revenue
  FROM rentals r
  JOIN users u ON u.id = r.user_id
  JOIN cars c ON c.id = r.car_id
  
  UNION ALL
  
  SELECT u.email, NULL AS model, r.price AS revenue
  FROM rentals r
  JOIN users u ON u.id = r.user_id
  
  UNION ALL
  
  SELECT NULL AS email, c.model, r.price AS revenue
  FROM rentals r
  JOIN cars c ON c.id = r.car_id
) t
GROUP BY email, model
ORDER BY email, model;

-- -----------------------------------------------------------------------------
-- 23) Последняя сессия пользователя
SELECT user_id, id AS session_id, ip, created_at
FROM sessions s1
WHERE created_at = (
    SELECT MAX(created_at) 
    FROM sessions 
    WHERE user_id = s1.user_id
)
ORDER BY user_id;

-- -----------------------------------------------------------------------------
-- 24) Минуты аренды по пользователю
SELECT u.id, u.email,
       (SELECT SUM(
          CASE 
            WHEN ended_at IS NOT NULL THEN 
              (EXTRACT(DAY FROM (ended_at - started_at)) * 24 * 60 +
               EXTRACT(HOUR FROM (ended_at - started_at)) * 60 +
               EXTRACT(MINUTE FROM (ended_at - started_at)))
            ELSE 0
          END
        ) FROM rentals r WHERE r.user_id = u.id) AS minutes_total
FROM users u
ORDER BY minutes_total DESC NULLS LAST;

-- -----------------------------------------------------------------------------
-- 25) Счётчики фотографий по типам
SELECT u.id, u.email,
       COUNT(CASE WHEN p.object_type = 'driver' THEN 1 END) AS driver_photos,
       COUNT(CASE WHEN p.object_type = 'document' THEN 1 END) AS document_photos, 
       COUNT(CASE WHEN p.object_type = 'car' THEN 1 END) AS car_photos
FROM users u
LEFT JOIN photos p ON p.uploaded_by = u.id
GROUP BY u.id, u.email
ORDER BY u.id;

-- -----------------------------------------------------------------------------
-- 26) Недоплаты: аренды, где сумма платежей < price
SELECT r.id AS rental_id, r.user_id, r.price, 
       CASE WHEN p.paid IS NOT NULL THEN p.paid ELSE 0 END AS paid,
       r.price - CASE WHEN p.paid IS NOT NULL THEN p.paid ELSE 0 END AS underpaid
FROM rentals r
LEFT JOIN (
  SELECT rental_id, SUM(price) AS paid
  FROM payment_logs
  GROUP BY rental_id
) p ON p.rental_id = r.id
WHERE CASE WHEN p.paid IS NOT NULL THEN p.paid ELSE 0 END < r.price
ORDER BY underpaid DESC;

-- -----------------------------------------------------------------------------
-- 27) Ежедневное число аренд за последние 28 дней
SELECT DATE(started_at) AS day, COUNT(*) AS rentals_cnt
FROM rentals 
WHERE started_at >= CURRENT_DATE - INTERVAL '27 days'
GROUP BY DATE(started_at)
ORDER BY day;
-- -----------------------------------------------------------------------------
-- 28) Аккаунты, ожидающие верификации прав
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
-- 29) Распределение пользователей по ролям
SELECT r.name AS role_name, 
       CASE WHEN cnt.cnt IS NOT NULL THEN cnt.cnt ELSE 0 END AS users_cnt
FROM roles r
LEFT JOIN (
  SELECT role_id, COUNT(*) AS cnt
  FROM users
  GROUP BY role_id
) cnt ON cnt.role_id = r.id
ORDER BY role_name;

-- -----------------------------------------------------------------------------
-- 30) Активность по IP
SELECT u.id, u.email,
       COUNT(s.id) AS sessions_cnt,
       MAX(s.created_at) AS last_seen
FROM users u
LEFT JOIN sessions s ON s.user_id = u.id
GROUP BY u.id, u.email
ORDER BY last_seen DESC;

-- -----------------------------------------------------------------------------
-- 31) Копирование активных пользователей в архивную таблицу
INSERT INTO logs (actor_user_id, action_type, target_id, created_at, ip)
SELECT 
    u.id AS actor_user_id,
    'user_export' AS action_type, 
    u.id AS target_id,
    NOW() AS created_at,
    '127.0.0.1' AS ip
FROM users u
WHERE u.status = 'active'
  AND u.created_at >= NOW() - INTERVAL '30 days';

-- -----------------------------------------------------------------------------
-- 32) Анализ производительности запроса активных аренд
EXPLAIN 
SELECT r.id AS rental_id, r.started_at, u.email, u.name || ' ' || u.surname AS user_fullname,
       c.plate_number, c.model
FROM rentals r
JOIN users   u ON u.id = r.user_id
JOIN cars    c ON c.id = r.car_id
WHERE r.status = 'active'
ORDER BY r.started_at DESC;
-- -----------------------------------------------------------------------------
-- 33) Рейтинг пользователей по выручке с ранжированием
SELECT 
    u.id,
    u.email,	
    u.name || ' ' || u.surname as full_name,
    COALESCE(SUM(r.price), 0) as total_revenue,
    RANK() OVER (ORDER BY COALESCE(SUM(r.price), 0) DESC) as revenue_rank,
    LAG(u.email) OVER (ORDER BY COALESCE(SUM(r.price), 0) DESC) as prev_user,
    ROUND(
        COALESCE(SUM(r.price), 0) * 100.0 / NULLIF(SUM(COALESCE(SUM(r.price), 0)) OVER(), 0), 
        2
    ) as revenue_percentage
FROM users u
LEFT JOIN rentals r ON r.user_id = u.id AND r.status = 'completed'
GROUP BY u.id, u.email, u.name, u.surname
ORDER BY total_revenue DESC;

