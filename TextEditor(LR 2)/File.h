#pragma once
#include <string>
#include <fstream>
#include <vector>

#include "FileSaver.h"

class File
{
public:
	File(std::string);
	void update(std::string);
	std::vector<std::string> to_string() const;
	void save();
	void saveAs(FileSaver* saver, std::string);
	std::string getPath() const;
private:
	std::string replaceExtension(std::string new_ext);

	std::string path;
	std::vector<std::string> text;
};

