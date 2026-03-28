#include "logger.h"

Logger::Logger(const std::string& service_name) 
{
        this->service_name = service_name;
        std::cout << "Logger created" << std::endl;
}

void Logger::log(const std::string& message, const int log_level)
{
    switch (log_level)
    {
    case 1:
        std::cout<<"[INFO] ";
        break;
    case 2:
        std::cout<<"[WARNING] ";
        break;
    
    default:
        std::cout<<"[ERROR] ";
        break;
    }

    std::cout << service_name << ": ";
    std::cout << message << std::endl;
}