DROP DATABASE IF EXISTS carsharing_db;
CREATE DATABASE carsharing_db;

\c carsharing_db;

CREATE EXTENSION IF NOT EXISTS postgis;

CREATE TABLE IF NOT EXISTS roles (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL CHECK (name IN ('admin','user','manager')),
    description TEXT
);

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

CREATE TABLE IF NOT EXISTS cars (
    id SERIAL PRIMARY KEY,
    vin VARCHAR(17) UNIQUE NOT NULL,
    plate_number VARCHAR(12) UNIQUE NOT NULL,
    model VARCHAR(100) NOT NULL,
    status VARCHAR(20) DEFAULT 'available' CHECK (status IN ('available','rented','maintenance')),
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    position geometry(Point, 4326),
    CONSTRAINT plate_by_format_chk CHECK (plate_number ~ '^[0-9]{4} [A-Z]{2}-[1-7]$')
);

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

CREATE TABLE IF NOT EXISTS driver_licenses (
    driver_id SERIAL PRIMARY KEY,
    license_number VARCHAR(40) UNIQUE NOT NULL,
    issued_by VARCHAR(255) NOT NULL,
    expiration_date DATE NOT NULL CHECK (expiration_date > CURRENT_DATE),
    document_photo_id INT NOT NULL REFERENCES photos(id) ON DELETE CASCADE,
    status VARCHAR(10) DEFAULT 'pending' CHECK (status IN ('pending','approved','rejected'))
);

ALTER TABLE users ADD COLUMN IF NOT EXISTS driver_id INT UNIQUE REFERENCES driver_licenses(driver_id) ON DELETE SET NULL;

CREATE TABLE IF NOT EXISTS sessions (
    id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip INET
);

CREATE TABLE IF NOT EXISTS car_states (
    id SERIAL PRIMARY KEY,
    car_id INT NOT NULL REFERENCES cars(id) ON DELETE CASCADE,
    checked_by INT REFERENCES users(id) ON DELETE SET NULL,
    checked_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    verified BOOLEAN DEFAULT FALSE,
    comment TEXT
);

CREATE TABLE IF NOT EXISTS rentals (
    id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    car_id INT NOT NULL REFERENCES cars(id) ON DELETE CASCADE,
    started_at TIMESTAMP NOT NULL,
    ended_at TIMESTAMP CHECK (ended_at > started_at OR ended_at IS NULL),
    price DECIMAL(12,2) NOT NULL CHECK (price >= 0),
    status VARCHAR(10) DEFAULT 'active' CHECK (status IN ('active','completed','cancelled'))
);

CREATE TABLE IF NOT EXISTS maintenance_requests (
    id SERIAL PRIMARY KEY,
    car_id INT NOT NULL REFERENCES cars(id) ON DELETE CASCADE,
    reported_by INT REFERENCES users(id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMP CHECK (resolved_at >= created_at OR resolved_at IS NULL),
    status VARCHAR(20) DEFAULT 'open' CHECK (status IN ('open','in_progress','resolved')),
    description TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS payment_logs (
    id SERIAL PRIMARY KEY,
    rental_id INT NOT NULL REFERENCES rentals(id) ON DELETE CASCADE,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    pay_type VARCHAR(30) NOT NULL CHECK (pay_type IN ('card','cashback')),
    price DECIMAL(10,2) NOT NULL CHECK (price >= 0),
    ip INET
);

CREATE TABLE IF NOT EXISTS logs (
    id SERIAL PRIMARY KEY,
    actor_user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    action_type VARCHAR(255) NOT NULL,
    target_id INT REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip INET
);

\dt
\echo '✅ DB Created'

