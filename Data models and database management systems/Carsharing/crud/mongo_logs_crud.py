from pymongo import MongoClient, ASCENDING, DESCENDING
from pymongo.errors import PyMongoError
from datetime import datetime, timedelta
from typing import List, Optional, Dict, Any
import os

from mongo_client import get_mongo_db
from schemas import ActionLogMongo, DbQueryLogMongo, ErrorLogMongo, LogFilter


ACTION_LOGS_COLLECTION = "action_logs"
DB_QUERY_LOGS_COLLECTION = "db_query_logs"
ERROR_LOGS_COLLECTION = "error_logs"


def get_collection(collection_name: str):
    db = get_mongo_db()
    return db[collection_name]


def setup_ttl_indexes():
    db = get_mongo_db()

    action_logs_ttl_days = int(os.getenv("ACTION_LOGS_TTL_DAYS", 90))
    action_logs_ttl_seconds = action_logs_ttl_days * 24 * 60 * 60

    db_query_logs_ttl_days = int(os.getenv("DB_QUERY_LOGS_TTL_DAYS", 30))
    db_query_logs_ttl_seconds = db_query_logs_ttl_days * 24 * 60 * 60

    error_logs_ttl_days = int(os.getenv("ERROR_LOGS_TTL_DAYS", 60))
    error_logs_ttl_seconds = error_logs_ttl_days * 24 * 60 * 60

    action_logs_collection = db[ACTION_LOGS_COLLECTION]
    action_logs_collection.create_index(
        [("created_at", ASCENDING)],
        expireAfterSeconds=action_logs_ttl_seconds,
        name="action_logs_ttl_idx"
    )

    db_query_logs_collection = db[DB_QUERY_LOGS_COLLECTION]
    db_query_logs_collection.create_index(
        [("created_at", ASCENDING)],
        expireAfterSeconds=db_query_logs_ttl_seconds,
        name="db_query_logs_ttl_idx"
    )

    error_logs_collection = db[ERROR_LOGS_COLLECTION]
    error_logs_collection.create_index(
        [("created_at", ASCENDING)],
        expireAfterSeconds=error_logs_ttl_seconds,
        name="error_logs_ttl_idx"
    )

    action_logs_collection.create_index(
        [("actor_user_id", ASCENDING), ("created_at", DESCENDING)],
        name="action_logs_actor_idx"
    )
    action_logs_collection.create_index(
        [("action_type", ASCENDING), ("created_at", DESCENDING)],
        name="action_logs_type_idx"
    )
    action_logs_collection.create_index(
        [("target_user_id", ASCENDING), ("created_at", DESCENDING)],
        name="action_logs_target_user_idx"
    )
    action_logs_collection.create_index(
        [("target_car_id", ASCENDING), ("created_at", DESCENDING)],
        name="action_logs_target_car_idx"
    )

    db_query_logs_collection.create_index(
        [("table_name", ASCENDING), ("created_at", DESCENDING)],
        name="db_query_logs_table_idx"
    )
    db_query_logs_collection.create_index(
        [("query_type", ASCENDING), ("created_at", DESCENDING)],
        name="db_query_logs_type_idx"
    )
    db_query_logs_collection.create_index(
        [("user_id", ASCENDING), ("created_at", DESCENDING)],
        name="db_query_logs_user_idx"
    )

    error_logs_collection.create_index(
        [("error_type", ASCENDING), ("created_at", DESCENDING)],
        name="error_logs_type_idx"
    )
    error_logs_collection.create_index(
        [("severity", ASCENDING), ("created_at", DESCENDING)],
        name="error_logs_severity_idx"
    )
    error_logs_collection.create_index(
        [("user_id", ASCENDING), ("created_at", DESCENDING)],
        name="error_logs_user_idx"
    )

    print(f"TTL indexes created: action_logs={action_logs_ttl_days}d, db_query_logs={db_query_logs_ttl_days}d, error_logs={error_logs_ttl_days}d")


def create_action_log_mongo(
    actor_user_id: int,
    action_type: str,
    description: Optional[str] = None,
    target_user_id: Optional[int] = None,
    target_car_id: Optional[int] = None,
    target_rental_id: Optional[int] = None,
    old_values: Optional[Dict[str, Any]] = None,
    new_values: Optional[Dict[str, Any]] = None,
    user_agent: Optional[str] = None,
    ip_address: Optional[str] = None,
    actor_email: Optional[str] = None,
    target_user_email: Optional[str] = None,
    target_car_vin: Optional[str] = None,
) -> ActionLogMongo:

    if not actor_email and actor_user_id:
        try:
            from crud.users_crud import get_user

            user = get_user(actor_user_id)
            if user:
                actor_email = user.get('email')
        except Exception:
            pass

    if not target_user_email and target_user_id:
        try:
            from crud.users_crud import get_user

            user = get_user(target_user_id)
            if user:
                target_user_email = user.get('email')
        except Exception:
            pass

    if not target_car_vin and target_car_id:
        try:
            from crud.cars_crud import get_car

            car = get_car(target_car_id)
            if car:
                target_car_vin = car.get('vin')
        except Exception:
            pass

    collection = get_collection(ACTION_LOGS_COLLECTION)
    
    log_data = {
        "actor_user_id": actor_user_id,
        "action_type": action_type,
        "description": description,
        "target_user_id": target_user_id,
        "target_car_id": target_car_id,
        "target_rental_id": target_rental_id,
        "old_values": old_values,
        "new_values": new_values,
        "user_agent": user_agent,
        "ip_address": ip_address,
        "actor_email": actor_email,
        "target_user_email": target_user_email,
        "target_car_vin": target_car_vin,
        "created_at": datetime.utcnow(),
    }
    
    result = collection.insert_one(log_data)
    
    return ActionLogMongo(
        id=str(result.inserted_id),
        **log_data
    )


def get_action_logs_mongo(
    offset: int = 0,
    limit: int = 10,
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    user_id: Optional[int] = None,
    action_type: Optional[str] = None,
    target_user_id: Optional[int] = None,
    target_car_id: Optional[int] = None,
) -> List[ActionLogMongo]:
    collection = get_collection(ACTION_LOGS_COLLECTION)
    
    query = {}
    
    if start_date:
        query["created_at"] = {"$gte": start_date}
    if end_date:
        query.setdefault("created_at", {})["$lte"] = end_date
    if user_id:
        query["actor_user_id"] = user_id
    if action_type:
        query["action_type"] = action_type
    if target_user_id:
        query["target_user_id"] = target_user_id
    if target_car_id:
        query["target_car_id"] = target_car_id
    
    cursor = collection.find(query).sort("created_at", DESCENDING).skip(offset).limit(limit)
    
    db = get_mongo_db()
    users_collection = db['users']

    logs = []
    for doc in cursor:
        doc["id"] = str(doc["_id"])
        del doc["_id"]

        if 'actor_user_id' in doc and not isinstance(doc['actor_user_id'], (int, str)):
            doc['actor_user_id'] = str(doc['actor_user_id'])
        if 'target_user_id' in doc and doc['target_user_id'] is not None and not isinstance(doc['target_user_id'], (int, str)):
            doc['target_user_id'] = str(doc['target_user_id'])
        if 'target_car_id' in doc and doc['target_car_id'] is not None and not isinstance(doc['target_car_id'], (int, str)):
            doc['target_car_id'] = str(doc['target_car_id'])
        if 'target_rental_id' in doc and doc['target_rental_id'] is not None and not isinstance(doc['target_rental_id'], (int, str)):
            doc['target_rental_id'] = str(doc['target_rental_id'])

        if (not doc.get('actor_email')) and doc.get('actor_user_id'):
            try:
                user_doc = users_collection.find_one({'_id': doc['actor_user_id']})
                if user_doc and user_doc.get('email'):
                    doc['actor_email'] = user_doc['email']
                else:
                    raise Exception('user not found in mongo')
            except Exception:
                try:
                    from crud.users_crud import get_user
                    user = get_user(doc['actor_user_id'])
                    if user and user.get('email'):
                        doc['actor_email'] = user.get('email')
                except Exception:
                    pass

        if (not doc.get('target_user_email')) and doc.get('target_user_id'):
            try:
                user_doc = users_collection.find_one({'_id': doc['target_user_id']})
                if user_doc and user_doc.get('email'):
                    doc['target_user_email'] = user_doc['email']
                else:
                    raise Exception('user not found in mongo')
            except Exception:
                try:
                    from crud.users_crud import get_user
                    user = get_user(doc['target_user_id'])
                    if user and user.get('email'):
                        doc['target_user_email'] = user.get('email')
                except Exception:
                    pass

        logs.append(ActionLogMongo(**doc))

    return logs


def get_action_logs_count_mongo(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    user_id: Optional[int] = None,
    action_type: Optional[str] = None,
) -> int:
    collection = get_collection(ACTION_LOGS_COLLECTION)
    
    query = {}
    
    if start_date:
        query["created_at"] = {"$gte": start_date}
    if end_date:
        query.setdefault("created_at", {})["$lte"] = end_date
    if user_id:
        query["actor_user_id"] = user_id
    if action_type:
        query["action_type"] = action_type
    
    return collection.count_documents(query)


def get_action_log_by_id_mongo(log_id: str) -> Optional[ActionLogMongo]:
    from bson import ObjectId

    collection = get_collection(ACTION_LOGS_COLLECTION)
    
    try:
        doc = collection.find_one({"_id": ObjectId(log_id)})
        if doc:
            doc["id"] = str(doc["_id"])
            del doc["_id"]
            return ActionLogMongo(**doc)
        return None
    except Exception:
        return None


def delete_action_log_mongo(log_id: str) -> bool:
    from bson import ObjectId

    collection = get_collection(ACTION_LOGS_COLLECTION)

    result = collection.delete_one({"_id": ObjectId(log_id)})
    return result.deleted_count > 0


def create_db_query_log_mongo(
    query: str,
    query_type: str,
    table_name: Optional[str] = None,
    execution_time_ms: Optional[float] = None,
    rows_affected: Optional[int] = None,
    user_id: Optional[int] = None,
    endpoint: Optional[str] = None,
    ip_address: Optional[str] = None,
) -> DbQueryLogMongo:
    collection = get_collection(DB_QUERY_LOGS_COLLECTION)
    
    log_data = {
        "query": query,
        "query_type": query_type,
        "table_name": table_name,
        "execution_time_ms": execution_time_ms,
        "rows_affected": rows_affected,
        "user_id": user_id,
        "endpoint": endpoint,
        "ip_address": ip_address,
        "created_at": datetime.utcnow(),
    }
    
    result = collection.insert_one(log_data)
    
    return DbQueryLogMongo(
        id=str(result.inserted_id),
        **log_data
    )


def get_db_query_logs_mongo(
    offset: int = 0,
    limit: int = 10,
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    table_name: Optional[str] = None,
    query_type: Optional[str] = None,
    user_id: Optional[int] = None,
    endpoint: Optional[str] = None,
) -> List[DbQueryLogMongo]:
    collection = get_collection(DB_QUERY_LOGS_COLLECTION)
    
    query = {}
    
    if start_date:
        query["created_at"] = {"$gte": start_date}
    if end_date:
        query.setdefault("created_at", {})["$lte"] = end_date
    if table_name:
        query["table_name"] = table_name
    if query_type:
        query["query_type"] = query_type
    if user_id:
        query["user_id"] = user_id
    if endpoint:
        query["endpoint"] = endpoint
    
    cursor = collection.find(query).sort("created_at", DESCENDING).skip(offset).limit(limit)
    
    logs = []
    for doc in cursor:
        doc["id"] = str(doc["_id"])
        del doc["_id"]
        logs.append(DbQueryLogMongo(**doc))
    
    return logs


def get_db_query_logs_count_mongo(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    table_name: Optional[str] = None,
    query_type: Optional[str] = None,
) -> int:
    collection = get_collection(DB_QUERY_LOGS_COLLECTION)
    
    query = {}
    
    if start_date:
        query["created_at"] = {"$gte": start_date}
    if end_date:
        query.setdefault("created_at", {})["$lte"] = end_date
    if table_name:
        query["table_name"] = table_name
    if query_type:
        query["query_type"] = query_type
    
    return collection.count_documents(query)


def create_error_log_mongo(
    error_type: str,
    error_message: str,
    stack_trace: Optional[str] = None,
    endpoint: Optional[str] = None,
    user_id: Optional[int] = None,
    user_email: Optional[str] = None,
    request_method: Optional[str] = None,
    request_url: Optional[str] = None,
    request_body: Optional[Dict[str, Any]] = None,
    ip_address: Optional[str] = None,
    severity: str = "ERROR",
) -> ErrorLogMongo:
    collection = get_collection(ERROR_LOGS_COLLECTION)
    
    log_data = {
        "error_type": error_type,
        "error_message": error_message,
        "stack_trace": stack_trace,
        "endpoint": endpoint,
        "user_id": user_id,
        "user_email": user_email,
        "request_method": request_method,
        "request_url": request_url,
        "request_body": request_body,
        "ip_address": ip_address,
        "severity": severity,
        "created_at": datetime.utcnow(),
    }
    
    result = collection.insert_one(log_data)
    
    return ErrorLogMongo(
        id=str(result.inserted_id),
        **log_data
    )


def get_error_logs_mongo(
    offset: int = 0,
    limit: int = 10,
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    error_type: Optional[str] = None,
    severity: Optional[str] = None,
    user_id: Optional[int] = None,
    endpoint: Optional[str] = None,
) -> List[ErrorLogMongo]:
    collection = get_collection(ERROR_LOGS_COLLECTION)
    
    query = {}
    
    if start_date:
        query["created_at"] = {"$gte": start_date}
    if end_date:
        query.setdefault("created_at", {})["$lte"] = end_date
    if error_type:
        query["error_type"] = error_type
    if severity:
        query["severity"] = severity
    if user_id:
        query["user_id"] = user_id
    if endpoint:
        query["endpoint"] = endpoint
    
    cursor = collection.find(query).sort("created_at", DESCENDING).skip(offset).limit(limit)
    
    logs = []
    for doc in cursor:
        doc["id"] = str(doc["_id"])
        del doc["_id"]
        logs.append(ErrorLogMongo(**doc))
    
    return logs


def get_error_logs_count_mongo(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    error_type: Optional[str] = None,
    severity: Optional[str] = None,
) -> int:
    collection = get_collection(ERROR_LOGS_COLLECTION)
    
    query = {}
    
    if start_date:
        query["created_at"] = {"$gte": start_date}
    if end_date:
        query.setdefault("created_at", {})["$lte"] = end_date
    if error_type:
        query["error_type"] = error_type
    if severity:
        query["severity"] = severity
    
    return collection.count_documents(query)
