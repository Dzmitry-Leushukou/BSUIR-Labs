from fastapi import APIRouter, HTTPException, Depends, Query, Response
from fastapi.responses import JSONResponse
from schemas import LogFilter
from crud.analytics_crud import (
    get_user_activity_stats,
    get_top_active_users,
    get_operations_distribution,
    get_time_series_trends,
    detect_user_anomalies,
    export_to_json,
    export_to_csv
)
from .users_router import get_current_user
from typing import Optional, List
from datetime import datetime
from pydantic import BaseModel


router = APIRouter(prefix="/analytics", tags=["Аналитические отчёты"])


# =============================================================================
# Request/Response schemas
# =============================================================================

class UserActivityStatsResponse(BaseModel):
    period: str
    total_actions: int
    unique_users_count: int
    actions_by_type: dict


class TopUserResponse(BaseModel):
    user_id: int
    email: Optional[str]
    total_actions: int
    actions_by_type: dict
    last_action: datetime
    first_action: datetime


class OperationsDistributionResponse(BaseModel):
    total_operations: int
    by_type: list
    crud_distribution: dict


class TimeSeriesTrendsResponse(BaseModel):
    total_periods: int
    avg_actions: float
    max_actions: int
    min_actions: int
    trend_data: list


class AnomalyDetectionResponse(BaseModel):
    anomalies: list
    statistics: dict


# =============================================================================
# Analytics endpoints
# =============================================================================

@router.get("/user-activity", response_model=List[UserActivityStatsResponse])
def get_user_activity_endpoint(
    period: str = Query('day', regex='^(day|week|month)$'),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """
    Статистика активности пользователей по периодам.
    
    - **period**: Группировка по дням/неделям/месяцам
    - **start_date**: Начальная дата периода (ISO 8601)
    - **end_date**: Конечная дата периода (ISO 8601)
    
    Возвращает:
    - period: Период (дата/неделя/месяц)
    - total_actions: Общее количество действий
    - unique_users_count: Количество уникальных пользователей
    - actions_by_type: Распределение по типам действий
    """
    try:
        data = get_user_activity_stats(period, start_date, end_date)
        return data
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка при получении статистики: {str(e)}")


@router.get("/top-users", response_model=List[TopUserResponse])
def get_top_users_endpoint(
    limit: int = Query(10, ge=1, le=100),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """
    ТОП-10 самых активных пользователей.
    
    - **limit**: Количество пользователей (1-100)
    - **start_date**: Начальная дата периода (ISO 8601)
    - **end_date**: Конечная дата периода (ISO 8601)
    
    Возвращает:
    - user_id: ID пользователя
    - email: Email пользователя
    - total_actions: Общее количество действий
    - actions_by_type: Распределение по типам действий
    - last_action: Время последнего действия
    - first_action: Время первого действия
    """
    try:
        data = get_top_active_users(limit, start_date, end_date)
        return data
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка при получении ТОП пользователей: {str(e)}")


@router.get("/operations-distribution", response_model=OperationsDistributionResponse)
def get_operations_distribution_endpoint(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """
    Распределение операций по типам (CRUD-статистика).
    
    - **start_date**: Начальная дата периода (ISO 8601)
    - **end_date**: Конечная дата периода (ISO 8601)
    
    Возвращает:
    - total_operations: Общее количество операций
    - by_type: Распределение по типам действий
    - crud_distribution: Распределение по CRUD операциям (Create/Read/Update/Delete)
    """
    try:
        data = get_operations_distribution(start_date, end_date)
        return data
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка при получении распределения операций: {str(e)}")


@router.get("/time-series", response_model=TimeSeriesTrendsResponse)
def get_time_series_endpoint(
    period: str = Query('hour', regex='^(hour|day_of_week|hour_of_day)$'),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """
    Временные тренды (time series analysis).
    
    - **period**: Тип группировки (hour/day_of_week/hour_of_day)
        - hour: По часам за период
        - day_of_week: По дням недели (0-6)
        - hour_of_day: По часам суток (0-23)
    - **start_date**: Начальная дата периода (ISO 8601)
    - **end_date**: Конечная дата периода (ISO 8601)
    
    Возвращает:
    - total_periods: Количество периодов
    - avg_actions: Среднее количество действий за период
    - max_actions: Максимальное количество действий
    - min_actions: Минимальное количество действий
    - trend_data: Детальные данные по периодам
    """
    try:
        data = get_time_series_trends(period, start_date, end_date)
        return data
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка при получении временных трендов: {str(e)}")


@router.get("/anomalies", response_model=AnomalyDetectionResponse)
def detect_anomalies_endpoint(
    std_threshold: float = Query(2.0, ge=1.0, le=5.0),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """
    Аномалии в поведении пользователей.
    
    Использует статистический анализ для обнаружения аномалий:
    - high_activity: Необычно высокая активность (> threshold стандартных отклонений)
    - low_activity: Необычно низкая активность
    - diverse_actions: Слишком разнообразное поведение (возможная автоматизация)
    
    - **std_threshold**: Порог в стандартных отклонениях (1.0-5.0)
    - **start_date**: Начальная дата периода (ISO 8601)
    - **end_date**: Конечная дата периода (ISO 8601)
    
    Возвращает:
    - anomalies: Список аномалий с деталями
    - statistics: Статистика анализа (среднее, отклонение, количество)
    """
    try:
        data = detect_user_anomalies(start_date, end_date, std_threshold)
        return data
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка при обнаружении аномалий: {str(e)}")


# =============================================================================
# Export endpoints
# =============================================================================

@router.get("/export/user-activity/json")
def export_user_activity_json(
    period: str = Query('day', regex='^(day|week|month)$'),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """Экспорт статистики активности в JSON формате."""
    try:
        data = get_user_activity_stats(period, start_date, end_date)
        json_data = export_to_json(data)
        return JSONResponse(
            content=data,
            headers={
                "Content-Disposition": f"attachment; filename=user_activity_{period}.json"
            }
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка экспорта: {str(e)}")


@router.get("/export/user-activity/csv")
def export_user_activity_csv(
    period: str = Query('day', regex='^(day|week|month)$'),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """Экспорт статистики активности в CSV формате."""
    try:
        data = get_user_activity_stats(period, start_date, end_date)
        csv_data = export_to_csv(data)
        return Response(
            content=csv_data,
            media_type="text/csv",
            headers={
                "Content-Disposition": f"attachment; filename=user_activity_{period}.csv"
            }
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка экспорта: {str(e)}")


@router.get("/export/top-users/json")
def export_top_users_json(
    limit: int = Query(10, ge=1, le=100),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """Экспорт ТОП пользователей в JSON формате."""
    try:
        data = get_top_active_users(limit, start_date, end_date)
        return JSONResponse(
            content=data,
            headers={
                "Content-Disposition": f"attachment; filename=top_users.json"
            }
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка экспорта: {str(e)}")


@router.get("/export/top-users/csv")
def export_top_users_csv(
    limit: int = Query(10, ge=1, le=100),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """Экспорт ТОП пользователей в CSV формате."""
    try:
        data = get_top_active_users(limit, start_date, end_date)
        csv_data = export_to_csv(data)
        return Response(
            content=csv_data,
            media_type="text/csv",
            headers={
                "Content-Disposition": f"attachment; filename=top_users.csv"
            }
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка экспорта: {str(e)}")


@router.get("/export/operations-distribution/json")
def export_operations_distribution_json(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """Экспорт распределения операций в JSON формате."""
    try:
        data = get_operations_distribution(start_date, end_date)
        return JSONResponse(
            content=data,
            headers={
                "Content-Disposition": "attachment; filename=operations_distribution.json"
            }
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка экспорта: {str(e)}")


@router.get("/export/anomalies/json")
def export_anomalies_json(
    std_threshold: float = Query(2.0, ge=1.0, le=5.0),
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    current_user: dict = Depends(get_current_user)
):
    """Экспорт аномалий в JSON формате."""
    try:
        data = detect_user_anomalies(start_date, end_date, std_threshold)
        return JSONResponse(
            content=data,
            headers={
                "Content-Disposition": "attachment; filename=anomalies.json"
            }
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ошибка экспорта: {str(e)}")
