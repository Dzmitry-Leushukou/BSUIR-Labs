#pragma once
#ifndef T_H
#define T_H

#include<string>

struct T
{

	std::string name;
	long long kol = 0;
	long double price = 0;
	std::string dat;
	long long expiration = 0;


	bool operator == (T& b);
	
};

#endif