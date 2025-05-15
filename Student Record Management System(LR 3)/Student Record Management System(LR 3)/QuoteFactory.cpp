#include "QuoteFactory.h"

DataObject* QuoteFactory::CreateObject(std::vector<std::string>v)
{
	return new QuoteDTO(v);
}