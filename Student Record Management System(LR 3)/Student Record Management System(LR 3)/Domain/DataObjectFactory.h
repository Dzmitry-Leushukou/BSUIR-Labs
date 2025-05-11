#pragma once
#include <vector>
#include <string>
#include "DataObject.h"
class DataObjectFactory
{
public:
	virtual ~DataObjectFactory() = default;
	virtual DataObject* CreateObject(std::vector<std::string>) = 0;
};

