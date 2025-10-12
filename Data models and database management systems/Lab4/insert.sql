\c carsharing_db;

INSERT INTO roles (name, description) VALUES
('admin','Админ'),('user','Клиент'),('manager','Менеджер');

INSERT INTO users (email, hashed_password, name, surname, cashback, role_id, status) VALUES
('admin1@by.local','hash_admin','Alice','Admin',0,(SELECT id FROM roles WHERE name='admin'),'active'),
('driver1@by.local','hash_driver1','Dmitry','Ivanov',5,(SELECT id FROM roles WHERE name='user'),'active'),
('client1@by.local','hash_client1','Olga','Petrova',2,(SELECT id FROM roles WHERE name='user'),'active'),
('manager2@by.local','hash_mgr2','Pavel','Sidorov',0,(SELECT id FROM roles WHERE name='manager'),'active'),
('banned1@by.local','hash_ban','Boris','Kozlov',0,(SELECT id FROM roles WHERE name='user'),'banned');

INSERT INTO photos (object_type, user_id, car_id, url, uploaded_by) VALUES
('driver',(SELECT id FROM users WHERE email='driver1@by.local'),NULL,'https://cdn.example.com/by/avatars/driver1.jpg',(SELECT id FROM users WHERE email='admin1@by.local')),
('driver',(SELECT id FROM users WHERE email='banned1@by.local'),NULL,'https://cdn.example.com/by/avatars/banned1.jpg',(SELECT id FROM users WHERE email='admin1@by.local')),
('document',(SELECT id FROM users WHERE email='driver1@by.local'),NULL,'https://cdn.example.com/by/docs/driver1_license_v1.jpg',(SELECT id FROM users WHERE email='admin1@by.local')),
('document',(SELECT id FROM users WHERE email='driver1@by.local'),NULL,'https://cdn.example.com/by/docs/driver1_license_v2.jpg',(SELECT id FROM users WHERE email='admin1@by.local')),
('document',(SELECT id FROM users WHERE email='client1@by.local'),NULL,'https://cdn.example.com/by/docs/client1_license_v1.jpg',(SELECT id FROM users WHERE email='admin1@by.local')),
('document',(SELECT id FROM users WHERE email='banned1@by.local'),NULL,'https://cdn.example.com/by/docs/banned1_license.jpg',(SELECT id FROM users WHERE email='admin1@by.local'));

INSERT INTO driver_licenses (license_number, issued_by, expiration_date, document_photo_id, status) VALUES
('BY-DRV-0001','Minsk MREO',(CURRENT_DATE + INTERVAL '3 years')::date,(SELECT id FROM photos WHERE url='https://cdn.example.com/by/docs/driver1_license_v1.jpg'),'approved'),
('BY-CLT-0001','Minsk MREO',(CURRENT_DATE + INTERVAL '4 years')::date,(SELECT id FROM photos WHERE url='https://cdn.example.com/by/docs/client1_license_v1.jpg'),'pending'),
('BY-BND-0001','Minsk MREO',(CURRENT_DATE + INTERVAL '5 years')::date,(SELECT id FROM photos WHERE url='https://cdn.example.com/by/docs/banned1_license.jpg'),'rejected'),
('BY-BRS-0003','Brest MREO',(CURRENT_DATE + INTERVAL '5 years')::date,(SELECT id FROM photos WHERE url='https://cdn.example.com/by/docs/driver1_license_v2.jpg'),'pending');

UPDATE users u
SET driver_id = dl.driver_id
FROM driver_licenses dl
WHERE u.email='driver1@by.local' AND dl.license_number='BY-DRV-0001' AND (u.driver_id IS DISTINCT FROM dl.driver_id);

INSERT INTO cars (vin, plate_number, model, status, position) VALUES
('JTDKB20U693500021','1234 AB-7','Toyota Prius','rented',ST_SetSRID(ST_MakePoint(27.5619,53.9023),4326)),
('VF1AAAAAA00000001','5678 CD-1','Renault Clio','available',ST_SetSRID(ST_MakePoint(27.5550,53.9000),4326)),
('WVWZZZ1JZXW000011','9012 EF-3','VW Golf','maintenance',ST_SetSRID(ST_MakePoint(27.5800,53.9100),4326));

INSERT INTO photos (object_type, user_id, car_id, url, uploaded_by) VALUES
('car',NULL,(SELECT id FROM cars WHERE vin='JTDKB20U693500021'),'https://cdn.example.com/by/cars/jtdkb20u693500021.jpg',(SELECT id FROM users WHERE email='admin1@by.local'));

INSERT INTO sessions (user_id, ip) VALUES
((SELECT id FROM users WHERE email='client1@by.local'),'10.1.0.11'::inet),
((SELECT id FROM users WHERE email='banned1@by.local'),'10.1.0.12'::inet);

INSERT INTO car_states (car_id, checked_by, verified, comment) VALUES
((SELECT id FROM cars WHERE vin='JTDKB20U693500021'),(SELECT id FROM users WHERE email='admin1@by.local'),FALSE,'Первичный осмотр'),
((SELECT id FROM cars WHERE vin='VF1AAAAAA00000001'),(SELECT id FROM users WHERE email='admin1@by.local'),TRUE,'ТО пройдено'),
((SELECT id FROM cars WHERE vin='WVWZZZ1JZXW000011'),(SELECT id FROM users WHERE email='admin1@by.local'),FALSE,'Ожидает ремонт');

INSERT INTO rentals (user_id, car_id, started_at, ended_at, price, status) VALUES
((SELECT id FROM users WHERE email='driver1@by.local'),(SELECT id FROM cars WHERE vin='JTDKB20U693500021'),NOW() - INTERVAL '1 day',NULL,25.00,'active'),
((SELECT id FROM users WHERE email='client1@by.local'),(SELECT id FROM cars WHERE vin='WVWZZZ1JZXW000011'),NOW() - INTERVAL '3 days',NOW() - INTERVAL '2 days',50.00,'completed'),
((SELECT id FROM users WHERE email='banned1@by.local'),(SELECT id FROM cars WHERE vin='VF1AAAAAA00000001'),NOW() - INTERVAL '5 days',NOW() - INTERVAL '5 days' + INTERVAL '1 hour',0.00,'cancelled');

INSERT INTO maintenance_requests (car_id, reported_by, status, description, created_at, resolved_at) VALUES
((SELECT id FROM cars WHERE vin='JTDKB20U693500021'),(SELECT id FROM users WHERE email='admin1@by.local'),'open','Проверить датчик давления шин',NOW(),NULL),
((SELECT id FROM cars WHERE vin='WVWZZZ1JZXW000011'),(SELECT id FROM users WHERE email='admin1@by.local'),'in_progress','Диагностика подвески',NOW() - INTERVAL '1 day',NULL),
((SELECT id FROM cars WHERE vin='VF1AAAAAA00000001'),(SELECT id FROM users WHERE email='admin1@by.local'),'resolved','Замена ламп ближнего света',NOW() - INTERVAL '2 days',NOW() - INTERVAL '1 day');

INSERT INTO payment_logs (rental_id, user_id, pay_type, price, ip) VALUES
((SELECT id FROM rentals WHERE user_id=(SELECT id FROM users WHERE email='driver1@by.local') AND car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021') LIMIT 1),(SELECT id FROM users WHERE email='driver1@by.local'),'card',10.00,'10.1.1.10'),
((SELECT id FROM rentals WHERE user_id=(SELECT id FROM users WHERE email='driver1@by.local') AND car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021') LIMIT 1),(SELECT id FROM users WHERE email='driver1@by.local'),'cashback',5.00,'10.1.1.10'),
((SELECT id FROM rentals WHERE user_id=(SELECT id FROM users WHERE email='client1@by.local') AND car_id=(SELECT id FROM cars WHERE vin='WVWZZZ1JZXW000011') LIMIT 1),(SELECT id FROM users WHERE email='client1@by.local'),'card',50.00,'10.1.1.20');

INSERT INTO logs (actor_user_id, action_type, target_id, ip) VALUES
((SELECT id FROM users WHERE email='admin1@by.local'),'cancel_rental',(SELECT id FROM users WHERE email='driver1@by.local'),'10.9.0.1'),
((SELECT id FROM users WHERE email='admin1@by.local'),'close_rental',(SELECT id FROM users WHERE email='client1@by.local'),'10.9.0.2');

\echo '✅ Seed done'

