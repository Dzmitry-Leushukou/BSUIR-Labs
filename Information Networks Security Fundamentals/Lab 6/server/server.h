#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "DBService.h"
#include "logger.h"


class Server
{
    public:
    Server(bool isSQLInjectionDef=false);
    ~Server()=default;
    
    void run();
    private:
    std::string make_query(const std::string& req, const std::string& login, const std::string& password);
    std::string make_safe_string(const std::string& str);
    bool check_sql_injection(const std::string& str);

    const int PORT = 8080;
    const int BUFFER_SIZE = 1024;
    Logger* logger=nullptr;
    DBService* db_service=nullptr;
    bool isSQLInjectionDef;
};