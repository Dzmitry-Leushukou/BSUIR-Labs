from fastapi import APIRouter, HTTPException, Depends, Request
from schemas import *
from crud.trip_completions_crud import *
from typing import List
from .users_router import get_current_user_from_header
from datetime import datetime, timezone

router = APIRouter(prefix="/trip-completions", tags=["Завершения поездок"])

@router.get("/", response_model=List[TripCompletion])
def get_trip_completions_endpoint(offset: int = 0, limit: int = 10):
    return get_trip_completions(offset, limit)

@router.get("/{completion_id}", response_model=TripCompletion)
def get_trip_completion_endpoint(completion_id: int):
    return get_trip_completion(completion_id)

@router.get("/rental/{rental_id}", response_model=TripCompletion)
def get_trip_completion_by_rental_endpoint(rental_id: int):
    completion = get_trip_completion_by_rental_id(rental_id)
    if not completion:
        raise HTTPException(status_code=404, detail="Trip completion not found for this rental")
    return completion

@router.post("/", response_model=TripCompletion)
def create_trip_completion_endpoint(request: Request, completion: TripCompletionCreate, current_user: dict = Depends(get_current_user_from_header)):
    # Check if user is admin
    from crud.users_crud import get_user
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    # Only admin can approve/reject trip completion, regular users can only create completion request
    if is_admin and completion.admin_approved is not None:
        # Admin approving/rejecting - set review time
        completion.admin_reviewed_at = datetime.now(timezone.utc)
    else:
        # Regular user creating a completion request - make sure admin_approved is None
        completion.admin_approved = None
        
        # Update rental status to pending_completion
        from crud.rentals_crud import get_rental, update_rental
        from schemas import RentalUpdate
        rental = get_rental(completion.rental_id)
        
        # Only allow creating trip completion if rental is currently active
        if rental['status'] != 'active':
            raise HTTPException(status_code=400, detail="Rental must be active to create completion request")
        
        # Update rental to pending completion status
        rental_update = RentalUpdate(status="pending_completion")
        update_rental(rental['id'], rental_update)
    
    return create_trip_completion(completion)

@router.put("/{completion_id}", response_model=TripCompletion)
def update_trip_completion_endpoint(request: Request, completion_id: int, completion: TripCompletionUpdate, current_user: dict = Depends(get_current_user_from_header)):
    # Only admin can update trip completion
    from crud.users_crud import get_user
    current_user_details = get_user(current_user['id'])
    is_admin = current_user_details['role_id'] == 1  # assuming admin role_id is 1
    
    if not is_admin:
        raise HTTPException(status_code=403, detail="Only admin can update trip completion")
    
    # Set review time if admin is approving/rejecting
    if completion.admin_approved is not None:
        completion.admin_reviewed_at = datetime.now(timezone.utc)
        completion.admin_reviewed_by = current_user['id']
        
        # Get the trip completion and associated rental
        from crud.rentals_crud import get_rental, update_rental
        from crud.cars_crud import update_car
        from schemas import RentalUpdate, CarUpdate
        trip_completion = get_trip_completion(completion_id)
        rental = get_rental(trip_completion['rental_id'])
        
        # Update rental to completed status
        rental_update = RentalUpdate(status="completed", ended_at=datetime.now(timezone.utc))
        updated_rental = update_rental(rental['id'], rental_update)
        
        # Determine car status based on admin approval and comments
        if completion.admin_approved and not completion.admin_comment:
            # Trip completed successfully with no damage - car becomes available
            car_update = CarUpdate(status="available")
            update_car(updated_rental['car_id'], car_update)
        elif completion.admin_approved and completion.admin_comment:
            # Trip completed but admin found damage - car needs maintenance
            car_update = CarUpdate(status="maintenance")
            update_car(updated_rental['car_id'], car_update)
            
            # Create a maintenance request for the damage
            from crud.maintenance_requests_crud import create_maintenance_request
            from schemas import MaintenanceRequestCreate
            maintenance_request = MaintenanceRequestCreate(
                car_id=updated_rental['car_id'],
                reported_by=completion.admin_reviewed_by,
                description=completion.admin_comment
            )
            create_maintenance_request(maintenance_request)
        else:
            # Trip was rejected by admin or still pending - set neutral status ("in_process")
            # We'll use "maintenance" status as a neutral state for cases where admin hasn't decided yet
            # or if admin rejected the trip completion
            car_update = CarUpdate(status="maintenance")
            update_car(updated_rental['car_id'], car_update)
            
            # If admin provided a comment about damage, create a maintenance request
            if completion.admin_comment:
                from crud.maintenance_requests_crud import create_maintenance_request
                from schemas import MaintenanceRequestCreate
                maintenance_request = MaintenanceRequestCreate(
                    car_id=updated_rental['car_id'],
                    reported_by=completion.admin_reviewed_by,
                    description=completion.admin_comment
                )
                create_maintenance_request(maintenance_request)
    
    return update_trip_completion(completion_id, completion)

@router.get("/count", response_model=dict)
def get_trip_completions_count_endpoint():
    from crud.trip_completions_crud import get_trip_completions_count
    count = get_trip_completions_count()
    return {"count": count}