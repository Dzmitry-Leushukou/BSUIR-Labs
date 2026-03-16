from dataclasses import dataclass
import datetime

@dataclass
class LogDTO:
    event: str = ""
    text: str = ""
    timestamp: datetime = ""

    def to_key_dict(self):
        return {
            'event': self.event,
            'text': self.text,
            'timestamp': self.timestamp
        }