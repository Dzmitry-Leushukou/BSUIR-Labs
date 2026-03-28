#include "server.h"

std::string Server::make_query(const std::string& req, const std::string& login, const std::string& password) {
    logger->log("Make query: " + req + " " + login + " " + password);
    std::string qlogin = login;
    std::string qpassword = password;
    if(isSQLInjectionDef)
    {

        if(check_sql_injection(qlogin) || check_sql_injection(qpassword))
        {
            return "SQL injection detected. Query doesn`t be executed";
        }

        qlogin = make_safe_string(qlogin);
        qpassword = make_safe_string(qpassword);

    }

    if (req == "register") {
        return db_service->register_user(qlogin, qpassword);
    } else if (req == "login") {
        return db_service->login_user(qlogin, qpassword);
    } else if (req == "update")
    {
        return db_service->update_user(qlogin, qpassword);
    }
    else if(req=="delete")
    {
        return db_service->delete_user(qlogin, qpassword);
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
        std::string login="";
        std::string password="";
        for(int i = 0; i < std::min((int)bytes_read,BUFFER_SIZE); i++)
        {
            if(buffer[i] == ' ')
            {
                if(req=="")
                    req=s,s="";
                else if (login=="")
                    login=s,s="";
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
        else if (login=="")
            login=s;
        else if (password=="")
            password=s;
        
        std::string str_response = make_query(req,login,password);
        const char *response = str_response.c_str();
        send(new_socket, response, strlen(response), 0);
        
        close(new_socket);
    }
    close(server_fd);
}


std::string Server::make_safe_string(const std::string& str) {
    logger->log("Make string\"" + str + "\" more safer");
    std::string safe_str = str;

    logger->log("Escape single quotes");
    for(int i = 0; i < safe_str.length(); i++) {
        if(safe_str[i] == '\'') {
            safe_str.insert(i, "'");
            i++;
        }
    }

    return safe_str;
}


bool Server::check_sql_injection(const std::string& str) {
    logger->log("Check SQL injection in string\"" + str + "\"");
    std::string upper_str = str;
    for(auto& c : upper_str) c = toupper(c);

    std::string dangerous_list[] = {
        "SELECT", "INSERT", "UPDATE", "DELETE", "CREATE", "DROP",
        "ALTER", "GRANT", "REVOKE", "EXECUTE", "DECLARE", "TRUNCATE",
        "RENAME", "UNION", "MERGE", "CALL", "FETCH"
    };

    for(auto& dangerous_word : dangerous_list) {
        size_t pos = 0;
        while((pos = upper_str.find(dangerous_word, pos)) != std::string::npos) {
            bool before_ok = (pos == 0 || !isalpha(upper_str[pos-1]));
            bool after_ok = (pos + dangerous_word.length() == upper_str.length() || 
                            !isalpha(upper_str[pos + dangerous_word.length()]));
            
            if(before_ok && after_ok) {
                logger->log("SQL injection detected: " + dangerous_word);
                return true;
            }
            pos += dangerous_word.length();
        }
    }
    
    if(str.find("--") != std::string::npos ||
       str.find(";") != std::string::npos) {
        logger->log("SQL injection detected: comment or multiple queries");
        return true;
    }
    

    size_t quote_pos = upper_str.find('\'');
    if(quote_pos != std::string::npos) {
        std::string after_quote = upper_str.substr(quote_pos);
        
        if(after_quote.find("' OR '") != std::string::npos ||
           after_quote.find("' AND '") != std::string::npos) {
            logger->log("SQL injection detected: quote-operator-quote pattern");
            return true;
        }
        
        if((after_quote.find("' OR ") != std::string::npos ||
            after_quote.find("' AND ") != std::string::npos) &&
           after_quote.find("=") != std::string::npos) {
            logger->log("SQL injection detected: OR/AND with equals");
            return true;
        }
    }
        
    return false;
}