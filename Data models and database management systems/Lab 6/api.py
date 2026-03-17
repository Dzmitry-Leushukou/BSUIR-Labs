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

@app.get("/validatation/count")
def validation_count():
    categories_count=mongo_service.get_collection_size("categories")
    users_count=mongo_service.get_collection_size("users")
    products_count=mongo_service.get_collection_size("products")
    orders_count=mongo_service.get_collection_size("orders")
    order_items_count=mongo_service.get_collection_size("order_items")

    categories_csv_lines_count=0
    users_csv_lines_count=0
    products_csv_lines_count=0
    orders_csv_lines_count=0
    order_items_csv_lines_count=0

    with open(files["categories"], "r") as f:
        categories_csv_lines_count=len(f.readlines())-1

    with open(files["users"], "r") as f:
        users_csv_lines_count=len(f.readlines())-1

    with open(files["products"], "r") as f:
        products_csv_lines_count=len(f.readlines())-1

    with open(files["orders"], "r") as f:
        orders_csv_lines_count=len(f.readlines())-1

    with open(files["order_items"], "r") as f:
        order_items_csv_lines_count=len(f.readlines())-1

    if categories_count!=categories_csv_lines_count:
        raise HTTPException(status_code=404, detail="Categories count does not match")

    if users_count != users_csv_lines_count:
        raise HTTPException(status_code=404, detail="Users count does not match")
    
    if products_count != products_csv_lines_count:
        raise HTTPException(status_code=404, detail="Products count does not match")

    if orders_count != orders_csv_lines_count:
        raise HTTPException(status_code=404, detail="Orders count does not match")
    
    if order_items_count != order_items_csv_lines_count:
        raise HTTPException(status_code=404, detail="Order items count does not match")

    return {
            "status": "Count validation successful",
            "categories_count":categories_count, 
            "users_count": users_count,
            "products_count":products_count,
            "orders_count":orders_count,
            "order_items_count": order_items_count
           }

@app.get("/validatation/user_id_in_orders")
def validation_user_id_in_orders():
    invalid_orders = mongo_service.check_userId_validity_in_orders()
    if not invalid_orders:
        return {"status": "User id in orders validation successful"}
    else:
        raise HTTPException(
            status_code=404,
            detail=f"Found {len(invalid_orders)} orders with invalid user_id"
        )

@app.get("/validation/orders_total_greater_than_1000")
def validation_orders_total_greater_than_1000():
    orders = mongo_service.get_orders_total_gt_1000()
    return{"status": "Orders total greater than 1000 validation successful", "orders": orders}

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
def performance_test(user_id: int = 1):
    try:
        mongo_service.drop_indexes()
        time.sleep(0.5)
        
        start = time.time()
        stats_without_index = mongo_service.get_query_performance(user_id)
        time_without_index = (time.time() - start) * 1000  # Convert to ms
        
        mongo_service.create_indexes()
        time.sleep(0.5)
        
        start = time.time()
        stats_with_index = mongo_service.get_query_performance(user_id)
        time_with_index = (time.time() - start) * 1000  # Convert to ms
        
        improvement_percent = ((time_without_index - time_with_index) / time_without_index * 100) if time_without_index > 0 else 0
        docs_reduction = ((stats_without_index['total_documents_examined'] - stats_with_index['total_documents_examined']) 
                         / stats_without_index['total_documents_examined'] * 100) if stats_without_index['total_documents_examined'] > 0 else 0
        
        return {
            "user_id": user_id,
            "without_index": {
                "time_ms": round(time_without_index, 2),
                "docs_examined": stats_without_index['total_documents_examined'],
                "docs_returned": stats_without_index['total_documents_returned'],
                "execution_stage": stats_without_index['execution_stages'],
                "index_used": stats_without_index['is_index_used']
            },
            "with_index": {
                "time_ms": round(time_with_index, 2),
                "docs_examined": stats_with_index['total_documents_examined'],
                "docs_returned": stats_with_index['total_documents_returned'],
                "execution_stage": stats_with_index['execution_stages'],
                "index_used": stats_with_index['is_index_used']
            },
            "improvement": {
                "time_improvement_percent": round(improvement_percent, 1),
                "docs_scanned_reduction_percent": round(docs_reduction, 1),
                "time_saved_ms": round(time_without_index - time_with_index, 2)
            }
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error running performance test: {str(e)}")

@app.on_event("shutdown")
def shutdown():
    mongo_service.close()


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)