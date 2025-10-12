\c carsharing_db;

UPDATE users SET status='banned' WHERE email='client1@by.local';
UPDATE users SET driver_id=(SELECT driver_id FROM driver_licenses WHERE license_number='BY-CLT-0001') WHERE email='client1@by.local';
UPDATE users SET cashback=cashback+15.50 WHERE email='client1@by.local';
UPDATE users SET role_id=(SELECT id FROM roles WHERE name='admin') WHERE email='client1@by.local';
UPDATE users SET name='Dima', surname='Ivanov' WHERE email='driver1@by.local';
UPDATE users SET hashed_password='hash_driver1_new' WHERE email='driver1@by.local';
UPDATE users SET email='manager.updated@by.local' WHERE email='manager2@by.local';

UPDATE roles SET name='manager' WHERE name='manager';
UPDATE roles SET description='Полный административный доступ' WHERE name='admin';
UPDATE roles SET description='Клиентские операции' WHERE name='user';
UPDATE roles SET description='Управление автопарком' WHERE name='manager';

UPDATE driver_licenses
SET license_number='BY-DRV-0002',
    issued_by='Minsk MREO #2',
    expiration_date=(CURRENT_DATE + INTERVAL '2 years')::date,
    document_photo_id=(SELECT id FROM photos WHERE url='https://cdn.example.com/by/docs/driver1_license_v2.jpg'),
    status='approved'
WHERE license_number='BY-DRV-0001';

UPDATE driver_licenses SET status='approved' WHERE license_number='BY-CLT-0001';

UPDATE cars SET vin='VF1AAAAAA00000002', model='Renault Clio II' WHERE vin='VF1AAAAAA00000001';
UPDATE cars SET plate_number='3456 GH-5' WHERE vin='WVWZZZ1JZXW000011';
UPDATE cars SET status='maintenance' WHERE vin='JTDKB20U693500021';
UPDATE cars SET position=ST_SetSRID(ST_MakePoint(27.5600,53.9000),4326) WHERE vin='JTDKB20U693500021';

UPDATE car_states cs
SET car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021'),
    checked_by=(SELECT id FROM users WHERE email='admin1@by.local'),
    checked_at=NOW(),
    verified=TRUE,
    comment='Повторная проверка после ТО'
WHERE cs.car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021') AND cs.verified=FALSE;

UPDATE photos
SET object_type='document',
    user_id=(SELECT id FROM users WHERE email='client1@by.local'),
    car_id=NULL,
    url='https://cdn.example.com/by/docs/client1_license_v2.jpg',
    uploaded_by=(SELECT id FROM users WHERE email='admin1@by.local'),
    uploaded_at=NOW()
WHERE url='https://cdn.example.com/by/docs/client1_license_v1.jpg';

UPDATE photos
SET object_type='document',
    uploaded_at=NOW()
WHERE url='https://cdn.example.com/by/avatars/banned1.jpg' AND object_type='driver';

UPDATE photos
SET uploaded_by=(SELECT id FROM users WHERE email='driver1@by.local'),
    uploaded_at=NOW()
WHERE url='https://cdn.example.com/by/cars/jtdkb20u693500021.jpg' AND object_type='car';

UPDATE rentals
SET status='completed',
    ended_at=NOW(),
    price=price+5.00
WHERE user_id=(SELECT id FROM users WHERE email='driver1@by.local')
  AND car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021')
  AND status='active';

UPDATE sessions SET updated_at=NOW() WHERE user_id=(SELECT id FROM users WHERE email='client1@by.local') AND ip='10.1.0.11';

UPDATE maintenance_requests
SET status='in_progress', description='Диагностика начата'
WHERE car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021') AND status='open';

UPDATE maintenance_requests
SET status='resolved', resolved_at=NOW(), description='Работы завершены'
WHERE car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021') AND status='in_progress';

\echo '✅ Update done'

