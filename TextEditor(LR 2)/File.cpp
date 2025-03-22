#include "File.h"

File::File(std::string PATH)
{
	path = PATH;

		std::ifstream fin;
		fin.open(path);
		std::string s;
		text.clear();
		while (std::getline(fin, s))
		{
			text.push_back(s);
		}

		fin.close();
}

std::vector<std::string> File::to_string() const
{
	return text;
}