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
        self.client.set(user.user_id, user.to_key_dict())
        pass

    def add_order(self, order:OrderDTO):
        self.client.set(order.order_id, order.to_key_dict())
        pass

    def add_category(self, category:CategoryDTO):
        self.client.set(category.category_id, category.to_key_dict())
        pass

    def add_product(self, product:ProductDTO):
        self.client.set(product.product_id, product.to_key_dict())
        pass

    def add_order_item(self, order_item:OrderItemDTO):
        self.client.set(order_item.order_item_id, order_item.to_key_dict())
        pass
