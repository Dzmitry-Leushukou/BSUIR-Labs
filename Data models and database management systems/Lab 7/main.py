from MongoService import MongoService
import uvicorn
import logging
import os

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)
handler = logging.StreamHandler()
logger.addHandler(handler)


def main():
    mongo_service = MongoService()
    db = mongo_service.db
    data_dir = "Data_csv"

    # Загружаем данные из CSV
    logger.info("Loading data from CSV files...")
    mongo_service.load_all_csv(data_dir)

    # Инициализируем склады и товары
    logger.info("Setting up warehouses...")
    mongo_service.setup_warehouses()

    # Создаём материализованное представление
    logger.info("Creating materialized view...")
    mongo_service.create_monthly_revenue_materialized_view()

    try:
        logger.info("Data loaded successfully!")
        from api import app
        logger.info("Starting API server...")
        uvicorn.run(app, host="0.0.0.0", port=8000)
    except Exception as e:
        logger.error(f"Error: {e}")
        import sys
        sys.exit(1)
    finally:
        mongo_service.close()
        logger.info("MongoDB connection closed.")


if __name__ == "__main__":
    main()
