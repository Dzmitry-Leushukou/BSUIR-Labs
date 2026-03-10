from dataclasses import dataclass

@dataclass
class ProductDTO:
    product_id: str = ""
    name: str = ""
    category_id: str = ""
    price: float = 0.0

    def from_csv(self, csv_row):
        self.product_id = csv_row[0]
        self.name = csv_row[1]
        self.category_id = csv_row[2]
        self.price = csv_row[3]

    def to_key_dict(self):
        return {
            'name': self.name,
            'category_id': self.category_id,
            'price': self.price
        }