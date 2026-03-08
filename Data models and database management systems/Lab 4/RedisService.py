import os
import redis
from DTO import UserDTO, OrderDTO, CategoryDTO, ProductDTO, OrderItemDTO



class RedisService:
    def __init__(self):
        self.host = os.getenv('REDIS_HOST')
        self.port = os.getenv('REDIS_PORT')
        self.client=redis.Redis(host=self.host, port=self.port,
            decode_responses=True,)

    def add_user(self, user:UserDTO):
        self.client.hset(f"user:{user.user_id}", user.to_key_dict())
        pass

    def add_order(self, order:OrderDTO):
        self.client.hset(f"order:{order.order_id}", order.to_key_dict())
        pass

    def add_category(self, category:CategoryDTO):
        self.client.hset(f"category:{category.category_id}", category.to_key_dict())
        pass

    def add_product(self, product:ProductDTO):
        self.client.hset(f"product:{product.product_id}", product.to_key_dict())
        self.client.sadd(f"category:{product.category_id}:products", f"product:{product.product_id}")
        pass

    def add_order_item(self, order_item:OrderItemDTO):
        self.client.hset(f"order_item:{order_item.order_item_id}", order_item.to_key_dict())
        pass
