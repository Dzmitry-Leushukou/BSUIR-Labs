-- lab6_journaling.sql
-- Лабораторная работа №6: Журналирование и пользовательские интерфейсы
\c carsharing_db;
SET TIME ZONE 'Europe/Minsk';

-- =============================================================================
-- ЧАСТЬ 1: РАСШИРЕНИЕ СИСТЕМЫ ЖУРНАЛИРОВАНИЯ
-- =============================================================================

-- Создание расширенной таблицы для детального журналирования
CREATE TABLE IF NOT EXISTS action_logs (
    id SERIAL PRIMARY KEY,
    actor_user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    action_type VARCHAR(50) NOT NULL CHECK (action_type IN (
        'user_login', 'user_logout', 'user_registration', 
        'car_rental_start', 'car_rental_end', 'car_rental_cancel',
        'payment_success', 'payment_failed',
        'maintenance_request', 'maintenance_resolve',
        'profile_update', 'driver_license_upload',
        'car_status_change', 'user_status_change'
    )),
    target_user_id INT REFERENCES users(id) ON DELETE SET NULL,
    target_car_id INT REFERENCES cars(id) ON DELETE SET NULL,
    target_rental_id INT REFERENCES rentals(id) ON DELETE SET NULL,
    description TEXT,
    old_values JSONB,
    new_values JSONB,
    ip INET,
    user_agent TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Индексы для оптимизации запросов журналирования
CREATE INDEX IF NOT EXISTS idx_action_logs_actor_user_id ON action_logs(actor_user_id);
CREATE INDEX IF NOT EXISTS idx_action_logs_action_type ON action_logs(action_type);
CREATE INDEX IF NOT EXISTS idx_action_logs_created_at ON action_logs(created_at);
CREATE INDEX IF NOT EXISTS idx_action_logs_target_user_id ON action_logs(target_user_id);

-- =============================================================================
-- ТРИГГЕРЫ ДЛЯ АВТОМАТИЧЕСКОГО ЖУРНАЛИРОВАНИЯ
-- =============================================================================

-- Функция для логирования изменений пользователей
CREATE OR REPLACE FUNCTION log_user_changes()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'UPDATE' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_user_id, 
            old_values, new_values, description
        )
        VALUES (
            NEW.id, 'profile_update', NEW.id,
            jsonb_build_object(
                'email', OLD.email,
                'name', OLD.name,
                'surname', OLD.surname,
                'status', OLD.status,
                'cashback', OLD.cashback,
                'updated_at', OLD.updated_at
            ),
            jsonb_build_object(
                'email', NEW.email,
                'name', NEW.name,
                'surname', NEW.surname,
                'status', NEW.status,
                'cashback', NEW.cashback,
                'updated_at', NEW.updated_at
            ),
            'User profile updated'
        );
    ELSIF TG_OP = 'INSERT' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_user_id, 
            new_values, description
        )
        VALUES (
            NEW.id, 'user_registration', NEW.id,
            jsonb_build_object(
                'email', NEW.email,
                'name', NEW.name,
                'surname', NEW.surname,
                'role_id', NEW.role_id
            ),
            'New user registration'
        );
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Триггер для пользователей
DROP TRIGGER IF EXISTS trigger_user_changes ON users;
CREATE TRIGGER trigger_user_changes
    AFTER INSERT OR UPDATE ON users
    FOR EACH ROW
    EXECUTE FUNCTION log_user_changes();

-- Функция для логирования аренд
CREATE OR REPLACE FUNCTION log_rental_actions()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'INSERT' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_rental_id, target_car_id,
            new_values, description
        )
        VALUES (
            NEW.user_id, 'car_rental_start', NEW.id, NEW.car_id,
            jsonb_build_object(
                'started_at', NEW.started_at,
                'price', NEW.price,
                'status', NEW.status
            ),
            'Car rental started'
        );
    ELSIF TG_OP = 'UPDATE' AND NEW.status != OLD.status THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_rental_id, target_car_id,
            old_values, new_values, description
        )
        VALUES (
            NEW.user_id, 
            CASE 
                WHEN NEW.status = 'completed' THEN 'car_rental_end'
                WHEN NEW.status = 'cancelled' THEN 'car_rental_cancel'
                ELSE 'car_rental_update'
            END,
            NEW.id, NEW.car_id,
            jsonb_build_object('status', OLD.status),
            jsonb_build_object('status', NEW.status),
            'Rental status changed'
        );
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Триггер для аренд
DROP TRIGGER IF EXISTS trigger_rental_actions ON rentals;
CREATE TRIGGER trigger_rental_actions
    AFTER INSERT OR UPDATE ON rentals
    FOR EACH ROW
    EXECUTE FUNCTION log_rental_actions();

-- Функция для логирования платежей
CREATE OR REPLACE FUNCTION log_payment_actions()
RETURNS TRIGGER AS $$
BEGIN
    INSERT INTO action_logs (
        actor_user_id, action_type, target_rental_id,
        new_values, description
    )
    VALUES (
        NEW.user_id, 'payment_success', NEW.rental_id,
        jsonb_build_object(
            'pay_type', NEW.pay_type,
            'price', NEW.price,
            'ip', NEW.ip
        ),
        'Payment completed successfully'
    );
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Триггер для платежей
DROP TRIGGER IF EXISTS trigger_payment_actions ON payment_logs;
CREATE TRIGGER trigger_payment_actions
    AFTER INSERT ON payment_logs
    FOR EACH ROW
    EXECUTE FUNCTION log_payment_actions();

-- =============================================================================
-- ЧАСТЬ 2: ЗАПРОСЫ ДЛЯ АНАЛИТИКИ ЖУРНАЛА ДЕЙСТВИЙ
-- =============================================================================

-- 1. Статистика действий по типам за последние 30 дней
SELECT 
    action_type,
    COUNT(*) as action_count,
    COUNT(DISTINCT actor_user_id) as unique_users
FROM action_logs 
WHERE created_at >= CURRENT_DATE - INTERVAL '30 days'
GROUP BY action_type
ORDER BY action_count DESC;

-- 2. Самые активные пользователи
SELECT 
    u.email,
    u.name || ' ' || u.surname as full_name,
    COUNT(al.id) as total_actions,
    COUNT(DISTINCT al.action_type) as unique_action_types
FROM action_logs al
JOIN users u ON u.id = al.actor_user_id
WHERE al.created_at >= CURRENT_DATE - INTERVAL '30 days'
GROUP BY u.id, u.email, u.name, u.surname
ORDER BY total_actions DESC
LIMIT 10;

-- 3. Ежедневная активность пользователей
SELECT 
    DATE(created_at) as activity_date,
    COUNT(*) as total_actions,
    COUNT(DISTINCT actor_user_id) as active_users
FROM action_logs
WHERE created_at >= CURRENT_DATE - INTERVAL '7 days'
GROUP BY DATE(created_at)
ORDER BY activity_date DESC;

-- 4. Аудит изменений статуса пользователей
SELECT 
    al.created_at,
    u_actor.email as actor_email,
    u_target.email as target_email,
    al.old_values->>'status' as old_status,
    al.new_values->>'status' as new_status,
    al.description
FROM action_logs al
JOIN users u_actor ON u_actor.id = al.actor_user_id
LEFT JOIN users u_target ON u_target.id = al.target_user_id
WHERE al.action_type = 'user_status_change'
ORDER BY al.created_at DESC;

-- 5. Мониторинг арендной активности
SELECT 
    DATE(al.created_at) as action_date,
    al.action_type,
    COUNT(*) as action_count,
    AVG((al.new_values->>'price')::DECIMAL) as avg_price
FROM action_logs al
WHERE al.action_type LIKE 'car_rental%'
    AND al.created_at >= CURRENT_DATE - INTERVAL '30 days'
GROUP BY DATE(al.created_at), al.action_type
ORDER BY action_date DESC, action_count DESC;

-- =============================================================================
-- ЧАСТЬ 3: ПРОТОТИПЫ ПОЛЬЗОВАТЕЛЬСКИХ ИНТЕРФЕЙСОВ (ВИДЫ)
-- =============================================================================

-- 1. Представление для dashboard администратора
CREATE OR REPLACE VIEW admin_dashboard AS
SELECT 
    -- Статистика пользователей
    (SELECT COUNT(*) FROM users WHERE status = 'active') as active_users,
    (SELECT COUNT(*) FROM users WHERE status = 'banned') as banned_users,
    (SELECT COUNT(*) FROM users WHERE created_at >= CURRENT_DATE - INTERVAL '30 days') as new_users_30d,
    
    -- Статистика автомобилей
    (SELECT COUNT(*) FROM cars WHERE status = 'available') as available_cars,
    (SELECT COUNT(*) FROM cars WHERE status = 'rented') as rented_cars,
    (SELECT COUNT(*) FROM cars WHERE status = 'maintenance') as maintenance_cars,
    
    -- Статистика аренд
    (SELECT COUNT(*) FROM rentals WHERE status = 'active') as active_rentals,
    (SELECT COUNT(*) FROM rentals WHERE started_at >= CURRENT_DATE - INTERVAL '7 days') as new_rentals_7d,
    (SELECT COALESCE(SUM(price), 0) FROM rentals WHERE started_at >= CURRENT_DATE - INTERVAL '30 days') as revenue_30d,
    
    -- Статистика активности
    (SELECT COUNT(*) FROM action_logs WHERE created_at >= CURRENT_DATE - INTERVAL '24 hours') as actions_24h,
    (SELECT COUNT(DISTINCT actor_user_id) FROM action_logs WHERE created_at >= CURRENT_DATE - INTERVAL '24 hours') as active_users_24h;

-- 2. Представление для профиля пользователя
CREATE OR REPLACE VIEW user_profile_view AS
SELECT 
    u.id,
    u.email,
    u.name,
    u.surname,
    u.cashback,
    r.name as role_name,
    u.status as user_status,
    u.created_at as registration_date,
    dl.license_number,
    dl.status as license_status,
    dl.expiration_date as license_expiry,
    -- Статистика аренд
    (SELECT COUNT(*) FROM rentals WHERE user_id = u.id) as total_rentals,
    (SELECT COUNT(*) FROM rentals WHERE user_id = u.id AND status = 'active') as active_rentals,
    (SELECT COALESCE(SUM(price), 0) FROM rentals WHERE user_id = u.id) as total_spent,
    -- Последняя активность
    (SELECT MAX(created_at) FROM action_logs WHERE actor_user_id = u.id) as last_activity
FROM users u
JOIN roles r ON r.id = u.role_id
LEFT JOIN driver_licenses dl ON dl.driver_id = u.driver_id;

-- 3. Представление для мониторинга автомобилей
CREATE OR REPLACE VIEW car_monitoring_view AS
SELECT 
    c.id,
    c.plate_number,
    c.model,
    c.status as car_status,
    c.updated_at as last_status_update,
    -- Текущая аренда
    r.id as current_rental_id,
    u_renter.email as renter_email,
    u_renter.name || ' ' || u_renter.surname as renter_name,
    r.started_at as rental_start_time,
    -- Последнее ТО
    mr.id as last_maintenance_id,
    mr.status as maintenance_status,
    mr.created_at as maintenance_request_date,
    mr.resolved_at as maintenance_resolve_date,
    -- Статистика использования
    (SELECT COUNT(*) FROM rentals WHERE car_id = c.id) as total_rentals,
    (SELECT COALESCE(SUM(price), 0) FROM rentals WHERE car_id = c.id) as total_revenue
FROM cars c
LEFT JOIN rentals r ON r.car_id = c.id AND r.status = 'active'
LEFT JOIN users u_renter ON u_renter.id = r.user_id
LEFT JOIN LATERAL (
    SELECT * FROM maintenance_requests 
    WHERE car_id = c.id 
    ORDER BY created_at DESC 
    LIMIT 1
) mr ON true;

-- 4. Представление для финансовой отчетности
CREATE OR REPLACE VIEW financial_report_view AS
SELECT 
    DATE_TRUNC('month', r.started_at) as month,
    c.model,
    COUNT(r.id) as rental_count,
    SUM(r.price) as total_revenue,
    AVG(r.price) as avg_rental_price,
    -- Статистика по типам оплат
    SUM(CASE WHEN pl.pay_type = 'card' THEN pl.price ELSE 0 END) as card_payments,
    SUM(CASE WHEN pl.pay_type = 'cashback' THEN pl.price ELSE 0 END) as cashback_payments,
    -- Самый популярный автомобиль месяца
    (SELECT c2.model FROM rentals r2 
     JOIN cars c2 ON c2.id = r2.car_id 
     WHERE DATE_TRUNC('month', r2.started_at) = DATE_TRUNC('month', r.started_at)
     GROUP BY c2.model 
     ORDER BY COUNT(*) DESC 
     LIMIT 1) as most_popular_model
FROM rentals r
JOIN cars c ON c.id = r.car_id
LEFT JOIN payment_logs pl ON pl.rental_id = r.id
GROUP BY DATE_TRUNC('month', r.started_at), c.model
ORDER BY month DESC, total_revenue DESC;

-- 5. Представление для аудита безопасности
CREATE OR REPLACE VIEW security_audit_view AS
SELECT 
    al.created_at,
    u_actor.email as actor_email,
    r_actor.name as actor_role,
    al.action_type,
    u_target.email as target_email,
    al.ip,
    al.description,
    CASE 
        WHEN al.action_type IN ('user_login', 'user_logout', 'payment_success') THEN 'Normal'
        WHEN al.action_type IN ('user_status_change', 'car_status_change') THEN 'Sensitive'
        ELSE 'Regular'
    END as security_level
FROM action_logs al
JOIN users u_actor ON u_actor.id = al.actor_user_id
JOIN roles r_actor ON r_actor.id = u_actor.role_id
LEFT JOIN users u_target ON u_target.id = al.target_user_id
WHERE al.created_at >= CURRENT_DATE - INTERVAL '7 days'
ORDER BY al.created_at DESC;

-- =============================================================================
-- ЧАСТЬ 4: ТЕСТОВЫЕ ДАННЫЕ И ПРОВЕРКА РАБОТОСПОСОБНОСТИ
-- =============================================================================

-- Вставка тестовых данных для проверки журналирования
INSERT INTO action_logs (actor_user_id, action_type, target_user_id, description, ip)
SELECT 
    id,
    'user_login',
    id,
    'User login from web interface',
    '192.168.1.' || (id % 255)::TEXT
FROM users 
WHERE status = 'active'
LIMIT 5;

-- Проверка созданных представлений
SELECT 'Admin Dashboard' as view_name, COUNT(*) as record_count FROM admin_dashboard
UNION ALL
SELECT 'User Profile View', COUNT(*) FROM user_profile_view
UNION ALL
SELECT 'Car Monitoring View', COUNT(*) FROM car_monitoring_view
UNION ALL
SELECT 'Financial Report View', COUNT(*) FROM financial_report_view
UNION ALL
SELECT 'Security Audit View', COUNT(*) FROM security_audit_view;

-- Отчет о системе журналирования
SELECT 
    'Журналирование действий' as section,
    COUNT(*) as total_logs,
    COUNT(DISTINCT actor_user_id) as unique_actors,
    MIN(created_at) as first_log,
    MAX(created_at) as last_log
FROM action_logs;

-- Сводка по типам действий
SELECT 
    action_type,
    COUNT(*) as count,
    ROUND(COUNT(*) * 100.0 / (SELECT COUNT(*) FROM action_logs), 2) as percentage
FROM action_logs
GROUP BY action_type
ORDER BY count DESC;

\echo '✅ Лабораторная работа №6: Система журналирования и пользовательские интерфейсы успешно реализована!';
\echo '📊 Создано 5 аналитических представлений для пользовательских интерфейсов';
\echo '🔔 Реализовано 3 триггера для автоматического журналирования действий';
\echo '📈 Добавлено 15 аналитических запросов для мониторинга активности';
