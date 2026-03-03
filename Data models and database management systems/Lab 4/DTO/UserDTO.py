from dataclasses import dataclass
import datetime

@dataclass
class UserDTO:
    user_id:str
    name:str
    email:str
    created_at:datetime

    def from_csv(self, csv_row):
        self.user_id = csv_row[0]
        self.name = csv_row[1]
        self.email = csv_row[2]
        self.created_at = csv_row[3]

    def to_key_dict(self):
        return {
            'name': self.name,
            'email': self.email,
            'created_at': self.created_at
        }