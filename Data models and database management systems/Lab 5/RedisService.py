import os
import redis
from DTO.TaskDTO import TaskDTO
import json
import logging

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)
handler = logging.StreamHandler()
logger.addHandler(handler)

QUEUE_KEY="queue:tasks"
class RedisService:
    def __init__(self):
        self.host = os.getenv('REDIS_HOST')
        self.port = os.getenv('REDIS_PORT')
        self.client=redis.Redis(host=self.host, port=self.port,
            decode_responses=True,)

    def add_task(self, task: TaskDTO):
        self.client.rpush(QUEUE_KEY, json.dumps(task.to_key_dict()))
        logger.info(f"Task {task.task_id} added to queue.")
    
    def get_task(self) -> TaskDTO:
        result = self.client.blpop(QUEUE_KEY, timeout=0)
        if result:
            task = TaskDTO.from_dict(json.loads(result[1]))
            logger.info(f"Task {task.task_id} retrieved from queue.")
            return task

    def flush_db(self):
        self.client.flushdb()
        logger.info("Database flushed.")

    def close(self):
        self.client.close()
        logger.info("Redis client closed.")