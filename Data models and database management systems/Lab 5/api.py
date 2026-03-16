from schemas import EmailTaskDTO, LogTaskDTO
from fastapi import FastAPI, HTTPException
from RedisService import RedisService
import uvicorn


app = FastAPI(title="Redis Service API")
redis_service = RedisService()


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