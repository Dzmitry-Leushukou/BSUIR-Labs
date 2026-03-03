from dataclasses import dataclass

@dataclass
class CategoryDTO:
    category_id:str
    name:str
    
    def from_csv(self, csv_row):
        self.category_id = csv_row[0]
        self.name = csv_row[1]


    def to_key_dict(self):
        return {
            'name': self.name
        }