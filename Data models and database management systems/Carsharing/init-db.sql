-- This script runs during PostgreSQL container initialization
-- The database specified by POSTGRES_DB in docker-compose.yml is already created and selected

-- Create PostGIS extension
CREATE EXTENSION IF NOT EXISTS postgis;

-- Create roles table
CREATE TABLE IF NOT EXISTS roles (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL CHECK (name IN ('admin','user','manager')),
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
    status VARCHAR(20) DEFAULT 'available' CHECK (status IN ('available','rented','maintenance')),
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    position geometry(Point, 4326),
    CONSTRAINT plate_by_format_chk CHECK (plate_number ~ '^[0-9]{4} [A-Z]{2}-[0-8]$')
);

-- Create photos table
CREATE TABLE IF NOT EXISTS photos (
    id SERIAL PRIMARY KEY,
    object_type VARCHAR(50) NOT NULL CHECK (object_type IN ('driver','car','document')),
    user_id INT,
    car_id INT,
    url VARCHAR(500) NOT NULL,
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

-- Add unique constraint to photos url column for ON CONFLICT to work
ALTER TABLE photos ADD CONSTRAINT uk_photos_url UNIQUE (url);

-- Create driver_licenses table
CREATE TABLE IF NOT EXISTS driver_licenses (
    driver_id SERIAL PRIMARY KEY,
    license_number VARCHAR(40) UNIQUE NOT NULL,
    issued_by VARCHAR(255) NOT NULL,
    expiration_date DATE NOT NULL CHECK (expiration_date > CURRENT_DATE),
    document_photo_id INT NOT NULL REFERENCES photos(id) ON DELETE CASCADE,
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

-- Create sessions table
CREATE TABLE IF NOT EXISTS sessions (
    id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip INET
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
    started_at TIMESTAMP NOT NULL,
    ended_at TIMESTAMP CHECK (ended_at > started_at OR ended_at IS NULL),
    price DECIMAL(12,2) NOT NULL CHECK (price >= 0),
    status VARCHAR(10) DEFAULT 'active' CHECK (status IN ('active','completed','cancelled'))
);

-- Create maintenance_requests table
CREATE TABLE IF NOT EXISTS maintenance_requests (
    id SERIAL PRIMARY KEY,
    car_id INT NOT NULL REFERENCES cars(id) ON DELETE CASCADE,
    reported_by INT REFERENCES users(id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMP CHECK (resolved_at >= created_at OR resolved_at IS NULL),
    status VARCHAR(20) DEFAULT 'open' CHECK (status IN ('open','in_progress','resolved')),
    description TEXT NOT NULL
);

-- Create payment_logs table
CREATE TABLE IF NOT EXISTS payment_logs (
    id SERIAL PRIMARY KEY,
    rental_id INT NOT NULL REFERENCES rentals(id) ON DELETE CASCADE,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    pay_type VARCHAR(30) NOT NULL CHECK (pay_type IN ('card','cashback')),
    price DECIMAL(10,2) NOT NULL CHECK (price >= 0),
    ip INET
);

-- Create logs table
CREATE TABLE IF NOT EXISTS logs (
    id SERIAL PRIMARY KEY,
    actor_user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    action_type VARCHAR(255) NOT NULL,
    target_id INT REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip INET
);

-- Set timezone
SET TIME ZONE 'Europe/Minsk';

-- Create action_logs table for logging system
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
            'price', NEW.price,
            'ip', NEW.ip::TEXT
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
    ('manager', 'Manager role with limited admin access') 
    ON CONFLICT (name) DO NOTHING;

\echo '✅ DB Created'