#include <iostream>

#pragma once

class Logger {
public:
    Logger(const std::string& service_name);
    void log(const std::string& message, const int log_level=1);
private:
    std::string service_name;

};