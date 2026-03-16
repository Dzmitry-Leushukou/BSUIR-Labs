import logging
from DTO.EmailDTO import EmailDTO
from DTO.LogDTO import LogDTO
from DTO.TaskDTO import TaskDTO
from time import sleep

logger = logging.getLogger("WorkerLogger")
logger.setLevel(logging.INFO)
handler = logging.StreamHandler()
logger.addHandler(handler)

class Worker:
    def __init__(self, redis_service):
        self.redis_service = redis_service

    def get_task(self):
            task = self.redis_service.get_task()
            if task:
                logger.info(f"Task received: {task}")
                return task

    def handle_email_task(self, task: TaskDTO):
        email_data: EmailDTO = task.data
        logger.info(f"Processing email task {task.task_id}: {email_data}")
        sleep(1)  
        logger.info(f"Email sent {email_data.text} from {email_data.sender_email} to {email_data.receiver_email}")

    def handle_log_task(self, task: TaskDTO):
        log_data: LogDTO = task.data
        logger.info(f"Processing log task {task.task_id}: {log_data}")
        sleep(0.2)  
        logger.info(f"Log entry added for {log_data.timestamp}: {log_data.event} - {log_data.text}")

    def run(self):
        logger.info("Worker started, waiting for tasks...")
        try:
            while True:
                logger.info("Checking for new tasks...")
                task=self.get_task()
                if isinstance(task.data, EmailDTO):
                    logger.info(f"It`s an email task: {task.data}")
                    self.handle_email_task(task)
                elif isinstance(task.data, LogDTO):
                    logger.info(f"It`s a log task: {task.data}")
                    self.handle_log_task(task)
                else:
                    logger.info(f"Unknown task type: {task.data}")

        except Exception as e:
            logger.error(f"Error in worker: {e}")
            