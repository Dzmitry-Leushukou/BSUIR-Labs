DROP DATABASE IF EXISTS carsharing_db;
CREATE DATABASE carsharing_db;

\c carsharing_db;

CREATE EXTENSION IF NOT EXISTS postgis;

CREATE TABLE roles (
    role_id SERIAL PRIMARY KEY,
    role_name VARCHAR(50) UNIQUE NOT NULL,
    description TEXT
);

CREATE TABLE users (
    user_id SERIAL PRIMARY KEY,
    email VARCHAR(255) UNIQUE NOT NULL,
    hashed_password VARCHAR(255) NOT NULL,
    name VARCHAR(50) NOT NULL,
    surname VARCHAR(50) NOT NULL,
    cashback DECIMAL(12,2) DEFAULT 0 CHECK (cashback >= 0),
    role_id INT NOT NULL REFERENCES roles(role_id),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(10) DEFAULT 'active' CHECK (status IN ('active', 'banned'))
);

CREATE TABLE photos (
    photo_id SERIAL PRIMARY KEY,
    object_type VARCHAR(50) NOT NULL CHECK (object_type IN ('driver', 'car', 'document')),
    object_id INT NOT NULL,
    url VARCHAR(500) NOT NULL,
    uploaded_by INT NOT NULL REFERENCES users(user_id),
    uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE driver_licenses (
    driver_id SERIAL PRIMARY KEY,
    license_number VARCHAR(40) UNIQUE NOT NULL,
    issued_by VARCHAR(255) NOT NULL,
    expiration_date DATE NOT NULL CHECK (expiration_date > CURRENT_DATE),
    document_photo_id INT NOT NULL REFERENCES photos(photo_id),
    status VARCHAR(10) DEFAULT 'pending' CHECK (status IN ('pending', 'approved', 'rejected'))
);

ALTER TABLE users ADD COLUMN driver_id INT UNIQUE REFERENCES driver_licenses(driver_id);

CREATE TABLE cars (
    car_id SERIAL PRIMARY KEY,
    vin VARCHAR(17) UNIQUE NOT NULL,
    plate_number VARCHAR(10) UNIQUE NOT NULL,
    model VARCHAR(100) NOT NULL,
    status VARCHAR(20) DEFAULT 'available' CHECK (status IN ('available', 'rented', 'maintenance')),
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    position geometry(Point, 4326)
);

CREATE TABLE sessions (
    session_id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(user_id),
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip INET
);

ALTER TABLE users ADD COLUMN session_id INT REFERENCES sessions(session_id);

CREATE TABLE car_states (
    car_state_id SERIAL PRIMARY KEY,
    car_id INT NOT NULL REFERENCES cars(car_id),
    checked_by INT NOT NULL REFERENCES users(user_id),
    checked_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    verified BOOLEAN DEFAULT FALSE,
    comment TEXT
);

CREATE TABLE rentals (
    rental_id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(user_id),
    car_id INT NOT NULL REFERENCES cars(car_id),
    started_at TIMESTAMP NOT NULL,
    ended_at TIMESTAMP CHECK (ended_at > started_at OR ended_at IS NULL),
    price DECIMAL(12,2) NOT NULL CHECK (price >= 0),
    status VARCHAR(10) DEFAULT 'active' CHECK (status IN ('active', 'completed', 'cancelled'))
);

CREATE TABLE maintenance_requests (
    request_id SERIAL PRIMARY KEY,
    car_id INT NOT NULL REFERENCES cars(car_id),
    reported_by INT NOT NULL REFERENCES users(user_id),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMP CHECK (resolved_at >= created_at OR resolved_at IS NULL),
    status VARCHAR(20) DEFAULT 'open' CHECK (status IN ('open', 'in_progress', 'resolved')),
    description TEXT NOT NULL
);

CREATE TABLE payment_logs (
    id SERIAL PRIMARY KEY,
    rental_id INT NOT NULL REFERENCES rentals(rental_id),
    user_id INT NOT NULL REFERENCES users(user_id),
    pay_type VARCHAR(30) NOT NULL CHECK (pay_type IN ('card', 'cashback')),
    price DECIMAL(10,2) NOT NULL CHECK (price >= 0),
    ip INET
);

CREATE TABLE logs (
    log_id SERIAL PRIMARY KEY,
    actor_user_id INT NOT NULL REFERENCES users(user_id),
    action_type VARCHAR(255) NOT NULL,
    target_id INT REFERENCES users(user_id),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip INET
);

INSERT INTO roles (role_name, description) VALUES 
('admin', 'Admin'),
('user', 'User'),
('manager', 'Manager');

\dt
\echo '✅ DB Created'