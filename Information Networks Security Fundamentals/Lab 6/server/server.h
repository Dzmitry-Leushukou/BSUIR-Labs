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
    std::string make_query(const std::string& req, const std::string& email, const std::string& password);
    const int PORT = 8080;
    const int BUFFER_SIZE = 1024;
    Logger* logger=nullptr;
    DBService* db_service=nullptr;
    bool isSQLInjectionDef;
};