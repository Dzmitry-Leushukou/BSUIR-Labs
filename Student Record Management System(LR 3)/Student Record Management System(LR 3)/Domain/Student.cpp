#include "Student.h"
Student::Student(int id,std::string s)
{
	this->id = id;
	name = s;
}
Student::Student(int id, std::string s, std::vector<int>v)
{
	this->id = id;
	name = s;
	marks = v;
}
std::string Student::getName() const
{
	return name;
}
std::vector<int> Student::getMarks() const
{
	return marks;
}
void Student::setName(std::string s)
{
	name = s;
}
void Student::setMarks(std::vector<int>v)
{
	marks = v;
}
void Student::setId(int id)
{
	this->id = id;
}
int Student::getId()
{
	return id;
}
std::string Student::to_string()const
{
	std::string s;
	s = std::to_string(id) + " | " + name + "\n==Marks==\n";
	for (auto& i : marks)
	{
		s += std::to_string(i) + "\n";
	}
	return s;
}
std::vector<std::string>Student::to_vector()const
{
	std::vector<std::string>v;
	v.push_back(name);
	std::string m;
	for (auto& i : marks)
	{
		m += std::to_string(i) + ",";
	}
	if(!m.empty())
		m.pop_back();
	v.push_back(m);
	return v;
}
Student::Student(int id, std::string name, std::string marks)
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