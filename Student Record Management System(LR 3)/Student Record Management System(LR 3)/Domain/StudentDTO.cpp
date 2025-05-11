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
