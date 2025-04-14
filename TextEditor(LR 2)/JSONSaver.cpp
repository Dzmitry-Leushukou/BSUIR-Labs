#include "JSONSaver.h"

void JSONSaver::save(std::vector<std::string>content, std::string filepath)
{
	nlohmann::json obj;
	obj["content"] = content;
	std::ofstream outputFile(filepath);
	if (outputFile.is_open())
	{
		outputFile << obj.dump(4);
		outputFile.close();
	}
	else throw std::invalid_argument("Can`t create/open json file");
}