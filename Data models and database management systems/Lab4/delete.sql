\c carsharing_db;

DELETE FROM photos
WHERE object_type='driver'
  AND user_id=(SELECT id FROM users WHERE email='driver1@by.local')
  AND url='https://cdn.example.com/by/avatars/driver1.jpg';

DELETE FROM payment_logs
WHERE rental_id=(SELECT id FROM rentals WHERE user_id=(SELECT id FROM users WHERE email='driver1@by.local') AND car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021') LIMIT 1)
  AND pay_type IN ('card','cashback');

DELETE FROM maintenance_requests
WHERE car_id=(SELECT id FROM cars WHERE vin='VF1AAAAAA00000002' OR vin='VF1AAAAAA00000001') AND status='resolved';

DELETE FROM rentals
WHERE car_id=(SELECT id FROM cars WHERE vin='WVWZZZ1JZXW000011') AND status='completed';

DELETE FROM rentals
WHERE car_id=(SELECT id FROM cars WHERE vin='VF1AAAAAA00000002') AND status='cancelled';

DELETE FROM car_states
WHERE car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021');

DELETE FROM photos
WHERE object_type='car'
  AND car_id=(SELECT id FROM cars WHERE vin='JTDKB20U693500021');

DELETE FROM cars
WHERE vin='JTDKB20U693500021';

DELETE FROM sessions
WHERE user_id=(SELECT id FROM users WHERE email='client1@by.local')
  AND ip='10.1.0.11';

DELETE FROM users
WHERE email='manager.updated@by.local' OR email='manager2@by.local';

DELETE FROM driver_licenses
WHERE license_number='BY-BRS-0003';

WITH del AS (
  SELECT id FROM logs
  WHERE action_type IN ('cancel_rental','close_rental')
  ORDER BY id
  LIMIT 1
)
DELETE FROM logs WHERE id IN (SELECT id FROM del);

DELETE FROM roles r
WHERE r.name='manager'
  AND NOT EXISTS (SELECT 1 FROM users u WHERE u.role_id=r.id);

\echo '✅ Delete done'

