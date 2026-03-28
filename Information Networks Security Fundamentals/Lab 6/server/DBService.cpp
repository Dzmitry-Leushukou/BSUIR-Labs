#include "DBService.h"


DBService::DBService()
{
    logger = new Logger("DBService");
    logger->log("Try to open database");
    int open_status=sqlite3_open(db_name, &db_pointer);
    
    if(open_status != SQLITE_OK)
    {
        logger->log("Database not exist",3);
        exit(1);
    }

    logger->log("Database opened");

    char * err_msg = nullptr;
    int exec_status=sqlite3_exec(db_pointer, SQL_INIT_DB, NULL, NULL, &err_msg);

    if(exec_status != SQLITE_OK)
    {
        logger->log(err_msg,3);
        logger->log("Clear memory for error message",3);
        sqlite3_free(err_msg);
        logger->log("Try to close database connection",3);
        sqlite3_close(db_pointer);
        logger->log("Database connection closed",3);
        exit(2);
    }
    sqlite3_free(err_msg);
    logger->log("Database successfuly initialized");
}

DBService::~DBService()
{
    sqlite3_close(db_pointer);
}

std::string DBService::register_user(const std::string& email, const std::string& password)
{
    char * err_msg = nullptr;

    std::string query="INSERT INTO users (email, password) VALUES ('" + email + "', '" + password + "')";

    logger->log("Register user: " + email + " " + password);
    int query_status=sqlite3_exec(db_pointer, query.c_str(), NULL, NULL, &err_msg);
    logger->log("Status of query: " + std::to_string(query_status), 2);
    if(query_status != SQLITE_OK)
    {
        logger->log(err_msg,3);
        sqlite3_free(err_msg);
        return "Failed to register user";
    }
    
    sqlite3_free(err_msg);
    return "User registered successfully";       
}

std::string DBService::login_user(const std::string& email, const std::string& password)
{
    
    return "User logged in successfully";       
}

std::string DBService::delete_user(const std::string& email, const std::string& password)
{
    
    return "User deleted successfully";       
}

std::string DBService::update_user(const std::string& email, const std::string& password)
{
    
    return "User updated successfully";       
}