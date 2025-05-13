#pragma once
#include "DataObjectFactory.h"
#include "Student.h"
class StudentFactory:public DataObjectFactory
{
public:
	DataObject* CreateObject(std::vector<std::string>);
};

