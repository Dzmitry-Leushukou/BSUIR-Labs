from dataclasses import dataclass

@dataclass
class EmailDTO:
    sender_email: str = ""
    receiver_email: str = ""
    text: str = ""

    def to_key_dict(self):
        return {
            'sender_email': self.sender_email,
            'receiver_email': self.receiver_email,
            'text': self.text
        }