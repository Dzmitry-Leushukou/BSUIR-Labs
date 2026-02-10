from typing import Generator
import psycopg2
from psycopg2.extras import RealDictCursor
import os
import time

def get_db_connection():
    max_retries = 5
    retries = 0
    
    while retries < max_retries:
        try:
            conn = psycopg2.connect(
                host=os.getenv("DB_HOST", "localhost"),
                database=os.getenv("DB_NAME", "carsharing_db"),
                user=os.getenv("DB_USER", "postgres"),
                password=os.getenv("DB_PASSWORD", "postgres"),
                port=os.getenv("DB_PORT", 5432)
            )
            return conn
        except psycopg2.OperationalError as e:
            if "could not translate host name" in str(e):
                print(f"Database host not available, retrying in 2 seconds... (attempt {retries+1}/{max_retries})")
                time.sleep(2)
                retries += 1
            else:
                # If it's a different error, raise it immediately
                raise e
    
    # If we've exhausted retries, raise an exception
    raise psycopg2.OperationalError("Could not connect to database after maximum retries")