"""
Middleware for logging database queries and application errors to MongoDB.
"""

from fastapi import Request, Response
from starlette.middleware.base import BaseHTTPMiddleware
from typing import Callable
import time
import traceback
import re
import os

from crud.mongo_logs_crud import create_db_query_log_mongo, create_error_log_mongo
from mongo_client import get_mongo_db


class MongoDBLoggingMiddleware(BaseHTTPMiddleware):
    """Middleware for logging errors to MongoDB."""
    
    async def dispatch(self, request: Request, call_next: Callable) -> Response:
        """Log errors and exceptions to MongoDB."""
        try:
            response = await call_next(request)
            
            # Log server errors
            if response.status_code >= 500:
                await self.log_error(
                    request=request,
                    error_type="HTTPException",
                    error_message=f"HTTP {response.status_code} error",
                    severity="ERROR",
                )
            
            return response
        except Exception as e:
            # Log exception to MongoDB
            await self.log_error(
                request=request,
                error_type=type(e).__name__,
                error_message=str(e),
                stack_trace=traceback.format_exc(),
                severity="CRITICAL",
            )
            raise
    
    async def log_error(
        self,
        request: Request,
        error_type: str,
        error_message: str,
        stack_trace: str = None,
        severity: str = "ERROR",
    ):
        """Log error to MongoDB."""
        try:
            # Check if MongoDB is available
            db = get_mongo_db()
            if db is None:
                return
            
            # Extract user info from request state (if available)
            user_id = None
            user_email = None
            if hasattr(request, "state") and hasattr(request.state, "current_user"):
                user_id = request.state.current_user.get("id")
                user_email = request.state.current_user.get("email")
            
            create_error_log_mongo(
                error_type=error_type,
                error_message=error_message,
                stack_trace=stack_trace,
                endpoint=request.url.path,
                user_id=user_id,
                user_email=user_email,
                request_method=request.method,
                request_url=str(request.url),
                ip_address=request.client.host if request.client else None,
                severity=severity,
            )
        except Exception as e:
            # Don't fail the request if logging fails
            print(f"Failed to log error to MongoDB: {str(e)}")


class DatabaseQueryLoggingMiddleware:
    """
    Middleware-like wrapper for logging database queries.
    This is used by monkey-patching the psycopg2 cursor.
    """
    
    _enabled = False
    _current_endpoint = None
    _current_user_id = None
    
    @classmethod
    def enable(cls):
        """Enable query logging."""
        cls._enabled = True
    
    @classmethod
    def disable(cls):
        """Disable query logging."""
        cls._enabled = False
    
    @classmethod
    def set_context(cls, endpoint: str = None, user_id: int = None):
        """Set the current request context for query logging."""
        cls._current_endpoint = endpoint
        cls._current_user_id = user_id
    
    @classmethod
    def clear_context(cls):
        """Clear the current request context."""
        cls._current_endpoint = None
        cls._current_user_id = None
    
    @classmethod
    def log_query(
        cls,
        query: str,
        execution_time_ms: float,
        rows_affected: int = None,
    ):
        """Log a database query to MongoDB."""
        if not cls._enabled:
            return
        
        try:
            # Skip logging for certain queries
            if cls._should_skip_query(query):
                return
            
            # Determine query type
            query_type = cls._get_query_type(query)
            
            # Extract table name
            table_name = cls._extract_table_name(query, query_type)
            
            # Check if MongoDB is available
            db = get_mongo_db()
            if db is None:
                return
            
            create_db_query_log_mongo(
                query=query,
                query_type=query_type,
                table_name=table_name,
                execution_time_ms=execution_time_ms,
                rows_affected=rows_affected,
                user_id=cls._current_user_id,
                endpoint=cls._current_endpoint,
            )
        except Exception as e:
            # Don't fail the query if logging fails
            print(f"Failed to log query to MongoDB: {str(e)}")
    
    @staticmethod
    def _should_skip_query(query: str) -> bool:
        """Check if query should be skipped from logging."""
        # Skip internal PostgreSQL queries
        skip_patterns = [
            r'^SELECT\s+pg_isready',
            r'^SELECT\s+version\(\)',
            r'^SHOW\s+',
            r'^SET\s+',
            r'^BEGIN\s+',
            r'^COMMIT\s+',
            r'^ROLLBACK\s+',
        ]
        
        query_upper = query.upper().strip()
        for pattern in skip_patterns:
            if re.match(pattern, query_upper, re.IGNORECASE):
                return True
        
        return False
    
    @staticmethod
    def _get_query_type(query: str) -> str:
        """Extract query type from SQL query."""
        query_upper = query.strip().upper()
        
        if query_upper.startswith("SELECT"):
            return "SELECT"
        elif query_upper.startswith("INSERT"):
            return "INSERT"
        elif query_upper.startswith("UPDATE"):
            return "UPDATE"
        elif query_upper.startswith("DELETE"):
            return "DELETE"
        elif query_upper.startswith("CREATE"):
            return "CREATE"
        elif query_upper.startswith("DROP"):
            return "DROP"
        elif query_upper.startswith("ALTER"):
            return "ALTER"
        else:
            return "OTHER"
    
    @staticmethod
    def _extract_table_name(query: str, query_type: str) -> str:
        """Extract table name from SQL query."""
        # Simple regex patterns for common query types
        patterns = {
            "SELECT": r'FROM\s+([a-zA-Z_][a-zA-Z0-9_]*)',
            "INSERT": r'INTO\s+([a-zA-Z_][a-zA-Z0-9_]*)',
            "UPDATE": r'UPDATE\s+([a-zA-Z_][a-zA-Z0-9_]*)',
            "DELETE": r'FROM\s+([a-zA-Z_][a-zA-Z0-9_]*)',
        }
        
        pattern = patterns.get(query_type)
        if pattern:
            match = re.search(pattern, query, re.IGNORECASE)
            if match:
                return match.group(1)
        
        return None


def setup_mongodb_logging():
    """Setup MongoDB logging - create TTL indexes."""
    from crud.mongo_logs_crud import setup_ttl_indexes
    setup_ttl_indexes()
