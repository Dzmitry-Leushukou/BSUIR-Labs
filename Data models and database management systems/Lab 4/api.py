from fastapi import FastAPI, HTTPException
from RedisService import RedisService
import uvicorn


app = FastAPI(title="Redis Service API")
redis_service = RedisService()


@app.get("/")
def read_root():
    return {"Status": "Alive"}

@app.get("/users/{user_id}/last_orders")
def last_user_orders(user_id: str, limit: int = 10):
    orders = redis_service.get_last_user_orders(user_id, limit)
    return {"user_id": user_id, "orders": orders}

@app.get("/products/top_by_sales")
def top_products_by_sales(limit: int = 10):
    products = redis_service.get_top_products_by_sales(limit)
    return {"products": products}

@app.get("/categories/{category_id}/products")
def products_in_category(category_id: str):
    products = redis_service.get_products_by_category(category_id)
    return {"category_id": category_id, "products": products}

@app.get("/users/{user_id}/revenue")
def user_revenue(user_id: str):
    revenue = redis_service.get_user_revenue(user_id)
    return {"user_id": user_id, "revenue": revenue}

@app.get("/users/top_by_orders")
def top_users_by_orders(limit: int = 10):
    users = redis_service.get_top_users_by_orders_count(limit)
    return {"users": users}

@app.get("/users/top_by_revenue")
def top_users_by_revenue(limit: int = 10):
    users = redis_service.get_top_users_by_revenue(limit)
    return {"users": users}

@app.get("/categories/{category_id}/top_product")
def top_product_in_category(category_id: str):
    product = redis_service.get_top_product_in_category(category_id)
    if not product:
        raise HTTPException(status_code=404, detail="No products in category or no sales")
    return {"category_id": category_id, "top_product": product}

@app.get("/users/last_registered")
def last_registered_users(limit: int = 5):
    users = redis_service.get_last_registered_users(limit)
    return {"users": users}

@app.on_event("shutdown")
def shutdown():
    redis_service.close()


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)