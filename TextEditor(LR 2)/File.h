#pragma once
#include <string>
#include <fstream>
#include <vector>

class File
{
public:
	File(std::string);
	void update(std::string);
	std::vector<std::string> to_string() const;
	void save();
	std::string getPath() const;
private:
	std::string path;
	std::vector<std::string> text;
};

