from dataclasses import dataclass
import datetime

@dataclass
class OrderDTO:
    order_id: str = ""
    user_id: str = ""
    created_at: datetime = ""
    status: str = ""

    def from_csv(self, csv_row):
        self.order_id = csv_row[0]
        self.user_id = csv_row[1]
        self.created_at = csv_row[2]
        self.status = csv_row[3]

    def to_key_dict(self):
        return {
            'user_id': self.user_id,
            'created_at': self.created_at,
            'status': self.status
        }