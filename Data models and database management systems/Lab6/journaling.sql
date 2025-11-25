\c carsharing_db;
SET TIME ZONE 'Europe/Minsk';

-- =============================================================================
-- СИСТЕМА ЖУРНАЛИРОВАНИЯ ДЕЙСТВИЙ ПОЛЬЗОВАТЕЛЕЙ
-- =============================================================================

-- Таблица для журналирования действий
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
            'ip', NEW.ip::TEXT
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
-- ЗАПРОСЫ ДЛЯ АНАЛИТИКИ ЖУРНАЛА ДЕЙСТВИЙ
-- =============================================================================

-- 1. Статистика действий по типам
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

-- 3. Ежедневная активность
SELECT 
    DATE(created_at) as activity_date,
    COUNT(*) as total_actions,
    COUNT(DISTINCT actor_user_id) as active_users
FROM action_logs
WHERE created_at >= CURRENT_DATE - INTERVAL '7 days'
GROUP BY DATE(created_at)
ORDER BY activity_date DESC;

-- 4. Аудит изменений пользователей
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

