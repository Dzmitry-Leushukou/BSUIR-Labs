from dataclasses import dataclass
import datetime

@dataclass
class OrderItemDTO:
    order_items_id: str = ""
    order_id: str = ""
    product_id: str = ""
    quantity: int = 0
    price: float = 0.0

    def from_csv(self, csv_row):
        self.order_items_id = csv_row[0]
        self.order_id = csv_row[1]
        self.product_id = csv_row[2]
        self.quantity = csv_row[3]
        self.price = csv_row[4]

    def to_key_dict(self):
        return {
            'order_id': self.order_id,
            'product_id': self.product_id,
            'quantity': self.quantity,
            'price': self.price
        }