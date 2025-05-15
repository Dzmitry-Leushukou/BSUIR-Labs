#pragma once
#include "QuoteDTO.h"
#include "DataObjectFactory.h"
class QuoteFactory :public DataObjectFactory
{
public:
	DataObject* CreateObject(std::vector<std::string>);
};

