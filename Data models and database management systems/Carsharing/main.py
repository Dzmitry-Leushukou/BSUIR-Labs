from fastapi import FastAPI
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

app = FastAPI(title="Carsharing API", description="API for carsharing application", version="1.0.0")

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

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)