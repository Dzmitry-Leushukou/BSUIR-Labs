#pragma once
#include <vector>
#include <string>
#include "StudentDTO.h"
class Checker
{
public:
	static bool checkId(std::string);
	static bool checkMarks(std::vector<std::string>);
};

