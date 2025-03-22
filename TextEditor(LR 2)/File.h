#pragma once
#include <string>
#include <fstream>
#include <vector>

class File
{
public:
	File(std::string);
	virtual ~File() {}
	void save();
	std::vector<std::string> to_string() const;
private:
	std::string path;
	std::vector<std::string> text;
};

