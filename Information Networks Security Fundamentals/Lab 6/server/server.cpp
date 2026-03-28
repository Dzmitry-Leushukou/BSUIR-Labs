#include "server.h"

std::string Server::make_query(const std::string& req, const std::string& email, const std::string& password) {
    logger->log("Make query: " + req + " " + email + " " + password);

    if(isSQLInjectionDef)
    {

    }

    if (req == "register") {
        return db_service->register_user(email, password);
    } else if (req == "login") {
        return db_service->login_user(email, password);
    } else if (req == "update")
    {
        return db_service->update_user(email, password);
    } 
    else if(req=="delete")
    {
        return db_service->delete_user(email, password);
    }
    else {
        return "Invalid request";
    }
}


Server::Server(bool isSQLInjectionDef) {
    logger = new Logger("Server");
    logger->log("Try to init DBService");
    db_service = new DBService();
    logger->log("DBService initialized");
    this->isSQLInjectionDef = isSQLInjectionDef;
    logger->log("SQL injection defense: " + (isSQLInjectionDef == false? std::string("off") : std::string("on")));
    logger->log("Server initialized");
}


void Server::run() 
{
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    logger->log("Server started");

    while(true)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        logger->log("Client connected");

        ssize_t bytes_read = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read < 0) {
            perror("recv");
            close(new_socket);
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        buffer[bytes_read] = '\0';

        logger->log("Received request: " + std::string(buffer));

        std::string req="";
        std::string s="";
        std::string email="";
        std::string password="";
        for(int i = 0; i < std::min((int)bytes_read,BUFFER_SIZE); i++)
        {
            if(buffer[i] == ' ')
            {
                if(req=="")
                    req=s,s="";
                else if (email=="")
                    email=s,s="";
                    else s+=buffer[i];
                continue;
            }
            else
            {
                s+=buffer[i];
            }
        }

        if(req=="")
            req=s;
        else if (email=="")
            email=s;
        else if (password=="")
            password=s;
        
        std::string str_response = make_query(req,email,password);
        const char *response = str_response.c_str();
        send(new_socket, response, strlen(response), 0);
        
        close(new_socket);
    }
    close(server_fd);
}
