from fastapi import FastAPI, HTTPException, Query
from MongoService import MongoService
import uvicorn
import os
import time

app = FastAPI(title="MongoService API - Lab 7: Aggregation Framework")
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
    return {
        "Status": "Alive",
        "Lab": "7 - MongoDB Aggregation Framework",
        "Features": [
            "Monthly revenue by category",
            "Market basket analysis (with confidence/lift)",
            "RFM analysis (with scoring & segmentation)",
            "Materialized views ($out)",
            "Performance comparison (live vs materialized)",
            "Order processing with transactions",
            "Write conflict demonstration",
            "Warehouse inventory management",
            "Isolation level analysis"
        ]
    }


# ===================== DATA LOADING =====================

@app.post("/data/load-csv")
def load_all_csv():
    """Загружает все данные из CSV-файлов."""
    try:
        mongo_service.load_all_csv(DATA_DIR)
        return {"status": "success", "message": "All CSV data loaded"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/data/regenerate")
def regenerate_data():
    """Перегенерировать тестовые данные и перезагрузить."""
    try:
        import subprocess
        result = subprocess.run(
            ["python", "generate_test_data.py"],
            capture_output=True, text=True, timeout=60
        )
        if result.returncode != 0:
            raise HTTPException(status_code=500, detail=result.stderr)
        mongo_service.flush_db()
        mongo_service.load_all_csv(DATA_DIR)
        mongo_service.setup_warehouses()
        mongo_service.create_monthly_revenue_materialized_view()
        counts = mongo_service.get_collection_counts()
        return {"status": "success", "message": result.stdout.strip(), "counts": counts}
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/data/flush")
def flush_db():
    """Очищает все коллекции."""
    mongo_service.flush_db()
    return {"status": "success", "message": "Database flushed"}


@app.get("/validation/collection-counts")
def validation_count():
    counts = mongo_service.get_collection_counts()
    return {"status": "Count validation successful", "counts": counts}


# ===================== AGGREGATIONS =====================

@app.get("/aggregations/monthly-revenue")
def monthly_revenue_by_category(year: int = Query(None, description="Filter by year")):
    """Ежемесячная выручка по категориям (GROUP BY category, month)."""
    result = mongo_service.get_monthly_revenue_by_category(year)
    return {
        "aggregation_type": "monthly_revenue_by_category",
        "year_filter": year,
        "records_count": len(result),
        "data": result
    }


@app.get("/aggregations/market-basket-analysis")
def market_basket_analysis(
    min_support: int = Query(2, ge=1),
    limit: int = Query(20, ge=1, le=100)
):
    """Корзиночный анализ: товары, часто покупаемые вместе + confidence/lift."""
    result = mongo_service.get_market_basket_analysis(min_support, limit)
    return {
        "aggregation_type": "market_basket_analysis",
        "min_support": min_support,
        "limit": limit,
        "pairs_count": len(result),
        "data": result
    }


@app.get("/aggregations/rfm-analysis")
def rfm_analysis(limit: int = Query(20, ge=1, le=100)):
    """RFM-анализ клиентов с скорингом и сегментацией."""
    result = mongo_service.get_rfm_analysis(limit)
    return {
        "aggregation_type": "rfm_analysis",
        "limit": limit,
        "users_count": len(result.get("data", [])),
        "segment_distribution": result.get("segments", {}),
        "data": result.get("data", [])
    }


# ===================== MATERIALIZED VIEWS =====================

@app.post("/materialized-views/monthly-revenue/create")
def create_materialized_view():
    """Создаёт материализованное представление (через $out)."""
    try:
        result = mongo_service.create_monthly_revenue_materialized_view()
        return result
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error creating materialized view: {str(e)}")


@app.get("/materialized-views/monthly-revenue/query")
def query_materialized_view():
    """Чтение из материализованного представления."""
    result = mongo_service.get_monthly_revenue_from_view()
    return result


@app.get("/materialized-views/performance-comparison")
def performance_comparison():
    """Сравнение производительности: live aggregation vs materialized view."""
    try:
        result = mongo_service.compare_aggregation_performance()
        return result
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error comparing performance: {str(e)}")


# ===================== ORDER TRANSACTIONS =====================

@app.post("/orders/process")
def process_order(order_id: int, new_status: str = "processing"):
    """Обработка заказа с транзакцией (обновление статуса + запись в историю)."""
    try:
        result = mongo_service.process_order_transaction(order_id, new_status)
        return result
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error processing order: {str(e)}")


@app.get("/orders/history")
def get_order_history(limit: int = Query(50, ge=1, le=500)):
    """История изменений заказов."""
    result = mongo_service.get_order_history(limit)
    return {
        "limit": limit,
        "records_count": len(result),
        "history": result
    }


# ===================== WRITE CONFLICT DEMO =====================

@app.post("/transactions/write-conflict")
def write_conflict_demo(product_id: int = 1):
    """Симуляция гонки: два пользователя пытаются купить последний товар.
    WriteConflict предотвращает некорректное состояние."""
    try:
        result = mongo_service.simulate_write_conflict(product_id)
        return result
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error simulating write conflict: {str(e)}")


# ===================== ISOLATION LEVELS =====================

@app.get("/transactions/isolation-levels")
def isolation_levels_analysis():
    """Анализ уровней изоляции: dirty reads, non-repeatable reads, phantom reads."""
    return mongo_service.demonstrate_isolation_levels()


# ===================== WAREHOUSE MANAGEMENT =====================

@app.post("/warehouses/setup")
def setup_warehouses():
    """Инициализация тестовых складов с товарами."""
    try:
        result = mongo_service.setup_warehouses()
        return result
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/warehouses/transfer")
def transfer_product(
    product_id: int,
    from_warehouse: str,
    to_warehouse: str,
    quantity: int = Query(1, ge=1)
):
    """Перемещение товара между складами с транзакцией.
    Включает проверку остатков, обновление инвентаря, запись в истории."""
    try:
        result = mongo_service.transfer_product_between_warehouses(
            product_id, from_warehouse, to_warehouse, quantity
        )
        if "error" in result:
            raise HTTPException(status_code=400, detail=result["error"])
        return result
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/warehouses/inventory")
def get_inventory_report(warehouse_id: str = Query(None)):
    """Отчёт по инвентарю (опционально фильтруется по складу)."""
    result = mongo_service.get_inventory_report(warehouse_id)
    return {
        "warehouse_filter": warehouse_id,
        "records_count": len(result),
        "inventory": result
    }


@app.get("/warehouses/transfers/history")
def get_transfer_history(limit: int = Query(50, ge=1, le=500)):
    """История перемещений между складами."""
    result = mongo_service.get_transfer_history(limit)
    return {
        "limit": limit,
        "records_count": len(result),
        "transfers": result
    }


# ===================== SHUTDOWN =====================

@app.on_event("shutdown")
def shutdown():
    mongo_service.close()


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
