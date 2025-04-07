#pragma once

#include <string>
#include <vector>
#include <fstream>

class FileSaver
{
public:
	virtual void save(std::vector<std::string> content, std::string filepath) = 0;
};

