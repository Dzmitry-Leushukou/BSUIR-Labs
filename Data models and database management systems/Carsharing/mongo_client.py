from pymongo import MongoClient
from pymongo.errors import ConnectionFailure, ServerSelectionTimeoutError
import os
import time


class MongoDBClient:
    """MongoDB client singleton with connection retry logic."""
    
    _instance = None
    _client = None
    _db = None
    
    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(MongoDBClient, cls).__new__(cls)
        return cls._instance
    
    def __init__(self):
        if self._client is None:
            self._connect()
    
    def _connect(self):
        """Connect to MongoDB with retry logic."""
        max_retries = 5
        retries = 0
        
        host = os.getenv("MONGODB_HOST", "localhost")
        port = os.getenv("MONGODB_PORT", 27017)
        db_name = os.getenv("MONGODB_DB", "carsharing_logs")
        
        while retries < max_retries:
            try:
                self._client = MongoClient(
                    host=host,
                    port=int(port),
                    serverSelectionTimeoutMS=5000,
                    connectTimeoutMS=10000,
                    socketTimeoutMS=10000,
                )
                # Test connection
                self._client.admin.command('ping')
                self._db = self._client[db_name]
                print(f"Successfully connected to MongoDB: {host}:{port}/{db_name}")
                return
            except (ConnectionFailure, ServerSelectionTimeoutError) as e:
                print(f"MongoDB connection failed, retrying in 2 seconds... (attempt {retries+1}/{max_retries}): {str(e)}")
                time.sleep(2)
                retries += 1
        
        raise ConnectionFailure("Could not connect to MongoDB after maximum retries")
    
    @property
    def client(self):
        """Get MongoDB client."""
        return self._client
    
    @property
    def db(self):
        """Get MongoDB database."""
        return self._db
    
    def get_collection(self, collection_name):
        """Get a collection from the database."""
        return self._db[collection_name]
    
    def is_connected(self):
        """Check if connected to MongoDB."""
        if self._client is None:
            return False
        try:
            self._client.admin.command('ping')
            return True
        except:
            return False


def get_mongo_client():
    """Get MongoDB client instance."""
    return MongoDBClient()


def get_mongo_db():
    """Get MongoDB database instance."""
    return MongoDBClient().db
