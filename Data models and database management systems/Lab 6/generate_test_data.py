from pymongo import MongoClient
import random
import time
from datetime import datetime, timedelta

MONGO_HOST = "localhost"
MONGO_PORT = 27017
MONGO_USER = "admin"
MONGO_PASSWORD = "password"
MONGO_DB = "lab6_db"

NUM_USERS = 100
NUM_PRODUCTS = 500
NUM_CATEGORIES = 20
NUM_ORDERS = 50000
NUM_ORDER_ITEMS_PER_ORDER = 3

def get_connection():
    connection_string = f"mongodb://{MONGO_USER}:{MONGO_PASSWORD}@{MONGO_HOST}:{MONGO_PORT}/"
    client = MongoClient(connection_string, serverSelectionTimeoutMS=5000)
    client.admin.command('ping')
    return client

def generate_categories(db, count):
    collection = db['categories']
    collection.drop()
    categories = []
    for i in range(1, count + 1):
        categories.append({
            "category_id": i,
            "name": f"Category {i}"
        })
    collection.insert_many(categories)
    print(f"Generated {count} categories")

def generate_users(db, count):
    collection = db['users']
    collection.drop()
    users = []
    for i in range(1, count + 1):
        users.append({
            "user_id": i,
            "name": f"User {i}",
            "email": f"user{i}@example.com",
            "created_at": datetime.now().isoformat()
        })
    collection.insert_many(users)
    print(f"Generated {count} users")

def generate_products(db, categories_count, count):
    collection = db['products']
    collection.drop()
    products = []
    for i in range(1, count + 1):
        products.append({
            "product_id": i,
            "name": f"Product {i}",
            "category_id": random.randint(1, categories_count),
            "price": round(random.uniform(10, 1000), 2)
        })
    collection.insert_many(products)
    print(f"Generated {count} products")

def generate_orders(db, users_count, count):
    collection = db['orders']
    collection.drop()
    orders = []
    base_date = datetime.now()
    for i in range(1, count + 1):
        orders.append({
            "order_id": i,
            "user_id": random.randint(1, users_count),
            "created_at": (base_date - timedelta(days=random.randint(0, 365))).isoformat(),
            "status": random.choice(["pending", "processing", "shipped", "delivered", "cancelled"])
        })
    collection.insert_many(orders)
    print(f"Generated {count} orders")

def generate_order_items(db, orders_count, products_count, items_per_order):
    collection = db['order_items']
    collection.drop()
    order_items = []
    order_item_id = 1
    for order_id in range(1, orders_count + 1):
        for _ in range(random.randint(1, items_per_order)):
            order_items.append({
                "order_item_id": order_item_id,
                "order_id": order_id,
                "product_id": random.randint(1, products_count),
                "quantity": random.randint(1, 10),
                "price": round(random.uniform(10, 1000), 2)
            })
            order_item_id += 1
    collection.insert_many(order_items)
    print(f"Generated {len(order_items)} order items")

def main():
    print("Connecting to MongoDB...")
    client = get_connection()
    db = client[MONGO_DB]
    
    print("\nGenerating test data...")
    start_time = time.time()
    
    generate_categories(db, NUM_CATEGORIES)
    generate_users(db, NUM_USERS)
    generate_products(db, NUM_CATEGORIES, NUM_PRODUCTS)
    generate_orders(db, NUM_USERS, NUM_ORDERS)
    generate_order_items(db, NUM_ORDERS, NUM_PRODUCTS, NUM_ORDER_ITEMS_PER_ORDER)
    
    elapsed = time.time() - start_time
    print(f"\nData generation completed in {elapsed:.2f} seconds")
    
    print(f"\nCollection counts:")
    for collection in ["categories", "users", "products", "orders", "order_items"]:
        count = db[collection].count_documents({})
        print(f"  {collection}: {count}")
    
    client.close()
    print("\nDone!")

if __name__ == "__main__":
    main()
