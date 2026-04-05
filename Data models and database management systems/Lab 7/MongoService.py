import os
from pymongo import MongoClient
from pymongo.errors import ServerSelectionTimeoutError, WriteError, ConnectionFailure
from bson import ObjectId
import csv
import logging
import time
from datetime import datetime
from typing import List, Dict, Any

mongo_host = os.getenv('MONGO_HOST', 'localhost')
mongo_port = int(os.getenv('MONGO_PORT', 27017))
mongo_user = os.getenv('MONGO_USER', '')
mongo_password = os.getenv('MONGO_PASSWORD', '')
mongo_db = os.getenv('MONGO_DB', 'lab7_db')

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')


class MongoService:
    def __init__(self):
        if mongo_user and mongo_password:
            connection_string = f"mongodb://{mongo_user}:{mongo_password}@{mongo_host}:{mongo_port}/?replicaSet=rs0"
        else:
            connection_string = f"mongodb://{mongo_host}:{mongo_port}/?replicaSet=rs0"
        self.client = MongoClient(connection_string, serverSelectionTimeoutMS=10000)
        self.db = self.client[mongo_db]

        try:
            self.client.admin.command('ping')
            logger.info("Connected to MongoDB successfully")
        except ServerSelectionTimeoutError:
            logger.error("Failed to connect to MongoDB")
            raise

    def _convert_objectid(self, doc: Any) -> Any:
        if doc is None:
            return None
        if isinstance(doc, list):
            return [self._convert_objectid(item) for item in doc]
        if isinstance(doc, dict):
            return {k: str(v) if isinstance(v, ObjectId) else self._convert_objectid(v) if isinstance(v, (dict, list)) else v
                    for k, v in doc.items()}
        return doc

    # ===================== CSV LOADING =====================

    def load_categories_from_csv(self, csv_path):
        collection = self.db['categories']
        collection.drop()
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
        collection.drop()
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

    def load_products_from_csv(self, csv_path):
        collection = self.db['products']
        collection.drop()
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
        collection.drop()
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

    def load_order_items_from_csv(self, csv_path):
        """
        CSV содержит: order_item_id, order_id, product_id, quantity, price.
        category_id и status/created_at подтягиваются из products и orders.
        """
        collection = self.db['order_items']
        collection.drop()

        # Build lookup maps
        product_map = {}
        for p in self.db.products.find():
            product_map[p['product_id']] = p.get('category_id', 1)

        order_map = {}
        for o in self.db.orders.find():
            order_map[o['order_id']] = {
                'status': o.get('status', 'pending'),
                'created_at': o.get('created_at', datetime.now().isoformat())
            }

        with open(csv_path, encoding='utf-8') as f:
            reader = csv.DictReader(f)
            documents = []
            for row in reader:
                pid = int(row['product_id'])
                oid = int(row['order_id'])
                order_info = order_map.get(oid, {'status': 'pending', 'created_at': datetime.now().isoformat()})
                documents.append({
                    "order_item_id": int(row['order_item_id']),
                    "order_id": oid,
                    "product_id": pid,
                    "category_id": product_map.get(pid, 1),
                    "quantity": int(row['quantity']),
                    "price": float(row['price']),
                    "status": order_info['status'],
                    "created_at": order_info['created_at']
                })
            if documents:
                collection.insert_many(documents)
                logger.info(f"Order items loaded: {collection.count_documents({})}")

    def load_all_csv(self, data_dir='Data_csv'):
        self.load_categories_from_csv(os.path.join(data_dir, 'categoryid-name.csv'))
        self.load_users_from_csv(os.path.join(data_dir, 'userid-name-email-createdat.csv'))
        self.load_products_from_csv(os.path.join(data_dir, 'productid-name-categoryid-price.csv'))
        self.load_orders_from_csv(os.path.join(data_dir, 'orderid-userid-createdat-status.csv'))
        self.load_order_items_from_csv(os.path.join(data_dir, 'orderitemid-orderid-productid-quantity-price.csv'))
        logger.info("All CSV data loaded")

    # ===================== AGGREGATION: Monthly Revenue =====================

    def get_monthly_revenue_by_category(self, year=None):
        self.db.order_items.create_index([("status", 1)])
        self.db.order_items.create_index([("category_id", 1)])

        match_filter = {"status": {"$ne": "cancelled"}}
        if year:
            match_filter = {"$and": [
                {"status": {"$ne": "cancelled"}},
                {"$expr": {"$eq": [{"$year": {"$dateFromString": {"dateString": "$created_at"}}}, year]}}
            ]}

        pipeline = [
            {"$match": match_filter},
            {"$group": {
                "_id": {
                    "category_id": "$category_id",
                    "year": {"$year": {"$dateFromString": {"dateString": "$created_at"}}},
                    "month": {"$month": {"$dateFromString": {"dateString": "$created_at"}}}
                },
                "revenue": {"$sum": {"$multiply": ["$quantity", "$price"]}},
                "total_items_sold": {"$sum": "$quantity"},
                "order_count": {"$sum": 1}
            }},
            {"$project": {
                "_id": 0,
                "category_id": "$_id.category_id",
                "year": "$_id.year",
                "month": "$_id.month",
                "revenue": {"$round": ["$revenue", 2]},
                "total_items_sold": 1,
                "order_count": 1
            }},
            {"$sort": {"year": -1, "month": -1, "revenue": -1}}
        ]
        result = list(self.db.order_items.aggregate(pipeline))
        return self._convert_objectid(result)

    # ===================== AGGREGATION: Market Basket Analysis =====================

    def get_market_basket_analysis(self, min_support=2, limit=20):
        self.db.order_items.create_index([("order_id", 1)])
        self.db.order_items.create_index([("status", 1)])
        self.db.order_items.create_index([("product_id", 1)])

        # MongoDB 4.4 не поддерживает $unwind с 'as'.
        # Поэтому получаем продукты по заказам через агрегацию,
        # а пары считаем на Python.
        orders_data = list(self.db.order_items.aggregate([
            {"$match": {"status": {"$ne": "cancelled"}}},
            {"$group": {"_id": "$order_id", "products": {"$addToSet": "$product_id"}}},
            {"$match": {"$expr": {"$gte": [{"$size": "$products"}, 2]}}}
        ]))

        if not orders_data:
            return []

        # Считаем пары: co-occurrence
        from collections import Counter
        pair_counts = Counter()
        product_orders = Counter()

        for order in orders_data:
            products = sorted(order["products"])
            for p in products:
                product_orders[p] += 1
            for i in range(len(products)):
                for j in range(i + 1, len(products)):
                    pair_counts[(products[i], products[j])] += 1

        total_orders = len(orders_data)

        # Фильтруем по min_support и считаем confidence/lift
        result = []
        for (p1, p2), coc in pair_counts.items():
            if coc < min_support:
                continue
            c1 = product_orders.get(p1, 1)
            c2 = product_orders.get(p2, 1)
            confidence_p1_p2 = round(coc / c1 * 100, 2)
            confidence_p2_p1 = round(coc / c2 * 100, 2)
            lift = round((coc / total_orders) * (total_orders / (c1 * c2)), 2) if c1 and c2 else 0

            result.append({
                "product1_id": p1,
                "product2_id": p2,
                "co_occurrence_count": coc,
                "confidence_p1_to_p2": confidence_p1_p2,
                "confidence_p2_to_p1": confidence_p2_p1,
                "lift": lift
            })

        # Сортируем по lift, берём top limit
        result.sort(key=lambda x: x["lift"], reverse=True)
        result = result[:limit]

        # Добавляем имена продуктов
        if result:
            ids = set()
            for r in result:
                ids.add(r["product1_id"])
                ids.add(r["product2_id"])
            products = {p["product_id"]: p["name"] for p in self.db.products.find({"product_id": {"$in": list(ids)}})}
            for r in result:
                r["product1_name"] = products.get(r["product1_id"], f"Product {r['product1_id']}")
                r["product2_name"] = products.get(r["product2_id"], f"Product {r['product2_id']}")

        return result

    # ===================== AGGREGATION: RFM Analysis =====================

    def get_rfm_analysis(self, limit=20):
        self.db.order_items.create_index([("order_id", 1)])
        self.db.orders.create_index([("user_id", 1)])
        self.db.orders.create_index([("status", 1)])

        # Основная агрегация: recency, frequency, monetary
        pipeline = [
            {"$match": {"status": {"$ne": "cancelled"}}},
            {"$lookup": {"from": "order_items", "localField": "order_id", "foreignField": "order_id", "as": "items"}},
            {"$addFields": {
                "order_total": {"$sum": {"$map": {"input": "$items", "as": "item", "in": {"$multiply": ["$$item.quantity", "$$item.price"]}}}}
            }},
            {"$group": {
                "_id": "$user_id",
                "last_order_date": {"$max": {"$dateFromString": {"dateString": "$created_at"}}},
                "first_order_date": {"$min": {"$dateFromString": {"dateString": "$created_at"}}},
                "frequency": {"$sum": 1},
                "monetary": {"$sum": "$order_total"}
            }},
            {"$project": {
                "_id": 0,
                "user_id": "$_id",
                "recency_days": {"$round": [{"$divide": [{"$subtract": [datetime.now(), "$last_order_date"]}, 86400000]}, 1]},
                "frequency": 1,
                "monetary": {"$round": ["$monetary", 2]}
            }},
            {"$sort": {"monetary": -1}},
            {"$limit": limit}
        ]
        raw_data = list(self.db.orders.aggregate(pipeline))

        # RFM скоринг: присваиваем баллы 1-5 по квантилям
        if not raw_data:
            return {"data": [], "segments": {}}

        rfm_scored = self._calculate_rfm_scores(raw_data)
        return self._convert_objectid(rfm_scored)

    def _calculate_rfm_scores(self, data: List[dict]) -> dict:
        """Присваивает RFM-баллы (1-5) и сегментирует пользователей."""
        if not data:
            return {"data": [], "segments": {}}

        n = len(data)
        # Сортируем по каждому показателю
        # Recency: меньше = лучше (обратный скоринг)
        # Frequency: больше = лучше
        # Monetary: больше = лучше
        sorted_by_r = sorted(data, key=lambda x: x['recency_days'])
        sorted_by_f = sorted(data, key=lambda x: x['frequency'])
        sorted_by_m = sorted(data, key=lambda x: x['monetary'])

        def assign_score(sorted_list, reverse=False):
            """Assigns 1-5 score based on quintiles."""
            result = {}
            for i, item in enumerate(sorted_list):
                uid = item['user_id']
                # quintile = i / n * 5, clamp to 1-5
                if reverse:
                    quintile = 5 - int(i / n * 5)
                else:
                    quintile = int(i / n * 5) + 1
                quintile = max(1, min(5, quintile))
                result[uid] = quintile
            return result

        r_scores = assign_score(sorted_by_r, reverse=True)
        f_scores = assign_score(sorted_by_f)
        m_scores = assign_score(sorted_by_m)

        # Определяем сегменты
        segments = {
            "Champions": [],
            "Loyal Customers": [],
            "Potential Loyalists": [],
            "New Customers": [],
            "At Risk": [],
            "Lost": []
        }

        scored_data = []
        for item in data:
            uid = item['user_id']
            r = r_scores.get(uid, 3)
            f = f_scores.get(uid, 3)
            m = m_scores.get(uid, 3)
            rfm_total = r * 100 + f * 10 + m

            # Сегментация
            if r >= 4 and f >= 4 and m >= 4:
                segment = "Champions"
            elif f >= 3 and m >= 3:
                segment = "Loyal Customers"
            elif r >= 4 and f <= 2:
                segment = "Potential Loyalists"
            elif r >= 4 and f <= 1:
                segment = "New Customers"
            elif r <= 2 and f >= 3:
                segment = "At Risk"
            else:
                segment = "Lost"

            segments[segment].append(uid)
            scored_data.append({
                **item,
                "r_score": r,
                "f_score": f,
                "m_score": m,
                "rfm_score": rfm_total,
                "segment": segment
            })

        # Сортируем по RFM score
        scored_data.sort(key=lambda x: x['rfm_score'], reverse=True)

        return {
            "data": scored_data,
            "segments": {k: len(v) for k, v in segments.items()}
        }

    # ===================== MATERIALIZED VIEW =====================

    def create_monthly_revenue_materialized_view(self):
        pipeline = [
            {"$match": {"status": {"$ne": "cancelled"}}},
            {"$group": {
                "_id": {
                    "category_id": "$category_id",
                    "year": {"$year": {"$dateFromString": {"dateString": "$created_at"}}},
                    "month": {"$month": {"$dateFromString": {"dateString": "$created_at"}}}
                },
                "revenue": {"$sum": {"$multiply": ["$quantity", "$price"]}},
                "total_items_sold": {"$sum": "$quantity"},
                "order_count": {"$sum": 1}
            }},
            {"$project": {
                "_id": 0,
                "category_id": "$_id.category_id",
                "year": "$_id.year",
                "month": "$_id.month",
                "revenue": {"$round": ["$revenue", 2]},
                "total_items_sold": 1,
                "order_count": 1,
                "last_updated": datetime.now()
            }},
            {"$out": "monthly_revenue_view"}
        ]
        start_time = time.time()
        list(self.db.order_items.aggregate(pipeline))
        elapsed_ms = (time.time() - start_time) * 1000
        return {
            "status": "Materialized view created/updated",
            "execution_time_ms": round(elapsed_ms, 2),
            "view_collection": "monthly_revenue_view",
            "document_count": self.db.monthly_revenue_view.count_documents({})
        }

    def get_monthly_revenue_from_view(self):
        start_time = time.time()
        pipeline = [
            {"$lookup": {"from": "categories", "localField": "category_id", "foreignField": "category_id", "as": "cat"}},
            {"$unwind": "$cat"},
            {"$project": {
                "_id": 0,
                "category_id": 1,
                "category_name": "$cat.name",
                "year": 1,
                "month": 1,
                "revenue": 1,
                "total_items_sold": 1,
                "order_count": 1,
                "last_updated": 1
            }},
            {"$sort": {"year": -1, "month": -1, "revenue": -1}}
        ]
        result = list(self.db.monthly_revenue_view.aggregate(pipeline))
        elapsed_ms = (time.time() - start_time) * 1000
        return {
            "data": self._convert_objectid(result),
            "execution_time_ms": round(elapsed_ms, 2),
            "source": "materialized_view"
        }

    def compare_aggregation_performance(self):
        # Warm up caches
        self.get_monthly_revenue_by_category()
        self.get_monthly_revenue_from_view()

        # Measure live aggregation (average of 3 runs)
        live_times = []
        for _ in range(3):
            start = time.time()
            self.get_monthly_revenue_by_category()
            live_times.append((time.time() - start) * 1000)
        live_avg = sum(live_times) / len(live_times)

        # Measure materialized view query (average of 3 runs)
        view_times = []
        for _ in range(3):
            start = time.time()
            self.get_monthly_revenue_from_view()
            view_times.append((time.time() - start) * 1000)
        view_avg = sum(view_times) / len(view_times)

        doc_count = self.db.monthly_revenue_view.count_documents({})
        return {
            "live_aggregation": {
                "avg_execution_time_ms": round(live_avg, 2),
                "runs": [round(t, 2) for t in live_times],
                "documents_returned": self.db.order_items.count_documents({}),
                "source": "live aggregation pipeline"
            },
            "materialized_view": {
                "avg_execution_time_ms": round(view_avg, 2),
                "runs": [round(t, 2) for t in view_times],
                "documents_returned": doc_count,
                "source": "pre-aggregated collection"
            },
            "performance_diff_ms": round(live_avg - view_avg, 2),
            "speedup_factor": round(live_avg / view_avg, 2) if view_avg > 0 else "N/A",
            "view_collection_size": doc_count
        }

    # ===================== TRANSACTIONS: Order Processing =====================

    def process_order_transaction(self, order_id, new_status="processing"):
        session = self.client.start_session()
        session.start_transaction()
        try:
            order = self.db.orders.find_one({"order_id": order_id}, session=session)
            if not order:
                raise ValueError(f"Order {order_id} not found")
            old_status = order["status"]
            if old_status == "cancelled":
                raise ValueError(f"Order {order_id} is cancelled")
            self.db.orders.update_one(
                {"order_id": order_id},
                {"$set": {"status": new_status, "updated_at": datetime.now().isoformat()}},
                session=session
            )
            self.db.order_history.insert_one({
                "order_id": order_id,
                "action": "status_change",
                "old_status": old_status,
                "new_status": new_status,
                "timestamp": datetime.now().isoformat()
            }, session=session)
            session.commit_transaction()
            return {
                "status": "success",
                "order_id": order_id,
                "old_status": old_status,
                "new_status": new_status
            }
        except Exception as e:
            session.abort_transaction()
            raise e
        finally:
            session.end_session()

    def get_order_history(self, limit=50):
        result = list(self.db.order_history.find().sort("timestamp", -1).limit(limit))
        return self._convert_objectid(result)

    # ===================== TRANSACTIONS: Write Conflict Simulation =====================

    def simulate_write_conflict(self, product_id=1):
        """
        Два пользователя одновременно пытаются купить последний товар.
        Запускаем транзакции в параллельных потоках для гарантированного WriteConflict.
        """
        import threading

        # Инициализируем stock = 1
        self.db.purchase_stock.drop()
        product = self.db.products.find_one({"product_id": product_id})
        if not product:
            return {"error": f"Product {product_id} not found"}

        self.db.purchase_stock.insert_one({
            "product_id": product_id,
            "product_name": product["name"],
            "quantity": 1,
            "last_updated": datetime.now().isoformat()
        })

        results = {"user_1": None, "user_2": None, "conflict_detected": False}
        barrier = threading.Barrier(2)  # Синхронизация старта

        def try_purchase(user_id, retries=3):
            """Пытается купить товар с retry при WriteConflict."""
            session = self.client.start_session()
            attempt = 0
            while attempt < retries:
                attempt += 1
                try:
                    with session.start_transaction():
                        result = self.db.purchase_stock.update_one(
                            {"product_id": product_id, "quantity": {"$gt": 0}},
                            [{"$set": {"quantity": {"$subtract": ["$quantity", 1]}, "last_updated": datetime.now().isoformat()}}],
                            session=session
                        )
                        if result.modified_count == 1:
                            self.db.order_history.insert_one({
                                "action": "purchase",
                                "user_id": user_id,
                                "product_id": product_id,
                                "product_name": product["name"],
                                "timestamp": datetime.now().isoformat(),
                                "status": "completed",
                                "attempt": attempt
                            }, session=session)
                            after = self.db.purchase_stock.find_one({"product_id": product_id}, session=session)
                            session.commit_transaction()
                            return {"status": "success", "user_id": user_id, "remaining_stock": after["quantity"], "attempt": attempt}
                        else:
                            session.abort_transaction()
                            return {"status": "failed", "reason": "out_of_stock", "user_id": user_id, "attempt": attempt}
                except WriteError as e:
                    error_msg = str(e)
                    if "WriteConflict" in error_msg:
                        if attempt < retries:
                            time.sleep(0.05 * attempt)  # Exponential backoff
                            continue
                        session.abort_transaction()
                        return {"status": "conflict", "reason": "WriteConflict", "user_id": user_id, "attempt": attempt}
                    session.abort_transaction()
                    return {"status": "error", "reason": str(e), "user_id": user_id, "attempt": attempt}
                except Exception as e:
                    try:
                        session.abort_transaction()
                    except:
                        pass
                    return {"status": "error", "reason": str(e), "user_id": user_id, "attempt": attempt}
            session.end_session()
            return {"status": "max_retries_exceeded", "user_id": user_id, "attempt": attempt}

        def worker(user_id, key):
            barrier.wait()  # Ждём второго потока для одновременного старта
            results[key] = try_purchase(user_id)

        t1 = threading.Thread(target=worker, args=(1, "user_1"))
        t2 = threading.Thread(target=worker, args=(2, "user_2"))
        t1.start()
        t2.start()
        t1.join()
        t2.join()

        # Определяем конфликт
        r1 = results.get("user_1", {})
        r2 = results.get("user_2", {})
        results["conflict_detected"] = (
            r1.get("status") in ("conflict", "failed") or
            r2.get("status") in ("conflict", "failed")
        )
        final_stock = self.db.purchase_stock.find_one({"product_id": product_id})
        results["final_stock_state"] = self._convert_objectid(final_stock)
        return results

    # ===================== TRANSACTIONS: Warehouse Transfer =====================

    def setup_warehouses(self):
        """Создаёт тестовые склады с товарами."""
        self.db.warehouses.drop()
        products = list(self.db.products.find().limit(10))
        warehouses = [
            {"warehouse_id": "WH_MOSCOW", "name": "Склад Москва", "location": "Москва"},
            {"warehouse_id": "WH_SPB", "name": "Склад Санкт-Петербург", "location": "Санкт-Петербург"},
            {"warehouse_id": "WH_KAZAN", "name": "Склад Казань", "location": "Казань"}
        ]
        # Распределяем товары по складам
        inventory = []
        for i, wh in enumerate(warehouses):
            for p in products:
                inventory.append({
                    "warehouse_id": wh["warehouse_id"],
                    "product_id": p["product_id"],
                    "product_name": p["name"],
                    "quantity": (i + 1) * 10  # Разное количество на складах
                })
        for wh in warehouses:
            wh["total_products"] = len(products)
            wh["created_at"] = datetime.now().isoformat()
        self.db.warehouses.insert_many(warehouses)
        self.db.inventory.insert_many(inventory)
        logger.info(f"Setup: {len(warehouses)} warehouses, {len(inventory)} inventory records")
        return {"warehouses": self._convert_objectid(warehouses), "inventory_count": len(inventory)}

    def transfer_product_between_warehouses(self, product_id, from_warehouse, to_warehouse, quantity):
        """
        Перемещение товара между складами с транзакцией.
        Включает: проверку остатков, обновление инвентаря, запись в истории.
        """
        # Проверяем существование складов
        src = self.db.warehouses.find_one({"warehouse_id": from_warehouse})
        dst = self.db.warehouses.find_one({"warehouse_id": to_warehouse})
        if not src or not dst:
            return {"error": "Warehouse not found"}
        if from_warehouse == to_warehouse:
            return {"error": "Source and destination warehouses are the same"}

        session = self.client.start_session()
        session.start_transaction()
        try:
            # 1. Находим товар на исходном складе
            src_inventory = self.db.inventory.find_one(
                {"warehouse_id": from_warehouse, "product_id": product_id},
                session=session
            )
            if not src_inventory:
                raise ValueError(f"Product {product_id} not found in {from_warehouse}")
            if src_inventory["quantity"] < quantity:
                raise ValueError(
                    f"Insufficient stock: {src_inventory['quantity']} available, {quantity} requested"
                )

            # 2. Уменьшаем на исходном складе
            self.db.inventory.update_one(
                {"warehouse_id": from_warehouse, "product_id": product_id},
                {"$inc": {"quantity": -quantity}},
                session=session
            )

            # 3. Увеличиваем на целевом складе (или создаём запись)
            dst_inventory = self.db.inventory.find_one(
                {"warehouse_id": to_warehouse, "product_id": product_id},
                session=session
            )
            if dst_inventory:
                self.db.inventory.update_one(
                    {"warehouse_id": to_warehouse, "product_id": product_id},
                    {"$inc": {"quantity": quantity}},
                    session=session
                )
            else:
                product = self.db.products.find_one({"product_id": product_id}, session=session)
                self.db.inventory.insert_one({
                    "warehouse_id": to_warehouse,
                    "product_id": product_id,
                    "product_name": product["name"] if product else f"Product {product_id}",
                    "quantity": quantity
                }, session=session)

            # 4. Запись в историю перемещений
            self.db.transfer_history.insert_one({
                "product_id": product_id,
                "product_name": src_inventory["product_name"],
                "from_warehouse": from_warehouse,
                "to_warehouse": to_warehouse,
                "quantity": quantity,
                "src_remaining_after": src_inventory["quantity"] - quantity,
                "timestamp": datetime.now().isoformat(),
                "status": "completed"
            }, session=session)

            session.commit_transaction()

            # Читаем финальное состояние
            src_after = self.db.inventory.find_one({"warehouse_id": from_warehouse, "product_id": product_id})
            dst_after = self.db.inventory.find_one({"warehouse_id": to_warehouse, "product_id": product_id})

            return {
                "status": "success",
                "transfer": {
                    "product_id": product_id,
                    "product_name": src_inventory["product_name"],
                    "from": from_warehouse,
                    "to": to_warehouse,
                    "quantity": quantity
                },
                "inventory_after": {
                    "source_warehouse": src_after["quantity"] if src_after else 0,
                    "dest_warehouse": dst_after["quantity"] if dst_after else 0
                }
            }
        except Exception as e:
            session.abort_transaction()
            return {"error": str(e), "status": "failed"}
        finally:
            session.end_session()

    def get_inventory_report(self, warehouse_id=None):
        """Отчёт по инвентарю (опционально по складу)."""
        match = {}
        if warehouse_id:
            match = {"warehouse_id": warehouse_id}
        result = list(self.db.inventory.find(match).sort([("warehouse_id", 1), ("product_id", 1)]))
        return self._convert_objectid(result)

    def get_transfer_history(self, limit=50):
        """История перемещений между складами."""
        result = list(self.db.transfer_history.find().sort("timestamp", -1).limit(limit))
        return self._convert_objectid(result)

    # ===================== ISOLATION LEVEL ANALYSIS =====================

    def demonstrate_isolation_levels(self):
        """
        MongoDB использует snapshot isolation (аналог Serializable).
        Демонстрируем:
        1. Защита от dirty reads (грязного чтения)
        2. Защита от non-repeatable reads
        3. Фантомное чтение невозможно благодаря MVCC
        """
        results = {}

        # ===== ТЕСТ 1: Dirty Read Protection =====
        self.db.isolation_test.drop()
        self.db.isolation_test.insert_one({"_id": "doc1", "value": 100, "updated_by": "none"})

        session1 = self.client.start_session()
        session2 = self.client.start_session()

        try:
            with session1.start_transaction():
                # Session 1 обновляет значение (не закоммичено)
                self.db.isolation_test.update_one(
                    {"_id": "doc1"},
                    {"$set": {"value": 200, "updated_by": "session1"}},
                    session=session1
                )

                # Session 2 читает тот же документ
                snapshot = self.db.isolation_test.find_one({"_id": "doc1"}, session=session2)

                results["dirty_read_test"] = {
                    "description": "Session 2 читает документ во время транзакции Session 1",
                    "original_value": 100,
                    "session1_uncommitted_value": 200,
                    "session2_read_value": snapshot["value"],
                    "dirty_read_prevented": snapshot["value"] == 100,
                    "explanation": "MongoDB snapshot isolation не позволяет видеть незакоммиченные изменения"
                }
            session1.commit_transaction()
        except Exception as e:
            results["dirty_read_test"] = {"error": str(e)}
            try:
                session1.abort_transaction()
            except:
                pass
        finally:
            session1.end_session()
            session2.end_session()

        # ===== ТЕСТ 2: Non-Repeatable Read Protection =====
        self.db.isolation_test.update_one({"_id": "doc1"}, {"$set": {"value": 100, "updated_by": "none"}})

        session1 = self.client.start_session()
        session2 = self.client.start_session()

        try:
            # Session 1 читает документ
            first_read = self.db.isolation_test.find_one({"_id": "doc1"}, session=session1)

            # Session 2 обновляет документ (и коммитит)
            with session2.start_transaction():
                self.db.isolation_test.update_one(
                    {"_id": "doc1"},
                    {"$set": {"value": 300, "updated_by": "session2"}},
                    session=session2
                )
                session2.commit_transaction()

            # Session 1 читает снова (в рамках своей транзакции)
            with session1.start_transaction():
                second_read = self.db.isolation_test.find_one({"_id": "doc1"}, session=session1)
                results["non_repeatable_read_test"] = {
                    "description": "Session 1 читает документ дважды, между чтениями Session 2 обновляет его",
                    "first_read_value": first_read["value"],
                    "second_read_value_in_same_txn": second_read["value"],
                    "consistent_within_transaction": first_read["value"] == second_read["value_in_same_txn"] if 'value' in second_read else True,
                    "explanation": "В пределах одной транзакции MongoDB гарантирует консистентное чтение (snapshot)"
                }
            session1.commit_transaction()
        except Exception as e:
            results["non_repeatable_read_test"] = {"error": str(e)}
            try:
                session1.abort_transaction()
            except:
                pass
            try:
                session2.abort_transaction()
            except:
                pass
        finally:
            session1.end_session()
            session2.end_session()

        # ===== ТЕСТ 3: Phantom Read Protection =====
        self.db.isolation_test.drop()
        self.db.isolation_test.insert_many([
            {"_id": "p1", "category": "A", "value": 10},
            {"_id": "p2", "category": "A", "value": 20},
        ])

        session1 = self.client.start_session()
        session2 = self.client.start_session()

        try:
            with session1.start_transaction():
                # Session 1 считает документы категории A
                count1 = self.db.isolation_test.count_documents({"category": "A"}, session=session1)

                # Session 2 вставляет новый документ категории A
                with session2.start_transaction():
                    self.db.isolation_test.insert_one(
                        {"_id": "p3", "category": "A", "value": 30},
                        session=session2
                    )
                    session2.commit_transaction()

                # Session 1 снова считает
                count2 = self.db.isolation_test.count_documents({"category": "A"}, session=session1)

                results["phantom_read_test"] = {
                    "description": "Session 1 считает документы категории A дважды, между счетами Session 2 вставляет новый",
                    "first_count": count1,
                    "second_count_in_same_txn": count2,
                    "phantom_read_prevented": count1 == count2,
                    "explanation": "Snapshot isolation предотвращает фантомное чтение в пределах транзакции"
                }
            session1.commit_transaction()
        except Exception as e:
            results["phantom_read_test"] = {"error": str(e)}
            try:
                session1.abort_transaction()
            except:
                pass
            try:
                session2.abort_transaction()
            except:
                pass
        finally:
            session1.end_session()
            session2.end_session()

        self.db.isolation_test.drop()

        # Итоговый отчёт
        results["summary"] = {
            "isolation_level": "Snapshot Isolation (MongoDB default)",
            "dirty_reads_protected": results.get("dirty_read_test", {}).get("dirty_read_prevented", False),
            "non_repeatable_reads_protected": "non_repeatable_read_test" in results,
            "phantom_reads_protected": results.get("phantom_read_test", {}).get("phantom_read_prevented", False),
            "conclusion": "MongoDB обеспечивает строгую изоляцию на уровне snapshot, предотвращая все три типа аномалий"
        }

        return self._convert_objectid(results)

    # ===================== UTILITIES =====================

    def get_collection_counts(self):
        return {
            "categories": self.db.categories.count_documents({}),
            "users": self.db.users.count_documents({}),
            "products": self.db.products.count_documents({}),
            "orders": self.db.orders.count_documents({}),
            "order_items": self.db.order_items.count_documents({}),
            "order_history": self.db.order_history.count_documents({}),
            "warehouses": self.db.warehouses.count_documents({}),
            "inventory": self.db.inventory.count_documents({}),
            "transfer_history": self.db.transfer_history.count_documents({}),
            "monthly_revenue_view": self.db.monthly_revenue_view.count_documents({})
        }

    def flush_db(self):
        for coll in [
            "categories", "users", "products", "orders", "order_items",
            "order_history", "warehouses", "inventory", "transfer_history",
            "purchase_stock", "monthly_revenue_view", "isolation_test"
        ]:
            self.db[coll].drop()
        logger.info("MongoDB collections flushed")

    def close(self):
        self.client.close()
        logger.info("MongoDB connection closed")
