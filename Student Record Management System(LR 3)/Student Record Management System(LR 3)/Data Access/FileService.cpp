#include "FileService.h"

const std::string FileService::path = "data.json";

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
		for (auto& [key, value] : j.items()) {
			Student tmp(stoi(key),value[0].get<std::string>(),value[1].get<std::vector<int>>());
			ans.push_back(tmp);
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