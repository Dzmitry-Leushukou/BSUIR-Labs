#pragma once
#include <string>
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include "StudentFactory.h"
class FileService
{
public:
	static void save(std::vector<Student>);
	static std::vector<Student> load();
	static void createDataFile();
private:
	static const std::string path;
	static StudentFactory*sf;
};
