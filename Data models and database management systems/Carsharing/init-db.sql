-- This script runs during PostgreSQL container initialization
-- The database specified by POSTGRES_DB in docker-compose.yml is already created and selected

-- Create PostGIS extension
CREATE EXTENSION IF NOT EXISTS postgis;

-- Create roles table
CREATE TABLE IF NOT EXISTS roles (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL CHECK (name IN ('admin','user')),
    description TEXT
);

-- Create users table
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    email VARCHAR(255) UNIQUE NOT NULL,
    hashed_password VARCHAR(255) NOT NULL,
    name VARCHAR(50) NOT NULL,
    surname VARCHAR(50) NOT NULL,
    cashback DECIMAL(12,2) DEFAULT 0 CHECK (cashback >= 0),
    role_id INT NOT NULL REFERENCES roles(id) ON DELETE RESTRICT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(10) DEFAULT 'active' CHECK (status IN ('active','banned'))
);

-- Create cars table
CREATE TABLE IF NOT EXISTS cars (
    id SERIAL PRIMARY KEY,
    vin VARCHAR(17) UNIQUE NOT NULL,
    plate_number VARCHAR(12) UNIQUE NOT NULL,
    model VARCHAR(100) NOT NULL,
    status VARCHAR(20) DEFAULT 'available' CHECK (status IN ('available','rented','maintenance','pending_completion')),
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    position geometry(Point, 4326),
    main_photo_id INT,
    CONSTRAINT plate_by_format_chk CHECK (plate_number ~ '^[0-9]{4} [A-Z]{2}-[0-8]$')
);

-- Create photos table
CREATE TABLE IF NOT EXISTS photos (
    id SERIAL PRIMARY KEY,
    object_type VARCHAR(50) NOT NULL CHECK (object_type IN ('driver','car','document')),
    user_id INT,
    car_id INT,
    file_data BYTEA NOT NULL,
    filename VARCHAR(255) NOT NULL,
    content_type VARCHAR(100) NOT NULL,
    file_size INT NOT NULL,
    uploaded_by INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT photos_object_match_chk CHECK (
        (object_type IN ('driver','document') AND user_id IS NOT NULL AND car_id IS NULL)
        OR
        (object_type = 'car' AND car_id IS NOT NULL AND user_id IS NULL)
    ),
    CONSTRAINT photos_object_user_fk FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT photos_object_car_fk  FOREIGN KEY (car_id)  REFERENCES cars(id)  ON DELETE CASCADE
);

-- Create driver_licenses table
CREATE TABLE IF NOT EXISTS driver_licenses (
    driver_id INT PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    license_number VARCHAR(40) UNIQUE NOT NULL,
    issued_by VARCHAR(255) NOT NULL,
    expiration_date DATE NOT NULL CHECK (expiration_date > CURRENT_DATE),
    document_photo_id INT NOT NULL REFERENCES photos(id) ON DELETE CASCADE,
    document_photo_back_id INT REFERENCES photos(id) ON DELETE CASCADE,
    status VARCHAR(10) DEFAULT 'pending' CHECK (status IN ('pending','approved','rejected'))
);

-- Add driver_id column to users table if it doesn't exist
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM information_schema.columns WHERE table_name = 'users' AND column_name = 'driver_id') THEN
        ALTER TABLE users ADD COLUMN driver_id INT UNIQUE REFERENCES driver_licenses(driver_id) ON DELETE SET NULL;
    END IF;
END
$$;

-- Add document_photo_back_id column if it doesn't exist
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM information_schema.columns
                   WHERE table_name = 'driver_licenses'
                   AND column_name = 'document_photo_back_id') THEN
        ALTER TABLE driver_licenses
        ADD COLUMN document_photo_back_id INT
        REFERENCES photos(id) ON DELETE CASCADE;
    END IF;
END
$$;

-- Create sessions table
CREATE TABLE IF NOT EXISTS sessions (
    id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create car_states table
CREATE TABLE IF NOT EXISTS car_states (
    id SERIAL PRIMARY KEY,
    car_id INT NOT NULL REFERENCES cars(id) ON DELETE CASCADE,
    checked_by INT REFERENCES users(id) ON DELETE SET NULL,
    checked_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    verified BOOLEAN DEFAULT FALSE,
    comment TEXT
);

-- Create rentals table
CREATE TABLE IF NOT EXISTS rentals (
    id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    car_id INT NOT NULL REFERENCES cars(id) ON DELETE CASCADE,
    started_at TIMESTAMP WITH TIME ZONE NOT NULL,
    ended_at TIMESTAMP WITH TIME ZONE CHECK (ended_at > started_at OR ended_at IS NULL),
    price DECIMAL(12,2) NOT NULL CHECK (price >= 0),
    status VARCHAR(20) DEFAULT 'active' CHECK (status IN ('active','completed','cancelled','pending_completion'))
);

-- Create maintenance_requests table
CREATE TABLE IF NOT EXISTS maintenance_requests (
    id SERIAL PRIMARY KEY,
    car_id INT NOT NULL REFERENCES cars(id) ON DELETE CASCADE,
    reported_by INT REFERENCES users(id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMP CHECK (resolved_at >= created_at OR resolved_at IS NULL),
    status VARCHAR(20) DEFAULT 'open' CHECK (status IN ('open','resolved')),
    description TEXT NOT NULL
);

-- Create trip_completions table
CREATE TABLE IF NOT EXISTS trip_completions (
    id SERIAL PRIMARY KEY,
    rental_id INT NOT NULL REFERENCES rentals(id) ON DELETE CASCADE,
    admin_approved BOOLEAN,
    admin_comment TEXT,
    admin_reviewed_by INT REFERENCES users(id) ON DELETE SET NULL,
    admin_reviewed_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create trip_completion_photos table to support multiple photos per trip completion
CREATE TABLE IF NOT EXISTS trip_completion_photos (
    id SERIAL PRIMARY KEY,
    trip_completion_id INT NOT NULL REFERENCES trip_completions(id) ON DELETE CASCADE,
    photo_id INT NOT NULL REFERENCES photos(id) ON DELETE CASCADE,
    is_primary BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(trip_completion_id, photo_id)
);

-- Create payment_logs table
CREATE TABLE IF NOT EXISTS payment_logs (
    id SERIAL PRIMARY KEY,
    rental_id INT NOT NULL REFERENCES rentals(id) ON DELETE CASCADE,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    pay_type VARCHAR(30) NOT NULL CHECK (pay_type IN ('card','cashback')),
    card_number VARCHAR(20) CHECK (card_number ~ '^[0-9]{16}$' OR card_number IS NULL), -- 16-digit card number
    price DECIMAL(10,2) NOT NULL CHECK (price >= 0)
);

-- Create logs table
CREATE TABLE IF NOT EXISTS logs (
    id SERIAL PRIMARY KEY,
    actor_user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    action_type VARCHAR(255) NOT NULL,
    target_id INT REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Set timezone
SET TIME ZONE 'UTC';
-- Create function to get current timestamp in UTC+3
CREATE OR REPLACE FUNCTION now_utc3()
RETURNS TIMESTAMP WITH TIME ZONE AS $$
BEGIN
 RETURN (NOW() AT TIME ZONE 'UTC') + INTERVAL '3 hours';
END;
$$ LANGUAGE plpgsql;

-- Update tables to use timezone-aware timestamps and default to UTC+3
-- Users table
ALTER TABLE users ALTER COLUMN created_at SET DEFAULT now_utc3();
ALTER TABLE users ALTER COLUMN updated_at SET DEFAULT now_utc3();

-- Cars table
ALTER TABLE cars ALTER COLUMN updated_at SET DEFAULT now_utc3();

-- Photos table
ALTER TABLE photos ALTER COLUMN uploaded_at SET DEFAULT now_utc3();

-- Sessions table
ALTER TABLE sessions ALTER COLUMN created_at SET DEFAULT now_utc3();
ALTER TABLE sessions ALTER COLUMN updated_at SET DEFAULT now_utc3();

-- Car states table
ALTER TABLE car_states ALTER COLUMN checked_at SET DEFAULT now_utc3();

-- Maintenance requests table
ALTER TABLE maintenance_requests ALTER COLUMN created_at SET DEFAULT now_utc3();
ALTER TABLE maintenance_requests ALTER COLUMN resolved_at TYPE TIMESTAMP WITH TIME ZONE USING resolved_at AT TIME ZONE 'UTC';

-- Trip completions table
ALTER TABLE trip_completions ALTER COLUMN admin_reviewed_at TYPE TIMESTAMP WITH TIME ZONE USING admin_reviewed_at AT TIME ZONE 'UTC';
ALTER TABLE trip_completions ALTER COLUMN created_at SET DEFAULT now_utc3();

-- Create action_logs table for logging system
CREATE TABLE IF NOT EXISTS action_logs (
    id SERIAL PRIMARY KEY,
    actor_user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    action_type VARCHAR(50) NOT NULL CHECK (action_type IN (
        'user_login', 'user_logout', 'user_registration',
        'car_rental_start', 'car_rental_end', 'car_rental_cancel', 'car_rental_pending_completion',
        'payment_success', 'payment_failed',
        'maintenance_request', 'maintenance_resolve',
        'profile_update', 'driver_license_upload',
        'car_status_change', 'user_status_change',
        'car_create', 'car_update', 'car_delete',
        'driver_license_create', 'driver_license_update', 'driver_license_delete',
        'maintenance_request_create', 'maintenance_request_update', 'maintenance_request_delete',
        'car_photo_upload', 'car_photo_delete',
        'trip_completion_create', 'trip_completion_update',
        'user_ban', 'user_unban'
    )),
    target_user_id INT REFERENCES users(id) ON DELETE SET NULL,
    target_car_id INT REFERENCES cars(id) ON DELETE SET NULL,
    target_rental_id INT REFERENCES rentals(id) ON DELETE SET NULL,
    description TEXT,
    old_values JSONB,
    new_values JSONB,
    user_agent TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Action logs table
ALTER TABLE action_logs ALTER COLUMN created_at SET DEFAULT now_utc3();

-- Logs table
ALTER TABLE logs ALTER COLUMN created_at SET DEFAULT now_utc3();

-- Create indexes for optimizing logging queries
CREATE INDEX IF NOT EXISTS idx_action_logs_actor_user_id ON action_logs(actor_user_id);
CREATE INDEX IF NOT EXISTS idx_action_logs_action_type ON action_logs(action_type);
CREATE INDEX IF NOT EXISTS idx_action_logs_created_at ON action_logs(created_at);
CREATE INDEX IF NOT EXISTS idx_action_logs_target_user_id ON action_logs(target_user_id);

-- Create function for logging user changes
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

-- Create trigger for users
DROP TRIGGER IF EXISTS trigger_user_changes ON users;
CREATE TRIGGER trigger_user_changes
    AFTER INSERT OR UPDATE ON users
    FOR EACH ROW
    EXECUTE FUNCTION log_user_changes();

-- Create function for logging rentals
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
                WHEN NEW.status = 'pending_completion' THEN 'car_rental_pending_completion'
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

-- Create trigger for rentals
DROP TRIGGER IF EXISTS trigger_rental_actions ON rentals;
CREATE TRIGGER trigger_rental_actions
    AFTER INSERT OR UPDATE ON rentals
    FOR EACH ROW
    EXECUTE FUNCTION log_rental_actions();

-- Create function for logging payments
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
            'price', NEW.price
        ),
        'Payment completed successfully'
    );
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create trigger for payments
DROP TRIGGER IF EXISTS trigger_payment_actions ON payment_logs;
CREATE TRIGGER trigger_payment_actions
    AFTER INSERT ON payment_logs
    FOR EACH ROW
    EXECUTE FUNCTION log_payment_actions();

-- Create function for logging car changes
CREATE OR REPLACE FUNCTION log_car_changes()
RETURNS TRIGGER AS $$
DECLARE
    current_user_id INTEGER := 1; -- Default user ID for system actions
BEGIN
    -- Try to get a valid user ID from the system, default to 1 if not available
    SELECT id INTO current_user_id FROM users ORDER BY id LIMIT 1;
    IF current_user_id IS NULL THEN
        current_user_id := 1; -- Default to user ID 1 if no users exist
    END IF;

    IF TG_OP = 'INSERT' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_car_id,
            new_values, description
        )
        VALUES (
            current_user_id, 'car_create', NEW.id,
            jsonb_build_object(
                'vin', NEW.vin,
                'plate_number', NEW.plate_number,
                'model', NEW.model,
                'status', NEW.status
            ),
            'Создание автомобиля'
        );
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_car_id,
            old_values, new_values, description
        )
        VALUES (
            current_user_id, 'car_update', NEW.id,
            jsonb_build_object(
                'vin', OLD.vin,
                'plate_number', OLD.plate_number,
                'model', OLD.model,
                'status', OLD.status,
                'updated_at', OLD.updated_at
            ),
            jsonb_build_object(
                'vin', NEW.vin,
                'plate_number', NEW.plate_number,
                'model', NEW.model,
                'status', NEW.status,
                'updated_at', NEW.updated_at
            ),
            'Обновление автомобиля'
        );
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_car_id,
            old_values, description
        )
        VALUES (
            current_user_id, 'car_delete', OLD.id,
            jsonb_build_object(
                'vin', OLD.vin,
                'plate_number', OLD.plate_number,
                'model', OLD.model,
                'status', OLD.status
            ),
            'Удаление автомобиля'
        );
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create trigger for cars
DROP TRIGGER IF EXISTS trigger_car_changes ON cars;
CREATE TRIGGER trigger_car_changes
    AFTER INSERT OR UPDATE OR DELETE ON cars
    FOR EACH ROW
    EXECUTE FUNCTION log_car_changes();

-- Create function for logging driver license changes
CREATE OR REPLACE FUNCTION log_driver_license_changes()
RETURNS TRIGGER AS $$
DECLARE
    current_user_id INTEGER := 1; -- Default user ID for system actions
BEGIN
    -- Try to get a valid user ID from the system, default to 1 if not available
    SELECT id INTO current_user_id FROM users ORDER BY id LIMIT 1;
    IF current_user_id IS NULL THEN
        current_user_id := 1; -- Default to user ID 1 if no users exist
    END IF;

    IF TG_OP = 'INSERT' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_user_id,
            new_values, description
        )
        VALUES (
            current_user_id, 'driver_license_create', NEW.driver_id,
            jsonb_build_object(
                'license_number', NEW.license_number,
                'issued_by', NEW.issued_by,
                'expiration_date', NEW.expiration_date,
                'status', NEW.status
            ),
            'Создание водительских прав'
        );
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_user_id,
            old_values, new_values, description
        )
        VALUES (
            current_user_id, 'driver_license_update', NEW.driver_id,
            jsonb_build_object(
                'license_number', OLD.license_number,
                'issued_by', OLD.issued_by,
                'expiration_date', OLD.expiration_date,
                'status', OLD.status
            ),
            jsonb_build_object(
                'license_number', NEW.license_number,
                'issued_by', NEW.issued_by,
                'expiration_date', NEW.expiration_date,
                'status', NEW.status
            ),
            'Обновление водительских прав'
        );
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_user_id,
            old_values, description
        )
        VALUES (
            current_user_id, 'driver_license_delete', OLD.driver_id,
            jsonb_build_object(
                'license_number', OLD.license_number,
                'issued_by', OLD.issued_by,
                'expiration_date', OLD.expiration_date,
                'status', OLD.status
            ),
            'Удаление водительских прав'
        );
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create trigger for driver_licenses
DROP TRIGGER IF EXISTS trigger_driver_license_changes ON driver_licenses;
CREATE TRIGGER trigger_driver_license_changes
    AFTER INSERT OR UPDATE OR DELETE ON driver_licenses
    FOR EACH ROW
    EXECUTE FUNCTION log_driver_license_changes();

-- Create function for logging maintenance request changes
CREATE OR REPLACE FUNCTION log_maintenance_request_changes()
RETURNS TRIGGER AS $$
DECLARE
    current_user_id INTEGER := 1; -- Default user ID for system actions
BEGIN
    -- Try to get a valid user ID from the system, default to 1 if not available
    SELECT id INTO current_user_id FROM users ORDER BY id LIMIT 1;
    IF current_user_id IS NULL THEN
        current_user_id := 1; -- Default to user ID 1 if no users exist
    END IF;

    IF TG_OP = 'INSERT' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_car_id,
            new_values, description
        )
        VALUES (
            COALESCE(NEW.reported_by, current_user_id), 'maintenance_request_create', NEW.car_id,
            jsonb_build_object(
                'description', NEW.description,
                'status', NEW.status
            ),
            'Создание запроса на обслуживание'
        );
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_car_id,
            old_values, new_values, description
        )
        VALUES (
            COALESCE(NEW.reported_by, current_user_id), 'maintenance_request_update', NEW.car_id,
            jsonb_build_object(
                'description', OLD.description,
                'status', OLD.status
            ),
            jsonb_build_object(
                'description', NEW.description,
                'status', NEW.status
            ),
            'Обновление запроса на обслуживание'
        );
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_car_id,
            old_values, description
        )
        VALUES (
            COALESCE(OLD.reported_by, current_user_id), 'maintenance_request_delete', OLD.car_id,
            jsonb_build_object(
                'description', OLD.description,
                'status', OLD.status
            ),
            'Удаление запроса на обслуживание'
        );
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create trigger for maintenance_requests
DROP TRIGGER IF EXISTS trigger_maintenance_request_changes ON maintenance_requests;
CREATE TRIGGER trigger_maintenance_request_changes
    AFTER INSERT OR UPDATE OR DELETE ON maintenance_requests
    FOR EACH ROW
    EXECUTE FUNCTION log_maintenance_request_changes();

-- Create function for logging photo changes
CREATE OR REPLACE FUNCTION log_photo_changes()
RETURNS TRIGGER AS $$
DECLARE
    current_user_id INTEGER := 1; -- Default user ID for system actions
BEGIN
    -- Try to get a valid user ID from the system, default to 1 if not available
    SELECT id INTO current_user_id FROM users ORDER BY id LIMIT 1;
    IF current_user_id IS NULL THEN
        current_user_id := 1; -- Default to user ID 1 if no users exist
    END IF;

    IF TG_OP = 'INSERT' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_user_id, target_car_id,
            new_values, description
        )
        VALUES (
            NEW.uploaded_by, 'car_photo_upload',
            CASE WHEN NEW.user_id IS NOT NULL THEN NEW.user_id ELSE NULL END,
            CASE WHEN NEW.car_id IS NOT NULL THEN NEW.car_id ELSE NULL END,
            jsonb_build_object(
                'filename', NEW.filename,
                'content_type', NEW.content_type,
                'object_type', NEW.object_type
            ),
            'Загрузка фото автомобиля'
        );
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_user_id, target_car_id,
            old_values, description
        )
        VALUES (
            current_user_id, 'car_photo_delete',
            CASE WHEN OLD.user_id IS NOT NULL THEN OLD.user_id ELSE NULL END,
            CASE WHEN OLD.car_id IS NOT NULL THEN OLD.car_id ELSE NULL END,
            jsonb_build_object(
                'filename', OLD.filename,
                'content_type', OLD.content_type,
                'object_type', OLD.object_type
            ),
            'Удаление фото автомобиля'
        );
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create trigger for photos
DROP TRIGGER IF EXISTS trigger_photo_changes ON photos;
CREATE TRIGGER trigger_photo_changes
    AFTER INSERT OR DELETE ON photos
    FOR EACH ROW
    EXECUTE FUNCTION log_photo_changes();


-- Create function for logging user status changes (ban/unban)
CREATE OR REPLACE FUNCTION log_user_status_changes()
RETURNS TRIGGER AS $$
DECLARE
    current_user_id INTEGER := 1; -- Default user ID for system actions
BEGIN
    -- Try to get a valid user ID from the system, default to 1 if not available
    SELECT id INTO current_user_id FROM users ORDER BY id LIMIT 1;
    IF current_user_id IS NULL THEN
        current_user_id := 1; -- Default to user ID 1 if no users exist
    END IF;

    IF TG_OP = 'UPDATE' AND OLD.status != NEW.status THEN
        INSERT INTO action_logs (
            actor_user_id, action_type, target_user_id,
            old_values, new_values, description
        )
        VALUES (
            COALESCE(NEW.id, current_user_id),
            CASE
                WHEN NEW.status = 'banned' AND OLD.status = 'active' THEN 'user_ban'
                WHEN NEW.status = 'active' AND OLD.status = 'banned' THEN 'user_unban'
                ELSE 'user_status_change'
            END,
            NEW.id,
            jsonb_build_object('status', OLD.status),
            jsonb_build_object('status', NEW.status),
            CASE
                WHEN NEW.status = 'banned' AND OLD.status = 'active' THEN 'Блокировка пользователя'
                WHEN NEW.status = 'active' AND OLD.status = 'banned' THEN 'Разблокировка пользователя'
                ELSE 'Изменение статуса пользователя'
            END
        );
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create trigger for user status changes
DROP TRIGGER IF EXISTS trigger_user_status_changes ON users;
CREATE TRIGGER trigger_user_status_changes
    AFTER UPDATE ON users
    FOR EACH ROW
    EXECUTE FUNCTION log_user_status_changes();

-- Create additional indexes for performance
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_users_role_id ON users(role_id);
CREATE INDEX IF NOT EXISTS idx_cars_vin ON cars(vin);
CREATE INDEX IF NOT EXISTS idx_cars_plate_number ON cars(plate_number);
CREATE INDEX IF NOT EXISTS idx_cars_status ON cars(status);
CREATE INDEX IF NOT EXISTS idx_rentals_user_id ON rentals(user_id);
CREATE INDEX IF NOT EXISTS idx_rentals_car_id ON rentals(car_id);
CREATE INDEX IF NOT EXISTS idx_rentals_status ON rentals(status);
CREATE INDEX IF NOT EXISTS idx_rentals_started_at ON rentals(started_at);
CREATE INDEX IF NOT EXISTS idx_rentals_ended_at ON rentals(ended_at);
CREATE INDEX IF NOT EXISTS idx_trip_completions_rental_id ON trip_completions(rental_id);
CREATE INDEX IF NOT EXISTS idx_trip_completions_admin_reviewed_by ON trip_completions(admin_reviewed_by);

-- Create indexes for trip_completion_photos table
CREATE INDEX IF NOT EXISTS idx_trip_completion_photos_trip_completion_id ON trip_completion_photos(trip_completion_id);
CREATE INDEX IF NOT EXISTS idx_trip_completion_photos_photo_id ON trip_completion_photos(photo_id);
CREATE INDEX IF NOT EXISTS idx_trip_completion_photos_is_primary ON trip_completion_photos(is_primary);
CREATE INDEX IF NOT EXISTS idx_maintenance_requests_car_id ON maintenance_requests(car_id);
CREATE INDEX IF NOT EXISTS idx_maintenance_requests_status ON maintenance_requests(status);
CREATE INDEX IF NOT EXISTS idx_payment_logs_rental_id ON payment_logs(rental_id);
CREATE INDEX IF NOT EXISTS idx_payment_logs_user_id ON payment_logs(user_id);
CREATE INDEX IF NOT EXISTS idx_photos_user_id ON photos(user_id);
CREATE INDEX IF NOT EXISTS idx_photos_car_id ON photos(car_id);
CREATE INDEX IF NOT EXISTS idx_photos_uploaded_by ON photos(uploaded_by);
CREATE INDEX IF NOT EXISTS idx_sessions_user_id ON sessions(user_id);
CREATE INDEX IF NOT EXISTS idx_car_states_car_id ON car_states(car_id);
CREATE INDEX IF NOT EXISTS idx_car_states_checked_by ON car_states(checked_by);

-- Insert default roles if they don't exist
INSERT INTO roles (name, description) VALUES 
    ('admin', 'Administrator role with full access') 
    ON CONFLICT (name) DO NOTHING;
INSERT INTO roles (name, description) VALUES 
    ('user', 'Regular user role') 
    ON CONFLICT (name) DO NOTHING;
INSERT INTO roles (name, description) VALUES
    ('admin', 'Administrator role with full access'),
    ('user', 'Regular user role with basic access')
    ON CONFLICT (name) DO NOTHING;

-- Add foreign key constraint for main_photo_id after both tables are created
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1
        FROM information_schema.table_constraints
        WHERE constraint_name = 'cars_main_photo_id_fkey'
        AND table_name = 'cars'
    ) THEN
        ALTER TABLE cars ADD CONSTRAINT cars_main_photo_id_fkey
        FOREIGN KEY (main_photo_id) REFERENCES photos(id) ON DELETE SET NULL;
    END IF;
END
$$;

\echo '✅ DB Created'