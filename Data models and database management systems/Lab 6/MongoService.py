import os
from pymongo import MongoClient
from pymongo.errors import ServerSelectionTimeoutError
from bson import ObjectId
import csv
import logging
from typing import List, Dict, Any

mongo_host = os.getenv('MONGO_HOST', 'localhost')
mongo_port = int(os.getenv('MONGO_PORT', 27017))
mongo_user = os.getenv('MONGO_USER', 'admin')
mongo_password = os.getenv('MONGO_PASSWORD', 'password')
mongo_db = os.getenv('MONGO_DB', 'lab6_db')

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger.setLevel(logging.INFO)
logger.info("Starting MongoService")


class MongoService:
    def __init__(self):
        connection_string = f"mongodb://{mongo_user}:{mongo_password}@{mongo_host}:{mongo_port}/"
        self.client = MongoClient(connection_string, serverSelectionTimeoutMS=5000)
        self.db = self.client[mongo_db]
        
        try:
            self.client.admin.command('ping')
            logger.info("Connected to MongoDB successfully")
        except ServerSelectionTimeoutError:
            logger.error("Failed to connect to MongoDB")
            raise

    def _convert_objectid(self, doc: Dict[str, Any]) -> Dict[str, Any]:
        """Convert ObjectId to string for JSON serialization"""
        if doc is None:
            return None
        if isinstance(doc, list):
            return [self._convert_objectid(item) for item in doc]
        if isinstance(doc, dict):
            return {k: str(v) if isinstance(v, ObjectId) else self._convert_objectid(v) if isinstance(v, (dict, list)) else v 
                    for k, v in doc.items()}
        return doc

    def load_categories_from_csv(self, csv_path):
        collection = self.db['categories']
        
        with open(csv_path, encoding='utf-8') as f:
            reader = csv.DictReader(f)
            documents = []
            for row in reader:
                documents.append({
                    "category_id": int(row['category_id']),
                    "name": row['name']
                })
            
            if documents:
                collection.insert_many(documents)
                logger.info(f"Categories loaded: {collection.count_documents({})}")


    def load_users_from_csv(self, csv_path):
        collection = self.db['users']
        
        with open(csv_path, encoding='utf-8') as f:
            reader = csv.DictReader(f)
            documents = []
            for row in reader:
                documents.append({
                    "user_id": int(row['user_id']),
                    "name": row['name'],
                    "email": row['email'],
                    "created_at": row['created_at']
                })
            
            if documents:
                collection.insert_many(documents)
                logger.info(f"Users loaded: {collection.count_documents({})}")

    def load_order_items_from_csv(self, csv_path):
        collection = self.db['order_items']
        
        with open(csv_path, encoding='utf-8') as f:
            reader = csv.DictReader(f)
            documents = []
            for row in reader:
                documents.append({
                    "order_item_id": int(row['order_item_id']),
                    "order_id": int(row['order_id']),
                    "product_id": int(row['product_id']),
                    "quantity": int(row['quantity']),
                    "price": float(row['price'])
                })
            
            if documents:
                collection.insert_many(documents)
                logger.info(f"Order items loaded: {collection.count_documents({})}")

    def load_products_from_csv(self, csv_path):
        collection = self.db['products']
        
        with open(csv_path, encoding='utf-8') as f:
            reader = csv.DictReader(f)
            documents = []
            for row in reader:
                documents.append({
                    "product_id": int(row['product_id']),
                    "name": row['name'],
                    "category_id": int(row['category_id']),
                    "price": float(row['price'])
                })
            
            if documents:
                collection.insert_many(documents)
                logger.info(f"Products loaded: {collection.count_documents({})}")

    def load_orders_from_csv(self, csv_path):
        collection = self.db['orders']
        
        with open(csv_path, encoding='utf-8') as f:
            reader = csv.DictReader(f)
            documents = []
            for row in reader:
                documents.append({
                    "order_id": int(row['order_id']),
                    "user_id": int(row['user_id']),
                    "created_at": row['created_at'],
                    "status": row['status']
                })
            
            if documents:
                collection.insert_many(documents)
                logger.info(f"Orders loaded: {collection.count_documents({})}")


    def migrate_data(self, categories_csv, users_csv, products_csv, orders_csv, order_items_csv):
        self.load_categories_from_csv(categories_csv)
        self.load_users_from_csv(users_csv)
        self.load_products_from_csv(products_csv)
        self.load_orders_from_csv(orders_csv)
        self.load_order_items_from_csv(order_items_csv)

    def get_collection_size(self, collection_name):
        collection = self.db[collection_name]
        return collection.count_documents({})

    def check_userId_validity_in_orders(self):
        pipeline = [
            {
                "$lookup": {
                    "from": "users",
                    "localField": "user_id",
                    "foreignField": "user_id", 
                    "as": "user"
                }
            },
            { "$match": { "user": [] } } 
        ]
        result = list(self.db.orders.aggregate(pipeline))
        return [self._convert_objectid(doc) for doc in result]
        
    def get_orders_total_gt_1000(self):
        pipeline = [
            {
                "$group": {
                    "_id": "$order_id",  
                    "total": {
                        "$sum": { "$multiply": [ "$quantity", "$price" ] }  
                    }
                }
            },
            { "$match": { "total": { "$gt": 1000 } } },
            { "$sort": { "total": -1 } } 
        ]
        result = list(self.db.order_items.aggregate(pipeline))
        return [self._convert_objectid(doc) for doc in result]

    def get_user_orders(self, user_id):
        result = list(self.db.orders.find({"user_id": user_id}))
        return [self._convert_objectid(doc) for doc in result]

    def get_order_with_details(self, order_id):
        pipeline = [
            { "$match": { "order_id": order_id } },
            {
                "$lookup": {
                    "from": "order_items",
                    "localField": "order_id",
                    "foreignField": "order_id",
                    "as": "items"
                }
            },
            {
                "$unwind": "$items"
            },
            {
                "$lookup": {
                    "from": "products",
                    "localField": "items.product_id",
                    "foreignField": "product_id",
                    "as": "product"
                }
            },
            {
                "$unwind": "$product"
            },
            {
                "$project": {
                    "order_id": 1,
                    "user_id": 1,
                    "created_at": 1,
                    "status": 1,
                    "product_id": "$items.product_id",
                    "product_name": "$product.name",
                    "quantity": "$items.quantity",
                    "price": "$items.price",
                    "total_price": { "$multiply": ["$items.quantity", "$items.price"] }
                }
            }
        ]
        result = list(self.db.orders.aggregate(pipeline))
        return [self._convert_objectid(doc) for doc in result]

    def get_top_products_by_revenue(self, limit=10):
        pipeline = [
            {
                "$group": {
                    "_id": "$product_id",
                    "revenue": {
                        "$sum": { "$multiply": ["$quantity", "$price"] }
                    },
                    "total_quantity": { "$sum": "$quantity" }
                }
            },
            {
                "$lookup": {
                    "from": "products",
                    "localField": "_id",
                    "foreignField": "product_id",
                    "as": "product"
                }
            },
            {
                "$unwind": "$product"
            },
            {
                "$project": {
                    "_id": 0,
                    "product_id": "$_id",
                    "product_name": "$product.name",
                    "revenue": 1,
                    "total_quantity": 1
                }
            },
            { "$sort": { "revenue": -1 } },
            { "$limit": limit }
        ]
        result = list(self.db.order_items.aggregate(pipeline))
        return [self._convert_objectid(doc) for doc in result]

    def create_indexes(self):
        try:
            self.db.orders.create_index([("user_id", 1), ("created_at", -1)])
            logger.info("Index on orders(user_id, created_at) created")
            
            self.db.order_items.create_index([("order_id", 1)])
            logger.info("Index on order_items(order_id) created")
            
            self.db.order_items.create_index([("product_id", 1)])
            logger.info("Index on order_items(product_id) created")
            
            self.db.orders.create_index([("user_id", 1)])
            logger.info("Index on orders(user_id) created")
            
        except Exception as e:
            logger.error(f"Error creating indexes: {e}")

    def drop_indexes(self):
        try:
            self.db.orders.drop_indexes()
            self.db.order_items.drop_indexes()
            logger.info("All indexes dropped")
        except Exception as e:
            logger.error(f"Error dropping indexes: {e}")

    def get_query_performance(self, user_id):
        query = self.db.orders.find({"user_id": user_id})
        explain_result = query.explain()
        
        execution_stats = explain_result.get("executionStats", {})
        return self._convert_objectid({
            "total_documents_examined": execution_stats.get("totalDocsExamined", 0),
            "total_documents_returned": execution_stats.get("nReturned", 0),
            "execution_stages": execution_stats.get("executionStages", {}).get("stage", "UNKNOWN"),
            "is_index_used": "COLLSCAN" not in str(execution_stats.get("executionStages", {}).get("stage", "")),
        })


    def flush_db(self):
        self.db.categories.drop()
        self.db.users.drop()
        self.db.products.drop()
        self.db.orders.drop()
        self.db.order_items.drop()
        logger.info("MongoDB collections flushed")

    def close(self):
        self.client.close()
        logger.info("MongoDB connection closed")