import os
import redis
from DTO.TaskDTO import TaskDTO
import json
import logging
import time

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


    def check_simple_rate(self,client_ip:str, limit:int = 100, window_seconds:int = 600) -> bool:
        key = f"rate:simple:{client_ip}"
        current = self.client.incr(key)
        if current == 1:
            self.client.expire(key, window_seconds)
        return current <= limit


    def check_sliding_window_rate(self, client_ip: str, limit: int = 100, window_seconds: int = 600) -> bool:
        lua_script = """
        local key = KEYS[1]
        local now = tonumber(ARGV[1])
        local window = tonumber(ARGV[2])
        local limit = tonumber(ARGV[3])
        
        local window_start = now - window
        redis.call('ZREMRANGEBYSCORE', key, 0, window_start)
        
        local current = redis.call('ZCARD', key)
        if current < limit then
            redis.call('ZADD', key, now, now)
            redis.call('EXPIRE', key, window)
            return 1
        end
        return 0
        """
        key = f"rate:sliding:{client_ip}"
        now = time.time()
        
        return bool(self.client.eval(lua_script, 1, key, now, window_seconds, limit))

    def flush_db(self):
        self.client.flushdb()
        logger.info("Database flushed.")

    def close(self):
        self.client.close()
        logger.info("Redis client closed.")