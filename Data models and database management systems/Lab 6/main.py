import os
import sys
from MongoService import MongoService
import uvicorn
import logging

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)
handler = logging.StreamHandler()
logger.addHandler(handler)

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
        logger.error("Files not found:", ", ".join(missing))
        sys.exit(1)
    mongo_service = MongoService()

    try:
        mongo_service.flush_db()
        logger.info("MongoDB collections flushed")

        logger.info("Start migration")

        mongo_service.load_categories_from_csv(files["categories"])
        mongo_service.load_users_from_csv(files["users"])
        mongo_service.load_products_from_csv(files["products"])
        mongo_service.load_orders_from_csv(files["orders"])
        mongo_service.load_order_items_from_csv(files["order_items"])

        logger.info("Migration succesfully completed!")
        from api import app
        logger.info("Starting API server...")
        uvicorn.run(app, host="0.0.0.0", port=8000)
    except Exception as e:
        logger.error(f"Error: {e}")
        sys.exit(1)

    finally:
        mongo_service.close()
        logger.info("MongoDB connection closed.")


if __name__ == "__main__":
    main()