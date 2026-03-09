import os
import redis
from DTO import UserDTO, OrderDTO, CategoryDTO, ProductDTO, OrderItemDTO
import datetime
import time
import csv

class RedisService:
    def __init__(self):
        self.host = os.getenv('REDIS_HOST')
        self.port = os.getenv('REDIS_PORT')
        self.client=redis.Redis(host=self.host, port=self.port,
            decode_responses=True,)

    def add_user(self, user:UserDTO):
        self.client.hset(f"user:{user.user_id}", mapping=user.to_key_dict())
        try:
            dt = datetime.datetime.strptime(user.created_at, "%Y-%m-%d %H:%M:%S")
            timestamp = dt.timestamp()
        except:
            timestamp = time.time()
        self.client.zadd("users:by_registration", {f"user:{user.user_id}": timestamp})

    def add_order(self, order:OrderDTO):
        self.client.hset(f"order:{order.order_id}", mapping=order.to_key_dict())
        user_key = f"user:{order.user_id}"
        self.client.sadd(f"{user_key}:orders_set", f"order:{order.order_id}")
        self.client.lpush(f"{user_key}:orders", f"order:{order.order_id}")
        self.client.ltrim(f"{user_key}:orders", 0, 9)
        self.client.zincrby("users:by_orders_count", 1, user_key)

    def add_category(self, category:CategoryDTO):
        self.client.hset(f"category:{category.category_id}", mapping=category.to_key_dict())

    def add_product(self, product:ProductDTO):
        self.client.hset(f"product:{product.product_id}", mapping=product.to_key_dict())
        self.client.sadd(f"category:{product.category_id}:products", f"product:{product.product_id}")
        self.client.zadd("products:by_price", {f"product:{product.product_id}": float(product.price)})

    def add_order_item(self, order_item:OrderItemDTO):
        self.client.hset(f"order_item:{order_item.order_item_id}", mapping=order_item.to_key_dict())
        order_key = f"order:{order_item.order_id}"
        product_key = f"product:{order_item.product_id}"
        user_id = self.client.hget(order_key, "user_id")

        if not user_id:
            raise ValueError(f"Order {order_item.order_id} not found")

        user_key = f"user:{user_id}"
        category_id = self.client.hget(product_key, "category_id")

        if not category_id:
            raise ValueError(f"Product {order_item.product_id} not found")

        category_key = f"category:{category_id}"
        self.client.rpush(f"{order_key}:items", f"order_item:{order_item.order_item_id}")
        revenue = float(order_item.price) * int(order_item.quantity)
        quantity = int(order_item.quantity)

        self.client.zincrby("products:by_sales", quantity, product_key)
        self.client.zincrby("products:by_revenue", revenue, product_key)
        self.client.zincrby(f"{category_key}:products_by_sales", quantity, product_key)
        self.client.zincrby("users:by_revenue", revenue, user_key)
        self.client.sadd(f"{user_key}:purchased", product_key)

    def load_categories_from_csv(self, filepath: str, has_header: bool = False):
        with open(filepath, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            if has_header:
                next(reader)  
            for row in reader:
                category = CategoryDTO()
                category.from_csv(row)
                self.add_category(category)
        print(f"Categories loaded from {filepath}")

    def load_users_from_csv(self, filepath: str, has_header: bool = False):
        with open(filepath, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            if has_header:
                next(reader)
            for row in reader:
                user = UserDTO()
                user.from_csv(row)
                self.add_user(user)
        print(f"Users loaded from {filepath}")

    def load_products_from_csv(self, filepath: str, has_header: bool = False):
        with open(filepath, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            if has_header:
                next(reader)
            for row in reader:
                product = ProductDTO()
                product.from_csv(row)
                self.add_product(product)
        print(f"Products loaded from {filepath}")

    def load_orders_from_csv(self, filepath: str, has_header: bool = False):
        with open(filepath, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            if has_header:
                next(reader)
            for row in reader:
                order = OrderDTO()
                order.from_csv(row)
                self.add_order(order)
        print(f"Orders loaded from {filepath}")

    def load_order_items_from_csv(self, filepath: str, has_header: bool = False):
        with open(filepath, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            if has_header:
                next(reader)
            for row in reader:
                order_item = OrderItemDTO()
                order_item.from_csv(row)
                self.add_order_item(order_item)
        print(f"Order items loaded from {filepath}")

    def migrate_all(self, categories_file=None, users_file=None, products_file=None,
                    orders_file=None, order_items_file=None, has_header=False):
        if categories_file:
            self.load_categories_from_csv(categories_file, has_header)
        if users_file:
            self.load_users_from_csv(users_file, has_header)
        if products_file:
            self.load_products_from_csv(products_file, has_header)
        if orders_file:
            self.load_orders_from_csv(orders_file, has_header)
        if order_items_file:
            self.load_order_items_from_csv(order_items_file, has_header)

    def flush_db(self):
        self.client.flushdb()
        print("Database flushed.")

    def close(self):
        self.client.close()