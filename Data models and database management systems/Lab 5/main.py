from RedisService import RedisService
import uvicorn
import sys
import threading
import logging

logger = logging.getLogger("MainLogger")
logger.setLevel(logging.INFO)
handler = logging.StreamHandler()
logger.addHandler(handler)

def main():
    redis_service = RedisService()

    try:
        redis_service.flush_db()
        logger.info("Redis flushed")
        logger.info("Starting worker...")
        from worker import Worker
        worker = Worker(redis_service)

        thread = threading.Thread(target=worker.run)
        thread.start()
        logger.info("Worker started in other thread")

        from api import app
        logger.info("Starting API server...")
        uvicorn.run(app, host="0.0.0.0", port=8000)
    except Exception as e:
        logger.error(f"Error: {e}")
        sys.exit(1)

    finally:
        redis_service.close()
        logger.info("Redis connection closed.")

if __name__ == "__main__":
    main()