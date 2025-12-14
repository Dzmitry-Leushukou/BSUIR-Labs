import psycopg2
import os
from datetime import datetime, timedelta, timezone
from dotenv import load_dotenv
import random
import bcrypt

load_dotenv()

def get_db_connection():
    conn = psycopg2.connect(
        host=os.getenv("DB_HOST", "localhost"),
        database=os.getenv("DB_NAME", "carsharing_db"),
        user=os.getenv("DB_USER", "postgres"),
        password=os.getenv("DB_PASSWORD", "postgres"),
        port=os.getenv("DB_PORT", 5432)
    )
    return conn

def populate_roles():
    conn = get_db_connection()
    cur = conn.cursor()
    
    roles = [
        ("admin", "Administrator role with full access"),
        ("user", "Regular user role")
    ]
    
    for role in roles:
        cur.execute(
            "INSERT INTO roles (name, description) VALUES (%s, %s) ON CONFLICT (name) DO NOTHING",
            role
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Roles populated successfully")

def populate_users():
    conn = get_db_connection()
    cur = conn.cursor()
    
    # Get role IDs
    cur.execute("SELECT id, name FROM roles")
    roles = {name: id for id, name in cur.fetchall()}
    
    users = [
        ("admin@example.com", bcrypt.hashpw("admin123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Admin", "User", 0.0, roles["admin"], "active"),
        ("john.doe@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "John", "Doe", 15.50, roles["user"], "active"),
        ("jane.smith@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Jane", "Smith", 25.00, roles["user"], "active"),
        ("bob.johnson@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Bob", "Johnson", 10.75, roles["user"], "active"),
        ("alice.brown@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Alice", "Brown", 30.25, roles["user"], "active"),
        ("charlie.wilson@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Charlie", "Wilson", 5.50, roles["user"], "active"),
        ("diana.miller@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Diana", "Miller", 40.00, roles["user"], "active"),
        ("eve.taylor@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Eve", "Taylor", 22.30, roles["user"], "banned"),
        ("frank.moore@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Frank", "Moore", 18.90, roles["user"], "active"),
        ("grace.lee@example.com", bcrypt.hashpw("password123".encode('utf-8'), bcrypt.gensalt()).decode('utf-8'), "Grace", "Lee", 35.75, roles["user"], "active")
    ]
    
    for user in users:
        cur.execute(
            """INSERT INTO users (email, hashed_password, name, surname, cashback, role_id, status) 
               VALUES (%s, %s, %s, %s, %s, %s, %s) ON CONFLICT (email) DO NOTHING""",
            user
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Users populated successfully")

def populate_cars():
    conn = get_db_connection()
    cur = conn.cursor()
    
    cars = [
        ("1HGBH41JXMN109186", "1234 AB-1", "Toyota Camry", "available", "POINT(27.5615 53.9041)"),  # Минск
        ("2T1BURHE5JC012345", "5678 CD-2", "Honda Civic", "available", "POINT(27.5575 53.9025)"), # Минск
        ("WBAVA33598NL67890", "9012 EF-3", "BMW X3", "rented", "POINT(27.5634 53.9062)"),  # Минск
        ("1FTFW1E83GKD12345", "3456 GH-4", "Ford F-150", "available", "POINT(27.5598 53.9012)"),  # Минск
        ("JH4NA21691T000123", "7890 IJ-5", "Acura TLX", "maintenance", "POINT(27.5651 53.9038)"),  # Минск
        ("KMHDH4AE2CU456789", "2345 JK-6", "Hyundai Sonata", "available", "POINT(27.587 53.9056)"),  # Минск
        ("1C4RJFAG4FC123456", "6789 KL-7", "Jeep Cherokee", "available", "POINT(27.5623 53.9071)"),  # Минск
        ("WBAVA53589NL78901", "1011 LM-7", "BMW 3 Series", "rented", "POINT(27.5569 53.9021)"),  # Минск
        ("2T1BURHE6JC023456", "1213 MN-7", "Honda Accord", "available", "POINT(27.5642 53.9049)"),  # Минск
        ("1HGBH41JXMN110197", "1415 OP-1", "Toyota Corolla", "available", "POINT(27.578 53.9067)"), # Минск
        ("1HGBH41JXMN109187", "1234 AB-2", "Toyota Camry", "available", "POINT(-122.4194 37.7749)"),
        ("2T1BURHE5JC012346", "5678 CD-3", "Honda Civic", "available", "POINT(-122.4184 37.759)"),
        ("WBAVA33598NL67891", "9012 EF-4", "BMW X3", "rented", "POINT(-122.4174 37.769)"),
        ("1FTFW1E83GKD12346", "3456 GH-5", "Ford F-150", "available", "POINT(-122.4164 37.7779)"),
        ("JH4NA21691T000124", "7890 IJ-6", "Acura TLX", "maintenance", "POINT(-122.4154 37.789)"),
        ("KMHDH4AE2CU456790", "2345 JK-7", "Hyundai Sonata", "available", "POINT(-122.4144 37.7799)"),
        ("1C4RJFAG4FC123457", "6789 KL-8", "Jeep Cherokee", "available", "POINT(-122.4134 37.7809)"),
        ("WBAVA53589NL78902", "1011 LM-8", "BMW 3 Series", "rented", "POINT(-122.4124 37.7819)"),
        ("2T1BURHE6JC023457", "1213 MN-8", "Honda Accord", "available", "POINT(-122.414 37.7829)"),
        ("1HGBH41JXMN110198", "1415 OP-2", "Toyota Corolla", "available", "POINT(-122.4104 37.7839)")
    ]
    
    for car in cars:
        cur.execute(
            """INSERT INTO cars (vin, plate_number, model, status, position) 
               VALUES (%s, %s, %s, %s, ST_GeomFromText(%s, 4326)) ON CONFLICT (vin) DO NOTHING""",
            car
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Cars populated successfully")

def populate_photos():
    conn = get_db_connection()
    cur = conn.cursor()
    
    # Get user and car IDs
    cur.execute("SELECT id FROM users")
    user_ids = [row[0] for row in cur.fetchall()]
    
    cur.execute("SELECT id FROM cars")
    car_ids = [row[0] for row in cur.fetchall()]
    
    photos = [
        ("driver", user_ids[1], None, b"fake_image_data_1", "driver1.jpg", "image/jpeg", 1024, user_ids[0]),
        ("driver", user_ids[2], None, b"fake_image_data_2", "driver2.jpg", "image/jpeg", 1024, user_ids[0]),
        ("car", None, car_ids[0], b"fake_image_data_3", "car1.jpg", "image/jpeg", 1024, user_ids[0]),
        ("car", None, car_ids[1], b"fake_image_data_4", "car2.jpg", "image/jpeg", 1024, user_ids[0]),
        ("document", user_ids[1], None, b"fake_image_data_5", "license1.jpg", "image/jpeg", 1024, user_ids[0]),
        ("car", None, car_ids[2], b"fake_image_data_6", "car3.jpg", "image/jpeg", 1024, user_ids[0]),
        ("driver", user_ids[3], None, b"fake_image_data_7", "driver3.jpg", "image/jpeg", 1024, user_ids[0]),
        ("document", user_ids[2], None, b"fake_image_data_8", "license2.jpg", "image/jpeg", 1024, user_ids[0]),
        ("car", None, car_ids[3], b"fake_image_data_9", "car4.jpg", "image/jpeg", 1024, user_ids[0]),
        ("car", None, car_ids[4], b"fake_image_data_10", "car5.jpg", "image/jpeg", 1024, user_ids[0])
    ]
    
    for photo in photos:
        cur.execute(
            """INSERT INTO photos (object_type, user_id, car_id, file_data, filename, content_type, file_size, uploaded_by)
               VALUES (%s, %s, %s, %s, %s, %s, %s, %s)""",
            photo
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Photos populated successfully")

def populate_driver_licenses():
    conn = get_db_connection()
    cur = conn.cursor()
    
    # Get photo IDs
    cur.execute("SELECT id FROM photos WHERE object_type = 'document'")
    photo_ids = [row[0] for row in cur.fetchall()]
    
    # Get user IDs
    cur.execute("SELECT id FROM users WHERE id NOT IN (SELECT driver_id FROM users WHERE driver_id IS NOT NULL)")
    user_ids = [row[0] for row in cur.fetchall()]
    
    licenses = []
    for i in range(min(len(user_ids), len(photo_ids))):
        license_number = f"LIC{i+1:04d}"
        issued_by = f"DMV State {i+1}"
        expiration_date = (datetime.now(timezone.utc) + timedelta(days=365*3)).strftime('%Y-%m-%d')
        document_photo_id = photo_ids[i]
        status = random.choice(["approved", "pending"])
        
        licenses.append((user_ids[i], license_number, issued_by, expiration_date, document_photo_id, status))
    
    for license in licenses:
        user_id, license_number, issued_by, expiration_date, document_photo_id, status = license
        cur.execute(
            """INSERT INTO driver_licenses (license_number, issued_by, expiration_date, document_photo_id, document_photo_back_id, status)
               VALUES (%s, %s, %s, %s, NULL, %s) ON CONFLICT (license_number) DO NOTHING""",
            (license_number, issued_by, expiration_date, document_photo_id, status)
        )
        
        # Update user with driver_id if the license was inserted
        if cur.rowcount > 0:  # Only if a new record was inserted
            # Get the last inserted driver_id
            cur.execute("SELECT currval('driver_licenses_driver_id_seq')")
            new_driver_id = cur.fetchone()[0]
            cur.execute("UPDATE users SET driver_id = %s WHERE id = %s", (new_driver_id, user_id))
    
    conn.commit()
    cur.close()
    conn.close()
    print("Driver licenses populated successfully")

def populate_sessions():
    conn = get_db_connection()
    cur = conn.cursor()
    
    # Get user IDs
    cur.execute("SELECT id FROM users")
    user_ids = [row[0] for row in cur.fetchall()]
    
    sessions = []
    for i in range(15):
        user_id = random.choice(user_ids)
        sessions.append((user_id,))
    
    for session in sessions:
        cur.execute(
            """INSERT INTO sessions (user_id)
               VALUES (%s)""",
            session
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Sessions populated successfully")

def populate_car_states():
    conn = get_db_connection()
    cur = conn.cursor()
    
    # Get car and user IDs
    cur.execute("SELECT id FROM cars")
    car_ids = [row[0] for row in cur.fetchall()]
    
    cur.execute("SELECT id FROM users")
    user_ids = [row[0] for row in cur.fetchall()]
    
    states = []
    for i in range(10):
        car_id = random.choice(car_ids)
        checked_by = random.choice(user_ids)
        verified = random.choice([True, False])
        comment = f"Inspection comment {i+1}" if random.random() > 0.3 else None
        states.append((car_id, checked_by, verified, comment))
    
    for state in states:
        cur.execute(
            """INSERT INTO car_states (car_id, checked_by, verified, comment) 
               VALUES (%s, %s, %s, %s)""",
            state
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Car states populated successfully")

def populate_rentals():
    conn = get_db_connection()
    cur = conn.cursor()
    
    # Get user and car IDs
    cur.execute("SELECT id FROM users WHERE role_id != (SELECT id FROM roles WHERE name = 'admin')")
    user_ids = [row[0] for row in cur.fetchall()]
    
    cur.execute("SELECT id FROM cars")
    car_ids = [row[0] for row in cur.fetchall()]
    
    rentals = []
    for i in range(15):
        user_id = random.choice(user_ids)
        car_id = random.choice(car_ids)
        started_at = datetime.now(timezone.utc) - timedelta(days=random.randint(1, 30))
        price = round(random.uniform(20.0, 100.0), 2)
        status = random.choice(["active", "completed", "cancelled"])
        
        ended_at = None
        if status == "completed":
            ended_at = started_at + timedelta(days=random.randint(1, 7))
        elif status == "cancelled":
            ended_at = started_at + timedelta(hours=random.randint(1, 24))
        
        rentals.append((user_id, car_id, started_at, ended_at, price, status))
    
    for rental in rentals:
        user_id, car_id, started_at, ended_at, price, status = rental
        cur.execute(
            """INSERT INTO rentals (user_id, car_id, started_at, ended_at, price, status) 
               VALUES (%s, %s, %s, %s, %s, %s)""",
            (user_id, car_id, started_at, ended_at, price, status)
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Rentals populated successfully")

def populate_maintenance_requests():
    conn = get_db_connection()
    cur = conn.cursor()
    
    # Get car and user IDs
    cur.execute("SELECT id FROM cars")
    car_ids = [row[0] for row in cur.fetchall()]
    
    cur.execute("SELECT id FROM users")
    user_ids = [row[0] for row in cur.fetchall()]
    
    for i in range(8):
        car_id = random.choice(car_ids)
        reported_by = random.choice(user_ids)
        status = random.choice(["open", "in_progress", "resolved"])
        description = f"Maintenance required for car {car_id}. Issue: {random.choice(['oil change', 'tire rotation', 'brake check', 'engine inspection', 'windshield repair'])}"
        
        # Determine dates based on status
        if status == "resolved":
            # For resolved requests, create them in the past and resolve them after some time
            created_at = datetime.now(timezone.utc) - timedelta(days=random.randint(5, 15))
            resolved_at = created_at + timedelta(days=random.randint(1, 4))
        elif status == "in_progress":
            # For in-progress requests, they were created in the past but not yet resolved
            created_at = datetime.now(timezone.utc) - timedelta(days=random.randint(1, 7))
            resolved_at = None
        else:  # open
            # For open requests, they were just created
            created_at = datetime.now(timezone.utc)
            resolved_at = None
        
        cur.execute(
            """INSERT INTO maintenance_requests (car_id, reported_by, created_at, resolved_at, status, description)
               VALUES (%s, %s, %s, %s, %s, %s)""",
            (car_id, reported_by, created_at, resolved_at, status, description)
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Maintenance requests populated successfully")

def populate_payment_logs():
    conn = get_db_connection()
    cur = conn.cursor()
    
    # Get rental and user IDs
    cur.execute("SELECT id, user_id FROM rentals WHERE status != 'cancelled'")
    rental_user_pairs = cur.fetchall()
    
    logs = []
    for rental_id, user_id in rental_user_pairs[:12]:  # Only use some rentals
        rental_id, user_id = rental_id, user_id
        pay_type = random.choice(["card", "cashback"])
        price = round(random.uniform(15.0, 80.0), 2)
        
        logs.append((rental_id, user_id, pay_type, price))
    
    for log in logs:
        cur.execute(
            """INSERT INTO payment_logs (rental_id, user_id, pay_type, price)
               VALUES (%s, %s, %s, %s)""",
            log
        )
    
    conn.commit()
    cur.close()
    conn.close()
    print("Payment logs populated successfully")

if __name__ == "__main__":
    print("Populating database with test data...")
    populate_roles()
    populate_users()
    populate_cars()
    populate_photos()
    populate_driver_licenses()
    populate_sessions()
    populate_car_states()
    populate_rentals()
    populate_maintenance_requests()
    populate_payment_logs()
    print("Database populated with test data successfully!")