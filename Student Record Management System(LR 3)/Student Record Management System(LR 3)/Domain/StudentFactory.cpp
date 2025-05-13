#include "StudentFactory.h"

DataObject* StudentFactory::CreateObject(std::vector<std::string>v)
{
	return new Student(v);
}