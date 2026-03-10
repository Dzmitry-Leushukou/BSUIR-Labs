import os
import sys
from RedisService import RedisService
import uvicorn


def main():
    DATA_DIR = "Data_csv"
    files = {
        "categories": os.path.join(DATA_DIR, "categoryid-name.csv"),
        "users": os.path.join(DATA_DIR, "userid-name-email-createdat.csv"),
        "products": os.path.join(DATA_DIR, "productid-name-categoryid-price.csv"),
        "orders": os.path.join(DATA_DIR, "orderid-userid-createdat-status.csv"),
        "order_items": os.path.join(DATA_DIR, "orderitemid-orderid-productid-quantity-price.csv"),
    }

    missing = [name for name, path in files.items() if not os.path.exists(path)]
    if missing:
        print("Files not found:", ", ".join(missing))
        sys.exit(1)

    redis_service = RedisService()

    try:
        redis_service.flush_db()
        print("Redis flushed")

        print("Start migration")

        redis_service.load_categories_from_csv(files["categories"])
        redis_service.load_users_from_csv(files["users"])
        redis_service.load_products_from_csv(files["products"])
        redis_service.load_orders_from_csv(files["orders"])
        redis_service.load_order_items_from_csv(files["order_items"])

        print("Migration succesfully completed!")
        from api import app
        print("Starting API server...")
        uvicorn.run(app, host="0.0.0.0", port=8000)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

    finally:
        redis_service.close()
        print("Redis connection closed.")

    

if __name__ == "__main__":
    main()