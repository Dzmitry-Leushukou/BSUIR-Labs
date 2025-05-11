#pragma once
#include "DataObjectFactory.h"
class StudentFactory:public DataObjectFactory
{
public:
	DataObject* CreateObject(std::vector<std::string>);
};

