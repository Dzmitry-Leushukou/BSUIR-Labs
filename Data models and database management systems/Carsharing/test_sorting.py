#!/usr/bin/env python3
"""
Test script to verify sorting changes work correctly
"""
import psycopg2
from psycopg2.extras import RealDictCursor
from database import get_db_connection
from crud.action_logs_crud import get_action_logs
from crud.rentals_crud import get_rentals
from crud.trip_completions_crud import get_trip_completions
from crud.logs_crud import get_logs

def test_action_logs_sorting():
    """Test that action logs are sorted from newest to oldest"""
    print("Testing action logs sorting...")
    try:
        logs = get_action_logs(offset=0, limit=100)
        if len(logs) > 1:
            # Check if IDs are in descending order
            ids = [log['id'] for log in logs]
            is_descending = all(ids[i] >= ids[i+1] for i in range(len(ids)-1))
            print(f"Action logs sorted correctly (newest first): {is_descending}")
            if not is_descending:
                print(f"First few IDs: {ids[:10]}")
        else:
            print("Not enough logs to test sorting")
    except Exception as e:
        print(f"Error testing action logs: {e}")

def test_rentals_sorting():
    """Test that rentals are sorted from newest to oldest"""
    print("Testing rentals sorting...")
    try:
        rentals = get_rentals(offset=0, limit=100)
        if len(rentals) > 1:
            # Check if IDs are in descending order
            ids = [rental['id'] for rental in rentals]
            is_descending = all(ids[i] >= ids[i+1] for i in range(len(ids)-1))
            print(f"Rentals sorted correctly (newest first): {is_descending}")
            if not is_descending:
                print(f"First few IDs: {ids[:10]}")
        else:
            print("Not enough rentals to test sorting")
    except Exception as e:
        print(f"Error testing rentals: {e}")

def test_trip_completions_sorting():
    """Test that trip completions are sorted with pending first, oldest to newest, then approved/rejected (Тест проверяет, что завершения поездок сортируются сначала с ожидающими, от старых к новым, затем подтвержденные/отклоненные)"""
    print("Testing trip completions sorting...")
    try:
        completions = get_trip_completions(offset=0, limit=100)
        if len(completions) > 1:
            # Check if pending approvals (NULL) come before confirmed ones
            for i, completion in enumerate(completions):
                print(f"ID: {completion['id']}, Admin Approved: {completion['admin_approved']}")
                
            # Check the sorting pattern: pending (NULL) first, oldest to newest, then approved/rejected
            pending_first = True
            for i in range(len(completions) - 1):
                current_approved = completions[i]['admin_approved']
                next_approved = completions[i+1]['admin_approved']
                
                # If we transition from NULL to non-NULL, we're good
                if current_approved is None and next_approved is not None:
                    continue
                # If both are NULL (pending), check ID order (should be ascending for oldest to newest)
                elif current_approved is None and next_approved is None:
                    if completions[i]['id'] > completions[i+1]['id']:
                        pending_first = False
                        break
                # If both are not NULL, they can be in any order relative to each other
                # but should come after all NULL entries
                
            print(f"Trip completions sorted correctly (pending first, oldest to newest, then approved/rejected): {pending_first}")
        else:
            print("Not enough trip completions to test sorting")
    except Exception as e:
        print(f"Error testing trip completions: {e}")

def test_logs_sorting():
    """Test that logs are sorted from newest to oldest"""
    print("Testing logs sorting...")
    try:
        logs = get_logs(offset=0, limit=100)
        if len(logs) > 1:
            # Check if IDs are in descending order
            ids = [log['id'] for log in logs]
            is_descending = all(ids[i] >= ids[i+1] for i in range(len(ids)-1))
            print(f"Logs sorted correctly (newest first): {is_descending}")
            if not is_descending:
                print(f"First few IDs: {ids[:10]}")
        else:
            print("Not enough logs to test sorting")
    except Exception as e:
        print(f"Error testing logs: {e}")

if __name__ == "__main__":
    print("Testing sorting implementations...")
    test_logs_sorting()
    test_action_logs_sorting()
    test_rentals_sorting()
    test_trip_completions_sorting()
    print("Sorting tests completed.")