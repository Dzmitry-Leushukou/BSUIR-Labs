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

    def get_collection_counts(self):
        return {
            "categories": self.db.categories.count_documents({}),
            "users": self.db.users.count_documents({}),
            "products": self.db.products.count_documents({}),
            "orders": self.db.orders.count_documents({}),
            "order_items": self.db.order_items.count_documents({})
        }

    def get_invalid_user_orders(self):
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

    def get_orders_with_sum_gt_1000(self):
        pipeline = [
            {
                "$group": {
                    "_id": "$order_id",
                    "total": { "$sum": { "$multiply": ["$quantity", "$price"] } }
                }
            },
            { "$match": { "total": { "$gt": 1000 } } },
            {
                "$lookup": {
                    "from": "orders",
                    "localField": "_id",
                    "foreignField": "order_id",
                    "as": "order_info"
                }
            },
            { "$unwind": "$order_info" },
            {
                "$project": {
                    "_id": 0,
                    "order_id": "$_id",
                    "total": 1,
                    "user_id": "$order_info.user_id",
                    "created_at": "$order_info.created_at",
                    "status": "$order_info.status"
                }
            },
            { "$sort": { "total": -1 } }
        ]
        result = list(self.db.order_items.aggregate(pipeline))
        return [self._convert_objectid(doc) for doc in result]

    def get_top_users_by_orders_count(self, limit=10):
        pipeline = [
            {
                "$group": {
                    "_id": "$user_id",
                    "order_count": { "$sum": 1 }
                }
            },
            { "$sort": { "order_count": -1 } },
            { "$limit": limit },
            {
                "$lookup": {
                    "from": "users",
                    "localField": "_id",
                    "foreignField": "user_id",
                    "as": "user_info"
                }
            },
            { "$unwind": "$user_info" },
            {
                "$project": {
                    "_id": 0,
                    "user_id": "$_id",
                    "user_name": "$user_info.name",
                    "user_email": "$user_info.email",
                    "order_count": 1
                }
            }
        ]
        result = list(self.db.orders.aggregate(pipeline))
        return [self._convert_objectid(doc) for doc in result]

    def get_top_categories_by_revenue(self, limit=10):
        pipeline = [
            {
                "$lookup": {
                    "from": "products",
                    "localField": "product_id",
                    "foreignField": "product_id",
                    "as": "product"
                }
            },
            { "$unwind": "$product" },
            {
                "$group": {
                    "_id": "$product.category_id",
                    "revenue": { "$sum": { "$multiply": ["$quantity", "$price"] } },
                    "total_quantity": { "$sum": "$quantity" }
                }
            },
            {
                "$lookup": {
                    "from": "categories",
                    "localField": "_id",
                    "foreignField": "category_id",
                    "as": "category_info"
                }
            },
            { "$unwind": "$category_info" },
            {
                "$project": {
                    "_id": 0,
                    "category_id": "$_id",
                    "category_name": "$category_info.name",
                    "revenue": 1,
                    "total_quantity": 1
                }
            },
            { "$sort": { "revenue": -1 } },
            { "$limit": limit }
        ]
        result = list(self.db.order_items.aggregate(pipeline))
        return [self._convert_objectid(doc) for doc in result]

    def get_order_statistics(self):
        pipeline = [
            {
                "$group": {
                    "_id": "$order_id",
                    "total": { "$sum": { "$multiply": ["$quantity", "$price"] } }
                }
            },
            {
                "$group": {
                    "_id": None,
                    "avg_order_total": { "$avg": "$total" },
                    "min_order_total": { "$min": "$total" },
                    "max_order_total": { "$max": "$total" },
                    "order_count": { "$sum": 1 }
                }
            },
            {
                "$project": {
                    "_id": 0,
                    "avg_order_total": 1,
                    "min_order_total": 1,
                    "max_order_total": 1,
                    "order_count": 1
                }
            }
        ]
        result = list(self.db.order_items.aggregate(pipeline))
        if result:
            return self._convert_objectid(result[0])
        return {}

    def get_all_indexes(self):
        indexes = {}
        for collection_name in ["categories", "users", "products", "orders", "order_items"]:
            collection = self.db[collection_name]
            indexes[collection_name] = list(collection.list_indexes())
        return indexes

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
        explain_result = self.db.command("explain", {
            "find": "orders",
            "filter": {"user_id": user_id}
        })

        execution_stats = explain_result.get("executionStats", {})
        return self._convert_objectid({
            "total_documents_examined": execution_stats.get("totalDocsExamined", 0),
            "total_documents_returned": execution_stats.get("nReturned", 0),
            "execution_time_ms": execution_stats.get("executionTimeMillis", 0),
            "execution_stages": execution_stats.get("executionStages", {}).get("stage", "UNKNOWN"),
            "is_index_used": "IXSCAN" in str(explain_result.get("queryPlanner", {}).get("winningPlan", {})),
        })

    def execute_query_and_measure_time(self, user_id):
        import time
        start = time.time()
        result = list(self.db.orders.find({"user_id": user_id}))
        elapsed_ms = (time.time() - start) * 1000
        return elapsed_ms, len(result)

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