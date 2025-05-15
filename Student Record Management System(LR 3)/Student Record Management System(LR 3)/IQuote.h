#pragma once
#include "QuoteDTO.h"
class IQuote
{
public:
	virtual QuoteDTO* getQuote() = 0;
};

