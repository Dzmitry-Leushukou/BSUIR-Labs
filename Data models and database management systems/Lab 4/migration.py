import csv
import os

import RedisService


def parse_user(filename:str):
    with open(filename, 'r') as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            user = {
                'id': row[0],
                'name': row[1],
                'email': row[2],
            }
            RedisService.add_user(user)

def parse_orders(filename:str):
    pass

def parse_categories(filename:str):
    pass

def parse_order_items(filename:str):
    pass

def parse_products(filename:str):
    pass
