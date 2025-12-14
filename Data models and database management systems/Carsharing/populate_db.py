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
        description = f"Требуется обслуживание для автомобиля {car_id}. Проблема: {random.choice(['замена масла', 'замена шин', 'проверка тормозов', 'осмотр двигателя', 'ремонт стекла'])}"
        
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

if __name__ == "__main__":
    print("Populating database with test data...")
    populate_roles()
    populate_users()
    populate_cars()
    populate_maintenance_requests()
    print("Database populated with test data successfully!")