#include "FileService.h"

const std::string FileService::path = "data.json";
StudentFactory* FileService::sf = new StudentFactory();
void FileService::save(std::vector<Student>v)
{
	nlohmann::json obj;
	std::ofstream fout(path);
	if (!fout.is_open())
	{
		throw std::invalid_argument("Can`t save data to the file");
	}
	for (auto& i : v)
	{
		obj[std::to_string(i.getId())] = { i.getName(),i.getMarks() };
		
	}
	fout << obj.dump(4);
	fout.close();
}

std::vector<Student> FileService::load()
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Can`t read data from the file");
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string json_string = buffer.str();
	file.close();
	try {
		std::vector<Student>ans;
		nlohmann::json j = nlohmann::json::parse(json_string);
		for (auto& [key, value] : j.items()) 
		{
			std::vector<std::string>a;
			a.push_back(key);
			a.push_back(value[0].get<std::string>());
			std::vector<int>m=(value[1].get<std::vector<int>>());
			for (auto val : m)
				a.push_back(std::to_string(val));

			DataObject* obj = sf->CreateObject(a);
			Student* tmpPtr = dynamic_cast<Student*>(obj); 
			ans.push_back(*tmpPtr);
		}
		return ans;
	}
	catch (...)
	{
		throw std::invalid_argument("Incorrect data in the file");
	}
}

void FileService::createDataFile()
{
	std::ofstream out(path);
	out.close();
}