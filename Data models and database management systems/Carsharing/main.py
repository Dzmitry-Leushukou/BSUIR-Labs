from fastapi import FastAPI, Request
from routers.roles_router import router as roles_router
from routers.users_router import router as users_router
from routers.cars_router import router as cars_router
from routers.photos_router import router as photos_router
from routers.driver_licenses_router import router as driver_licenses_router
from routers.sessions_router import router as sessions_router
from routers.car_states_router import router as car_states_router
from routers.rentals_router import router as rentals_router
from routers.maintenance_requests_router import router as maintenance_requests_router
from routers.payment_logs_router import router as payment_logs_router
from routers.logs_router import router as logs_router
from routers.action_logs_router import router as action_logs_router
from routers.trip_completions_router import router as trip_completions_router
from routers.analytics_router import router as analytics_router

from fastapi.staticfiles import StaticFiles
from fastapi.responses import HTMLResponse
from starlette.responses import FileResponse
from middleware.mongo_logging import MongoDBLoggingMiddleware, DatabaseQueryLoggingMiddleware, setup_mongodb_logging
import os

app = FastAPI(title="Carsharing API", description="API for carsharing application", version="1.0.0")

# Custom StaticFiles class to add cache control headers
class NoCacheStaticFiles(StaticFiles):
    async def get_response(self, path: str, scope):
        response = await super().get_response(path, scope)
        if isinstance(response, FileResponse):
            # Add cache control headers to prevent 304 issues
            response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
            response.headers["Pragma"] = "no-cache"
            response.headers["Expires"] = "0"
        return response

# Mount static files with cache control
app.mount("/static", NoCacheStaticFiles(directory="frontend"), name="static")
app.mount("/uploads", NoCacheStaticFiles(directory="frontend/uploads"), name="uploads")

# Add MongoDB error logging middleware
app.add_middleware(MongoDBLoggingMiddleware)

# Include routers
app.include_router(roles_router)
app.include_router(users_router)
app.include_router(cars_router)
app.include_router(photos_router)
app.include_router(driver_licenses_router)
app.include_router(sessions_router)
app.include_router(car_states_router)
app.include_router(rentals_router)
app.include_router(maintenance_requests_router)
app.include_router(payment_logs_router)
app.include_router(logs_router)
app.include_router(action_logs_router)
app.include_router(trip_completions_router)
app.include_router(analytics_router)


@app.middleware("http")
async def set_request_context(request: Request, call_next):
    """Set request context for query logging."""
    # Extract user info from request state if available
    endpoint = request.url.path
    user_id = None
    
    # User info will be set by authentication middleware later
    # For now, we just set the endpoint
    DatabaseQueryLoggingMiddleware.set_context(endpoint=endpoint, user_id=user_id)
    
    try:
        response = await call_next(request)
        
        # Update context with user info if available after authentication
        if hasattr(request.state, "current_user"):
            DatabaseQueryLoggingMiddleware.set_context(
                endpoint=endpoint,
                user_id=request.state.current_user.get("id")
            )
        
        return response
    finally:
        DatabaseQueryLoggingMiddleware.clear_context()


# Маршрут для главной страницы
@app.get("/")
async def read_root():
    with open("frontend/index.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для страницы профиля
@app.get("/profile")
async def read_profile():
    with open("frontend/profile.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для админ панели
@app.get("/admin")
async def read_admin():
    with open("frontend/admin.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для таблицы Action logs
@app.get("/admin/action_logs")
async def read_admin_action_logs():
    with open("frontend/admin_action_logs.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для таблицы Cars
@app.get("/admin/cars")
async def read_admin_cars():
    with open("frontend/admin_cars.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для таблицы Maintenance requests
@app.get("/admin/maintenance_requests")
async def read_admin_maintenance_requests():
    with open("frontend/admin_maintenance_requests.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для таблицы Payment logs
@app.get("/admin/payment_logs")
async def read_admin_payment_logs():
    with open("frontend/admin_payment_logs.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для таблицы подтверждения документов
@app.get("/admin/driver_licenses")
async def read_admin_driver_licenses():
    with open("frontend/admin_driver_licenses.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для таблицы Rentals
@app.get("/admin/rentals")
async def read_admin_rentals():
    with open("frontend/admin_rentals.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для таблицы Users
@app.get("/admin/users")
async def read_admin_users():
    with open("frontend/admin_users.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())

# Маршрут для таблицы подтверждения завершения поездок
@app.get("/admin/trip_completions")
async def read_admin_trip_completions():
    with open("frontend/admin_trip_completions.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())


# Маршрут для страницы аналитики
@app.get("/admin/analytics")
async def read_admin_analytics():
    with open("frontend/admin_analytics.html", "r", encoding="utf-8") as file:
        return HTMLResponse(content=file.read())


@app.on_event("startup")
async def startup_event():
    """Initialize MongoDB logging on startup."""
    try:
        # Check if MongoDB logging is enabled
        if os.getenv("MONGODB_HOST"):
            setup_mongodb_logging()
            print("MongoDB logging initialized successfully")
        else:
            print("MongoDB logging not configured (MONGODB_HOST not set)")
    except Exception as e:
        print(f"Warning: Could not initialize MongoDB logging: {e}")


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
