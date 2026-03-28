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
    if (email == "" || password == "")
    {
        logger->log("Email or password is empty",3);
        return "Email or password is empty";
    }


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
    if (email == "" || password == "")
    {
        logger->log("Email or password is empty",3);
        return "Email or password is empty";
    }

    char * err_msg = nullptr;
    int count = 0;
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        int* result = static_cast<int*>(data);
        (*result)++;
        return 0;
    };
    
    
    std::string query="SELECT * FROM users WHERE email='" + email + "' AND password='" + password + "'";

    logger->log("Login user: " + email + " " + password);
    int query_status = sqlite3_exec(db_pointer, query.c_str(), callback, &count, &err_msg);
    logger->log("Status of query: " + std::to_string(query_status), 2);
    if(query_status != SQLITE_OK || count==0)
    {
        if (count==0)
        {
            logger->log("User not found",3);
            sqlite3_free(err_msg);
            return "User not found";
        }
        logger->log(err_msg,3);
        sqlite3_free(err_msg);
        return "Failed to login user";
    }
    sqlite3_free(err_msg);
    return "User logged is successfully";
     
}

std::string DBService::delete_user(const std::string& email, const std::string& password)
{
    if (email == "" || password == "")
    {
        logger->log("Email or password is empty",3);
        return "Email or password is empty";
    }

    char* err_msg = nullptr;
    std::string query="DELETE FROM users WHERE email='" + email + "' AND password='" + password + "'";

    logger->log("Delete user: " + email + " " + password);
    int query_status = sqlite3_exec(db_pointer, query.c_str(), NULL, NULL, &err_msg);
    logger->log("Status of query: " + std::to_string(query_status), 2);
    if(query_status != SQLITE_OK)
    {
        logger->log(err_msg,3);
        sqlite3_free(err_msg);
        return "Failed to delete user";
    }
    sqlite3_free(err_msg);
    int rows_deleted = sqlite3_changes(db_pointer);
    logger->log("Rows deleted: " + std::to_string(rows_deleted), 2);
    
    if (rows_deleted > 0) {
        return "User deleted successfully";
    } 
    
    logger->log("User not found: " + email, 3);
    return "User not found";
    
}

std::string DBService::update_user(const std::string& email, const std::string& new_password)
{
    if (email == "" || new_password == "")
    {
        logger->log("Email or password is empty",3);
        return "Email or password is empty";
    }

    char* err_msg = nullptr;
    std::string query="UPDATE users SET password='" + new_password + "' WHERE email='" + email + "'";

    logger->log("Update user: " + email + " " + new_password);
    int query_status = sqlite3_exec(db_pointer, query.c_str(), NULL, NULL, &err_msg);
    logger->log("Status of query: " + std::to_string(query_status), 2);
    if(query_status != SQLITE_OK)
    {
        logger->log(err_msg,3);
        sqlite3_free(err_msg);
        return "Failed to update user";
    }
    sqlite3_free(err_msg);
    int rows_deleted = sqlite3_changes(db_pointer);
    logger->log("Rows updated: " + std::to_string(rows_deleted), 2);
    
    if (rows_deleted > 0) {
        return "User updated successfully";
    } 
    
    logger->log("User not found: " + email, 3);
    return "User not found";
          
}