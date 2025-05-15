#include "QuoteDTO.h"

QuoteDTO::QuoteDTO(std::vector<std::string>v)
{
	data = v.at(0);
	author = v.at(1);
}

std::string QuoteDTO::to_string()const
{
	return data + "\n- " + author;
}