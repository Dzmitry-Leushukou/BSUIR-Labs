#include "StudentDTO.h"
StudentDTO::StudentDTO(std::string s)
{
	name = s;
}
StudentDTO::StudentDTO(std::string s, std::vector<int>v)
{
	name = s;
	marks = v;
}
std::string StudentDTO::getName() const
{
	return name;
}
std::vector<int> StudentDTO::getMarks() const
{
	return marks;
}
int StudentDTO::getId()const
{
	return id;
}
void StudentDTO::setId(int v)
{
	id = v;
}
void StudentDTO::setName(std::string s)
{
	name = s;
}
void StudentDTO::setMarks(std::vector<int>v)
{
	marks = v;
}
StudentDTO::StudentDTO(int id, std::string name, std::string marks)
{
	this->id = id;
	this->name = name;
	std::vector<int>v;
	std::string tmp;
	for (auto& i : marks)
	{
		if (i == ',')
		{
			if (tmp == "")
				throw std::invalid_argument("Wrong marks string");
			v.push_back(std::stoi(tmp));
			tmp = "";
		}
		else
			if (i >= '0' && i <= '9')
			{
				tmp += i;
			}
			else
				throw std::invalid_argument("Wrong marks string");
	}
	if (tmp != "")
		v.push_back(std::stoi(tmp));
	this->marks = v;
}
