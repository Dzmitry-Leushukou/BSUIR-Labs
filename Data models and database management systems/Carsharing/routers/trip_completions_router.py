from fastapi import APIRouter, HTTPException, Depends, Request
from schemas import *
from crud.trip_completions_crud import *
from crud.photos_crud import get_photos_by_trip_completion_id
from typing import List
from .users_router import get_current_user
from datetime import datetime, timezone

router = APIRouter(prefix="/trip-completions", tags=["Завершения поездок"])

@router.get("/", response_model=List[TripCompletion])
def get_trip_completions_endpoint(offset: int = 0, limit: int = 10):
    return get_trip_completions(offset, limit)

@router.get("/count", response_model=dict)
def get_trip_completions_count_endpoint(current_user: dict = Depends(get_current_user)):
    from crud.trip_completions_crud import get_trip_completions_count
    count = get_trip_completions_count()
    return {"count": count}

@router.get("/{completion_id}", response_model=TripCompletion)
def get_trip_completion_endpoint(completion_id: int):
    return get_trip_completion(completion_id)

@router.get("/rental/{rental_id}", response_model=TripCompletion)
def get_trip_completion_by_rental_endpoint(rental_id: int):
    completion = get_trip_completion_by_rental_id(rental_id)
    if not completion:
        raise HTTPException(status_code=404, detail="Завершение поездки не найдено для этой аренды")
    return completion

@router.post("/", response_model=TripCompletion)
def create_trip_completion_endpoint(request: Request, completion: TripCompletionCreate, current_user: dict = Depends(get_current_user)):
    from crud.users_crud import get_user
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    if is_admin and completion.admin_approved is not None:
        completion.admin_reviewed_at = datetime.now(timezone.utc)
    else:
        completion.admin_approved = None

        from crud.rentals_crud import get_rental, update_rental
        from schemas import RentalUpdate
        rental = get_rental(completion.rental_id)

        if rental['status'] != 'active':
            raise HTTPException(status_code=400, detail="Аренда должна быть активной для создания запроса на завершение")

        rental_update = RentalUpdate(status="pending_completion")
        update_rental(rental['id'], rental_update)

    return create_trip_completion(completion)

@router.put("/{completion_id}", response_model=TripCompletion)
def update_trip_completion_endpoint(request: Request, completion_id: int, completion: TripCompletionUpdate, current_user: dict = Depends(get_current_user)):
    from crud.users_crud import get_user
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1

    if not is_admin:
        raise HTTPException(status_code=403, detail="Только администратор может обновлять завершение поездки")

    if completion.admin_approved is not None:
        completion.admin_reviewed_at = datetime.now(timezone.utc)
        completion.admin_reviewed_by = current_user['id']

        from crud.rentals_crud import get_rental, update_rental
        from crud.cars_crud import update_car
        from schemas import RentalUpdate, CarUpdate
        trip_completion = get_trip_completion(completion_id)
        rental = get_rental(trip_completion['rental_id'])

        rental_update = RentalUpdate(status="completed", ended_at=datetime.now(timezone.utc))
        updated_rental = update_rental(rental['id'], rental_update)

        if completion.admin_approved is True and not completion.admin_comment:
            car_update = CarUpdate(status="available")
            update_car(updated_rental['car_id'], car_update)
        elif completion.admin_approved is True and completion.admin_comment:
            car_update = CarUpdate(status="maintenance")
            update_car(updated_rental['car_id'], car_update)

            from crud.maintenance_requests_crud import create_maintenance_request
            from schemas import MaintenanceRequestCreate
            maintenance_request = MaintenanceRequestCreate(
                car_id=updated_rental['car_id'],
                reported_by=completion.admin_reviewed_by,
                description=completion.admin_comment
            )
            create_maintenance_request(maintenance_request)
        elif completion.admin_approved is False:
            car_update = CarUpdate(status="maintenance")
            update_car(updated_rental['car_id'], car_update)

            if completion.admin_comment:
                from crud.maintenance_requests_crud import create_maintenance_request
                from schemas import MaintenanceRequestCreate
                maintenance_request = MaintenanceRequestCreate(
                    car_id=updated_rental['car_id'],
                    reported_by=completion.admin_reviewed_by,
                    description=completion.admin_comment
                )
                create_maintenance_request(maintenance_request)
        else:
            from crud.cars_crud import get_car
            current_car = get_car(updated_rental['car_id'])
            car_update = CarUpdate(status=current_car['status'])
            update_car(updated_rental['car_id'], car_update)

    return update_trip_completion(completion_id, completion)