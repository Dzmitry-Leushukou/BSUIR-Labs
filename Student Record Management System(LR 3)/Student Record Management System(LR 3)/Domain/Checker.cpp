#include "Checker.h"

bool Checker::checkId(std::string ch)
{
	try
	{
		stoi(ch);
		for (auto& i : ch)
		{
			if (i < '0' || i>'9')
				throw std::invalid_argument("incorrect id");
		}
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
		for (auto& ch : v)
		{
			stoi(ch);
			for (auto& i : ch)
			if (i < '0' || i>'9')
				throw std::invalid_argument("incorrect id");
		}
		return true;
	}
	catch (...) {
		return false;
	}
}
