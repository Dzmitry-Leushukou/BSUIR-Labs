#include <sqlite3.h>
#include <string>
#include "logger.h"

class DBService
{
public:
    DBService();
    ~DBService();

    std::string register_user(const std::string& email, const std::string& password);
    std::string login_user(const std::string& email, const std::string& password);
    std::string update_user(const std::string& email, const std::string& password);
    std::string delete_user(const std::string& email, const std::string& password);

private:
    sqlite3 *db_pointer;
    const char* SQL_INIT_DB="CREATE TABLE IF NOT EXISTS\
     users(\
     id int primary key,\
     email text unique, \
     password text\
     )";
    Logger* logger=nullptr;
    const char* db_name="lab.db";
};