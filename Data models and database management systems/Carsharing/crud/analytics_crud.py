"""
Analytics reports using MongoDB Aggregation Framework.
"""

from pymongo import MongoClient, ASCENDING, DESCENDING
from datetime import datetime, timedelta
from typing import List, Dict, Any, Optional
import csv
import io
import json

from mongo_client import get_mongo_db


def get_action_logs_collection():
    """Get action_logs collection."""
    db = get_mongo_db()
    return db['action_logs']


# =============================================================================
# 1. Статистика активности пользователей по периодам (день/неделя/месяц)
# =============================================================================

def get_user_activity_stats(
    period: str = 'day',
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None
) -> List[Dict[str, Any]]:
    """
    Get user activity statistics grouped by period.
    
    Args:
        period: 'day', 'week', or 'month'
        start_date: Filter from this date
        end_date: Filter until this date
    
    Returns:
        List of dicts with period, total_actions, unique_users, and actions breakdown
    """
    collection = get_action_logs_collection()
    
    # Define date format based on period
    date_format = {
        'day': '%Y-%m-%d',
        'week': '%Y-W%V',
        'month': '%Y-%m'
    }.get(period, '%Y-%m-%d')
    
    # Build match stage
    match_stage = {}
    if start_date or end_date:
        match_stage['created_at'] = {}
        if start_date:
            match_stage['created_at']['$gte'] = start_date
        if end_date:
            match_stage['created_at']['$lte'] = end_date
    
    pipeline = [
        {'$match': match_stage} if match_stage else {'$match': {}},
        {
            '$facet': {
                'by_period': [
                    {
                        '$group': {
                            '_id': {
                                '$dateToString': {'format': date_format, 'date': '$created_at'}
                            },
                            'total_actions': {'$sum': 1},
                            'unique_users': {'$addToSet': '$actor_user_id'},
                            'actions_by_type': {
                                '$push': '$action_type'
                            }
                        }
                    },
                    {
                        '$project': {
                            'period': '$_id',
                            'total_actions': 1,
                            'unique_users_count': {'$size': '$unique_users'},
                            'actions_by_type': {
                                '$let': {
                                    'vars': {
                                        'types': '$actions_by_type'
                                    },
                                    'in': {
                                        '$map': {
                                            'input': {'$setUnion': '$$types'},
                                            'as': 'type',
                                            'in': {
                                                'k': '$$type',
                                                'v': {
                                                    '$size': {
                                                        '$filter': {
                                                            'input': '$$types',
                                                            'as': 't',
                                                            'cond': {'$eq': ['$$t', '$$type']}
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            '_id': 0
                        }
                    },
                    {'$sort': {'period': -1}}
                ]
            }
        }
    ]
    
    result = collection.aggregate(pipeline)
    data = list(result)[0]['by_period']
    
    # Convert actions_by_type to proper format
    for item in data:
        if 'actions_by_type' in item:
            type_counts = {}
            for type_item in item['actions_by_type']:
                if isinstance(type_item, dict) and 'k' in type_item:
                    type_counts[type_item['k']] = type_item['v']
            item['actions_by_type'] = type_counts
    
    return data


# =============================================================================
# 2. ТОП-10 самых активных пользователей
# =============================================================================

def get_top_active_users(
    limit: int = 10,
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None
) -> List[Dict[str, Any]]:
    """
    Get top N most active users.
    
    Args:
        limit: Number of users to return
        start_date: Filter from this date
        end_date: Filter until this date
    
    Returns:
        List of dicts with user_id, email, total_actions, actions_by_type, last_action
    """
    collection = get_action_logs_collection()
    
    match_stage = {}
    if start_date or end_date:
        match_stage['created_at'] = {}
        if start_date:
            match_stage['created_at']['$gte'] = start_date
        if end_date:
            match_stage['created_at']['$lte'] = end_date
    
    pipeline = [
        {'$match': match_stage} if match_stage else {'$match': {}},
        {
            '$group': {
                '_id': {
                    'user_id': '$actor_user_id',
                    'email': '$actor_email'
                },
                'total_actions': {'$sum': 1},
                'actions_by_type': {
                    '$push': '$action_type'
                },
                'last_action': {'$max': '$created_at'},
                'first_action': {'$min': '$created_at'}
            }
        },
        {
            '$project': {
                'user_id': '$_id.user_id',
                'email': '$_id.email',
                'total_actions': 1,
                'actions_by_type': {
                    '$let': {
                        'vars': {
                            'types': '$actions_by_type'
                        },
                        'in': {
                            '$map': {
                                'input': {'$setUnion': '$$types'},
                                'as': 'type',
                                'in': {
                                    'k': '$$type',
                                    'v': {
                                        '$size': {
                                            '$filter': {
                                                'input': '$$types',
                                                'as': 't',
                                                'cond': {'$eq': ['$$t', '$$type']}
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                },
                'last_action': 1,
                'first_action': 1,
                '_id': 0
            }
        },
        {'$sort': {'total_actions': -1}},
        {'$limit': limit}
    ]
    
    result = collection.aggregate(pipeline)
    data = list(result)

    # Получаем коллекцию пользователей из MongoDB (если имеется)
    db = get_mongo_db()
    users_collection = db['users']

    # Convert actions_by_type to proper format и подставить email если отсутствует
    for item in data:
        if 'actions_by_type' in item:
            type_counts = {}
            for type_item in item['actions_by_type']:
                if isinstance(type_item, dict) and 'k' in type_item:
                    type_counts[type_item['k']] = type_item['v']
            item['actions_by_type'] = type_counts

        # Подставить email по user_id если отсутствует
        if (not item.get('email')) and item.get('user_id'):
            # Сначала пробуем взять из MongoDB users
            try:
                user_doc = users_collection.find_one({'_id': item['user_id']})
                if user_doc and user_doc.get('email'):
                    item['email'] = user_doc['email']
                else:
                    raise Exception('user not found in mongo')
            except Exception:
                # fallback: взять из Postgres
                try:
                    from crud.users_crud import get_user
                    user = get_user(item['user_id'])
                    if user and user.get('email'):
                        item['email'] = user.get('email')
                except Exception:
                    pass

    return data


# =============================================================================
# 3. Распределение операций по типам (CRUD-статистика)
# =============================================================================

def get_operations_distribution(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None
) -> Dict[str, Any]:
    """
    Get operations distribution by type with CRUD classification.
    
    Args:
        start_date: Filter from this date
        end_date: Filter until this date
    
    Returns:
        Dict with total_operations, by_type breakdown, and CRUD classification
    """
    collection = get_action_logs_collection()
    
    match_stage = {}
    if start_date or end_date:
        match_stage['created_at'] = {}
        if start_date:
            match_stage['created_at']['$gte'] = start_date
        if end_date:
            match_stage['created_at']['$lte'] = end_date
    
    # Define CRUD classification
    crud_mapping = {
        'Create': ['user_registration', 'car_create', 'driver_license_create', 
                   'driver_license_upload', 'trip_completion_create', 
                   'maintenance_request_create', 'car_photo_upload'],
        'Read': ['user_login', 'user_logout'],
        'Update': ['profile_update', 'car_update', 'driver_license_update',
                   'driver_license_approved', 'driver_license_rejected',
                   'car_rental_start', 'car_rental_end', 'car_rental_cancel',
                   'car_rental_pending_completion', 'trip_completion_approved',
                   'trip_completion_rejected', 'trip_completion_update',
                   'maintenance_request_update', 'user_status_change',
                   'car_status_change', 'payment_success', 'payment_failed'],
        'Delete': ['car_delete', 'driver_license_delete', 'user_ban', 
                   'user_unban', 'maintenance_request_delete', 'car_photo_delete']
    }
    
    pipeline = [
        {'$match': match_stage} if match_stage else {'$match': {}},
        {
            '$facet': {
                'by_type': [
                    {
                        '$group': {
                            '_id': '$action_type',
                            'count': {'$sum': 1},
                            'percentage': {'$sum': 1}
                        }
                    },
                    {'$sort': {'count': -1}}
                ],
                'total': [
                    {'$count': 'total'}
                ]
            }
        }
    ]
    
    result = list(collection.aggregate(pipeline))[0]
    
    total = result['total'][0]['total'] if result['total'] else 0
    
    # Process by_type
    by_type = []
    for item in result['by_type']:
        by_type.append({
            'action_type': item['_id'],
            'count': item['count'],
            'percentage': round((item['count'] / total * 100) if total > 0 else 0, 2)
        })
    
    # Calculate CRUD distribution
    crud_distribution = {'Create': 0, 'Read': 0, 'Update': 0, 'Delete': 0}
    for item in by_type:
        for crud_type, actions in crud_mapping.items():
            if item['action_type'] in actions:
                crud_distribution[crud_type] += item['count']
                break
    
    # Add percentages to CRUD
    for crud_type in crud_distribution:
        crud_distribution[crud_type] = {
            'count': crud_distribution[crud_type],
            'percentage': round((crud_distribution[crud_type] / total * 100) if total > 0 else 0, 2)
        }
    
    return {
        'total_operations': total,
        'by_type': by_type,
        'crud_distribution': crud_distribution
    }


# =============================================================================
# 4. Временные тренды (time series analysis)
# =============================================================================

def get_time_series_trends(
    period: str = 'hour',
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None
) -> Dict[str, Any]:
    """
    Get time series trends for activity analysis.
    
    Args:
        period: 'hour', 'day_of_week', or 'hour_of_day'
        start_date: Filter from this date
        end_date: Filter until this date
    
    Returns:
        Dict with trends data
    """
    collection = get_action_logs_collection()
    
    match_stage = {}
    if start_date or end_date:
        match_stage['created_at'] = {}
        if start_date:
            match_stage['created_at']['$gte'] = start_date
        if end_date:
            match_stage['created_at']['$lte'] = end_date
    
    if period == 'hour':
        group_id = {
            '$dateToString': {'format': '%Y-%m-%d %H:00', 'date': '$created_at'}
        }
        sort_order = {'_id': 1}
    elif period == 'day_of_week':
        group_id = {'$dayOfWeek': '$created_at'}
        sort_order = {'_id': 1}
    elif period == 'hour_of_day':
        group_id = {'$hour': '$created_at'}
        sort_order = {'_id': 1}
    else:
        group_id = {'$dateToString': {'format': '%Y-%m-%d', 'date': '$created_at'}}
        sort_order = {'_id': 1}
    
    pipeline = [
        {'$match': match_stage} if match_stage else {'$match': {}},
        {
            '$group': {
                '_id': group_id,
                'count': {'$sum': 1},
                'unique_users': {'$addToSet': '$actor_user_id'},
                'avg_actions_per_user': {
                    '$avg': {'$cond': [{'$eq': ['$actor_user_id', None]}, 0, 1]}
                }
            }
        },
        {
            '$project': {
                'period': '$_id',
                'count': 1,
                'unique_users_count': {'$size': '$unique_users'},
                '_id': 0
            }
        },
        {'$sort': sort_order}
    ]
    
    result = collection.aggregate(pipeline)
    data = list(result)
    
    # Calculate statistics
    if data:
        counts = [item['count'] for item in data]
        stats = {
            'total_periods': len(data),
            'avg_actions': sum(counts) / len(counts),
            'max_actions': max(counts),
            'min_actions': min(counts),
            'trend_data': data
        }
    else:
        stats = {
            'total_periods': 0,
            'avg_actions': 0,
            'max_actions': 0,
            'min_actions': 0,
            'trend_data': []
        }
    
    return stats


# =============================================================================
# 5. Аномалии в поведении пользователей
# =============================================================================

def detect_user_anomalies(
    start_date: Optional[datetime] = None,
    end_date: Optional[datetime] = None,
    std_threshold: float = 2.0
) -> List[Dict[str, Any]]:
    """
    Detect anomalies in user behavior using statistical analysis.
    
    Args:
        start_date: Filter from this date
        end_date: Filter until this date
        std_threshold: Number of standard deviations for anomaly detection
    
    Returns:
        List of anomalies with user details and anomaly type
    """
    collection = get_action_logs_collection()
    
    match_stage = {}
    if start_date or end_date:
        match_stage['created_at'] = {}
        if start_date:
            match_stage['created_at']['$gte'] = start_date
        if end_date:
            match_stage['created_at']['$lte'] = end_date
    
    # First, calculate user statistics
    user_stats_pipeline = [
        {'$match': match_stage} if match_stage else {'$match': {}},
        {
            '$group': {
                '_id': {
                    'user_id': '$actor_user_id',
                    'email': '$actor_email'
                },
                'action_count': {'$sum': 1},
                'unique_action_types': {'$addToSet': '$action_type'},
                'actions_per_day': {
                    '$push': {
                        'date': {'$dateToString': {'format': '%Y-%m-%d', 'date': '$created_at'}},
                        'count': 1
                    }
                }
            }
        },
        {
            '$project': {
                'user_id': '$_id.user_id',
                'email': '$_id.email',
                'action_count': 1,
                'unique_action_types_count': {'$size': '$unique_action_types'},
                '_id': 0
            }
        }
    ]
    
    user_stats = list(collection.aggregate(user_stats_pipeline))
    
    if not user_stats:
        return []
    
    # Calculate mean and std for action counts
    counts = [u['action_count'] for u in user_stats]
    mean_count = sum(counts) / len(counts)
    variance = sum((x - mean_count) ** 2 for x in counts) / len(counts)
    std_count = variance ** 0.5
    
    anomalies = []
    
    for user in user_stats:
        anomaly_types = []
        
        # Check for unusually high activity
        if std_count > 0 and (user['action_count'] - mean_count) / std_count > std_threshold:
            anomaly_types.append('high_activity')
        
        # Check for unusually low activity (if user has some actions but very few)
        if std_count > 0 and mean_count - user['action_count'] > std_threshold * std_count and user['action_count'] > 0:
            anomaly_types.append('low_activity')
        
        # Check for diverse action types (potential automated behavior)
        if user['unique_action_types_count'] > 10:
            anomaly_types.append('diverse_actions')
        
        if anomaly_types:
            anomalies.append({
                'user_id': user['user_id'],
                'email': user['email'],
                'action_count': user['action_count'],
                'unique_action_types_count': user['unique_action_types_count'],
                'anomaly_types': anomaly_types,
                'deviation': round((user['action_count'] - mean_count) / std_count, 2) if std_count > 0 else 0
            })
    
    # Sort by deviation
    anomalies.sort(key=lambda x: abs(x['deviation']), reverse=True)
    
    return {
        'anomalies': anomalies,
        'statistics': {
            'mean_actions_per_user': round(mean_count, 2),
            'std_deviation': round(std_count, 2),
            'threshold': std_threshold,
            'total_users_analyzed': len(user_stats),
            'anomalies_detected': len(anomalies)
        }
    }


# =============================================================================
# 6. Экспорт отчётов
# =============================================================================

def export_to_json(data: Any, filename: Optional[str] = None) -> str:
    """
    Export data to JSON format.
    
    Args:
        data: Data to export
        filename: Optional filename (returns string if not provided)
    
    Returns:
        JSON string or saves to file
    """
    json_str = json.dumps(data, indent=2, default=str)
    
    if filename:
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(json_str)
        return f"Exported to {filename}"
    
    return json_str


def export_to_csv(
    data: List[Dict[str, Any]], 
    filename: Optional[str] = None
) -> str:
    """
    Export data to CSV format.
    
    Args:
        data: List of dicts to export
        filename: Optional filename (returns string if not provided)
    
    Returns:
        CSV string or saves to file
    """
    if not data:
        return ""
    
    output = io.StringIO()
    writer = csv.DictWriter(output, fieldnames=data[0].keys())
    writer.writeheader()
    writer.writerows(data)
    
    csv_str = output.getvalue()
    
    if filename:
        with open(filename, 'w', encoding='utf-8', newline='') as f:
            f.write(csv_str)
        return f"Exported to {filename}"
    
    return csv_str
