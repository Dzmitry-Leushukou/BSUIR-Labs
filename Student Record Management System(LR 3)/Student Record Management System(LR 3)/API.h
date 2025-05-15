#pragma once
#include"curl/curl.h"
#include "QuoteFactory.h"
#include "nlohmann/json.hpp"
class API
{
public:
	static QuoteDTO* get();
private:
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
	static QuoteFactory* qf;
};

