#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>

class History
{
public:
	static void createdFile(std::string user, std::string filepath);
	static void savedFile(std::string user, std::string filepath);
	static std::string getNotify(std::string filepath);
	static std::string getTime();
private:
	static std::string filesaver; 
	static std::time_t parseDateTime(const std::string& dateTimeStr);
	
};

