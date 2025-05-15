#include "QuoteAdapter.h"
QuoteDTO* QuoteAdapter::getQuote()
{
	return API::get();
}