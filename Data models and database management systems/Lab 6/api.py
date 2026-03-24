from fastapi import FastAPI, HTTPException
from MongoService import MongoService
import uvicorn
import os
import time

app = FastAPI(title="MongoService API")
mongo_service = MongoService()
DATA_DIR = "Data_csv"
files = {
    "categories": os.path.join(DATA_DIR, "categoryid-name.csv"),
    "users": os.path.join(DATA_DIR, "userid-name-email-createdat.csv"),
    "products": os.path.join(DATA_DIR, "productid-name-categoryid-price.csv"),
    "orders": os.path.join(DATA_DIR, "orderid-userid-createdat-status.csv"),
    "order_items": os.path.join(DATA_DIR, "orderitemid-orderid-productid-quantity-price.csv"),
}

@app.get("/")
def read_root():
    return {"Status": "Alive"}

@app.get("/validation/collection-counts")
def validation_count():
    counts = mongo_service.get_collection_counts()
    return {"status": "Count validation successful", "counts": counts}

@app.get("/validation/invalid-user-orders")
def validation_user_id_in_orders():
    invalid_orders = mongo_service.get_invalid_user_orders()
    if not invalid_orders:
        return {"status": "User id in orders validation successful", "invalid_orders": []}
    else:
        return {"status": "Found invalid orders", "invalid_orders_count": len(invalid_orders), "invalid_orders": invalid_orders}

@app.get("/validation/orders-total-greater-than-1000")
def validation_orders_total_greater_than_1000():
    orders = mongo_service.get_orders_with_sum_gt_1000()
    gt_1000 = [o for o in orders if o.get("total", 0) > 1000]
    return {"status": "Orders total greater than 1000", "orders_count": len(gt_1000), "orders": gt_1000}

@app.get("/users/{user_id}/orders")
def get_user_orders(user_id: int):
    orders = mongo_service.get_user_orders(user_id)
    if not orders:
        raise HTTPException(status_code=404, detail=f"No orders found for user {user_id}")
    return {"user_id": user_id, "orders_count": len(orders), "orders": orders}

@app.get("/orders/{order_id}/details")
def get_order_with_details(order_id: int):
    details = mongo_service.get_order_with_details(order_id)
    if not details:
        raise HTTPException(status_code=404, detail=f"Order {order_id} not found")
    
    total_price = sum(item.get("total_price", 0) for item in details)
    return {
        "order_id": order_id,
        "items_count": len(details),
        "total_price": total_price,
        "details": details
    }

@app.get("/products/top-by-revenue")
def get_top_products_by_revenue(limit: int = 10):
    products = mongo_service.get_top_products_by_revenue(limit)
    if not products:
        raise HTTPException(status_code=404, detail="No product sales data found")
    
    total_revenue = sum(p.get("revenue", 0) for p in products)
    return {
        "limit": limit,
        "products_count": len(products),
        "total_revenue": total_revenue,
        "products": products
    }

@app.post("/indexes/create")
def create_indexes():
    try:
        mongo_service.create_indexes()
        return {"status": "Indexes created successfully"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error creating indexes: {str(e)}")

@app.post("/indexes/drop")
def drop_indexes():
    try:
        mongo_service.drop_indexes()
        return {"status": "Indexes dropped successfully"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error dropping indexes: {str(e)}")

@app.get("/performance/query/{user_id}")
def get_query_performance(user_id: int):
    try:
        stats = mongo_service.get_query_performance(user_id)
        return {
            "user_id": user_id,
            "performance_stats": stats
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error getting performance stats: {str(e)}")

@app.post("/performance/test/{user_id}")
def performance_test(user_id: int = 1, iterations: int = 5):
    try:
        mongo_service.drop_indexes()
        time.sleep(0.5)

        times_without_index = []
        stats_without_index = None

        for _ in range(iterations):
            elapsed_ms, docs_returned = mongo_service.execute_query_and_measure_time(user_id)
            times_without_index.append(elapsed_ms)
            time.sleep(0.1)

        stats_without_index = mongo_service.get_query_performance(user_id)
        avg_time_without_index = sum(times_without_index) / len(times_without_index)

        mongo_service.create_indexes()
        time.sleep(0.5)

        times_with_index = []
        stats_with_index = None

        for _ in range(iterations):
            elapsed_ms, docs_returned = mongo_service.execute_query_and_measure_time(user_id)
            times_with_index.append(elapsed_ms)
            time.sleep(0.1)

        stats_with_index = mongo_service.get_query_performance(user_id)
        avg_time_with_index = sum(times_with_index) / len(times_with_index)

        improvement_percent = ((avg_time_without_index - avg_time_with_index) / avg_time_without_index * 100) if avg_time_without_index > 0 else 0
        docs_reduction = ((stats_without_index['total_documents_examined'] - stats_with_index['total_documents_examined'])
                         / stats_without_index['total_documents_examined'] * 100) if stats_without_index['total_documents_examined'] > 0 else 0

        return {
            "user_id": user_id,
            "test_iterations": iterations,
            "notes": "Negative improvement means index is slower (common on small datasets)",
            "without_index": {
                "avg_time_ms": round(avg_time_without_index, 2),
                "min_time_ms": round(min(times_without_index), 2),
                "max_time_ms": round(max(times_without_index), 2),
                "docs_examined": stats_without_index['total_documents_examined'],
                "docs_returned": stats_without_index['total_documents_returned'],
                "execution_stage": stats_without_index['execution_stages'],
                "index_used": stats_without_index['is_index_used']
            },
            "with_index": {
                "avg_time_ms": round(avg_time_with_index, 2),
                "min_time_ms": round(min(times_with_index), 2),
                "max_time_ms": round(max(times_with_index), 2),
                "docs_examined": stats_with_index['total_documents_examined'],
                "docs_returned": stats_with_index['total_documents_returned'],
                "execution_stage": stats_with_index['execution_stages'],
                "index_used": stats_with_index['is_index_used']
            },
            "improvement": {
                "time_improvement_percent": round(improvement_percent, 1),
                "docs_scanned_reduction_percent": round(docs_reduction, 1),
                "time_saved_ms": round(avg_time_without_index - avg_time_with_index, 2),
                "verdict": "FASTER with index ✓" if improvement_percent > 10 else "SLOWER with index (normal on small datasets)" if improvement_percent < -10 else "SIMILAR performance"
            }
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error running performance test: {str(e)}")

@app.get("/analytics/top-users")
def top_users(limit: int = 10):
    users = mongo_service.get_top_users_by_orders_count(limit)
    return {
        "limit": limit,
        "users_count": len(users),
        "users": users
    }

@app.get("/analytics/top-categories")
def top_categories(limit: int = 10):
    categories = mongo_service.get_top_categories_by_revenue(limit)
    return {
        "limit": limit,
        "categories_count": len(categories),
        "categories": categories
    }

@app.get("/analytics/order-statistics")
def order_statistics():
    stats = mongo_service.get_order_statistics()
    return {"statistics": stats}

@app.get("/analytics/indexes")
def get_indexes():
    return mongo_service.get_all_indexes()

@app.on_event("shutdown")
def shutdown():
    mongo_service.close()


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)