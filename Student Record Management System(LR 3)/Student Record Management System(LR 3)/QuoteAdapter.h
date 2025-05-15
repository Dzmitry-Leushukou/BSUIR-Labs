#pragma once
#include "IQuote.h"
#include "API.h"
class QuoteAdapter : public IQuote {
public:
    QuoteDTO* getQuote() override;
};
