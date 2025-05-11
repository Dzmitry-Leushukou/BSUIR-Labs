#include "Checker.h"

bool Checker::checkId(std::string ch)
{
	try
	{
		stoi(ch);
		return true;
	}
	catch (...) {
		return false;
	}
}
bool Checker::checkMarks(std::vector<std::string>v)
{
	try
	{
		for(auto& ch:v)
		stoi(ch);
		return true;
	}
	catch (...) {
		return false;
	}
}
