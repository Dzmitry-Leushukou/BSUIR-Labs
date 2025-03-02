#include "Figure.h"

void Figure::move(int x, int y)
{
	for (auto& i : vertex)
	{
		if(i.first + x < 0 || i.first + x > 19)
			throw std::invalid_argument("Invalid move argument. Figure vertices should located on OX 0..19");
		if (i.second + y < 0 || i.second + y > 59)
			throw std::invalid_argument("Invalid move argument. Figure vertices should located on OY 0..59");
	}
	
	for (auto& i : vertex)
	{
		i.first += x;
		i.second += y;
	}
}
void Figure::setFill()
{
	isFill = true;
}

time_t Figure::getDrawTime()
{
	return drawTime;
}
time_t Figure::getFillTime()
{
	return fillTime;
}