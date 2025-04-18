#pragma once
#include <string>
#include <curl/curl.h>
#include <stdexcept>	
#include <fstream>

class CloudService
{
public:
	static void open(std::string){}
	static void save(std::string);
	static void deleteFile(std::string){}
private:
	static std::string access_token;
	
};

