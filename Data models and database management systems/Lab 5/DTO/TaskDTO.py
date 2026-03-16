from dataclasses import dataclass
from typing import Union
from .EmailDTO import EmailDTO
from .LogDTO import LogDTO

@dataclass
class TaskDTO:
    task_id: str = ""
    data: Union[EmailDTO, LogDTO] = None
    
    def __post_init__(self):
        if self.data is None:
            raise ValueError("Either email or log must be provided")
        if not isinstance(self.data, (EmailDTO, LogDTO)):
            raise ValueError("data must be either EmailDTO or LogDTO")
    
    def to_key_dict(self):
        return {
            'task_id': self.task_id,
            'data': self.data.to_key_dict()
        }
    
    @classmethod
    def from_dict(cls, data: dict):
        data_obj = data.get('data', {})
    
        if 'sender_email' in data_obj:
            data['data'] = EmailDTO(**data_obj)
        elif 'event' in data_obj:
            data['data'] = LogDTO(**data_obj)
        
        return cls(**data)