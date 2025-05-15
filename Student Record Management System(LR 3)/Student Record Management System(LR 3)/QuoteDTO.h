#pragma once
#include "DataObject.h"
#include <string>
#include <vector>

class QuoteDTO:public DataObject
{
public:
	QuoteDTO(std::vector<std::string>);
	std::string to_string()const;
private:
	std::string data,author;	
};

