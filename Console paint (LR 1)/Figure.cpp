#include "Figure.h"

void Figure::move(int x, int y)
{
	for (auto& i : vertex)
	{
		if(i.first + x < 0 || i.first + x > 59)
			throw std::invalid_argument("Invalid move argument. Figure vertices should located on OX 0..59");
		if (i.second + y < 0 || i.second + y > 19)
			throw std::invalid_argument("Invalid move argument. Figure vertices should located on OY 0..19");
	}
	
	for (auto& i : vertex)
	{
		i.first += x;
		i.second += y;
	}
}

bool Figure::getIsFill()
{
	return isFill;
}
