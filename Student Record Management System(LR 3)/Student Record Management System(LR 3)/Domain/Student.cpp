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