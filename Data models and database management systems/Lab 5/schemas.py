from pydantic import BaseModel
from typing import Union

class EmailDTO(BaseModel):
    sender_email: str = ""
    receiver_email: str = ""
    text: str = ""

class LogDTO(BaseModel):
    event: str = ""
    text: str = ""
    timestamp: str = ""

class TaskDTO(BaseModel):
    task_id: str = ""
    def to_key_dict(self):
        return self.model_dump()

class EmailTaskDTO(TaskDTO):
    data: EmailDTO

class LogTaskDTO(TaskDTO):
    data: LogDTO