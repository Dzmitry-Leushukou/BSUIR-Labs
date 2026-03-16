from schemas import EmailTaskDTO, LogTaskDTO
from fastapi import FastAPI, HTTPException, Request
from RedisService import RedisService
import uvicorn
from fastapi.responses import JSONResponse

app = FastAPI(title="Redis Service API")
redis_service = RedisService()



@app.middleware("http")
async def rate_limit_middleware(request: Request, call_next):
    client_ip = request.client.host
    if request.url.path == "/task/email":
        if not redis_service.check_simple_rate(client_ip):
            return JSONResponse(
                status_code=429, 
                content={"detail": "Too many requests for email tasks"}
            )
    elif request.url.path == "/task/log":
        if not redis_service.check_sliding_window_rate(client_ip):
            return JSONResponse(
                status_code=429, 
                content={"detail": "Too many requests for log tasks"}
            )
    response = await call_next(request)
    return response

@app.get("/")
def read_root():
    return {"Status": "Alive"}


@app.post("/task/email")
def create_email_task(task: EmailTaskDTO):
    try:
        dto_task = EmailTaskDTO(
            task_id=task.task_id,
            data=task.data
        )
        redis_service.add_task(dto_task)
        return {"message": f"Email task {task.task_id} added to queue."}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/task/log")
def create_log_task(task: LogTaskDTO):
    try:
        dto_task = LogTaskDTO(
            task_id=task.task_id,
            data=task.data
        )
        redis_service.add_task(dto_task)
        return {"message": f"Log task {task.task_id} added to queue."}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.on_event("shutdown")
def shutdown():
    redis_service.close()


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)