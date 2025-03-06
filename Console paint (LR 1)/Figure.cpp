#include "Figure.h"
#include "Canvas.h"

bool Figure::checkFields() const
{
	for (auto& i : vertex)
	{
		if (i.first < 0 || i.first >= Canvas::getVSIZE())
		{
			return false;
		}
		if (i.second < 0 || i.second >= Canvas::getHSIZE())
		{
			return false;
		}
	}
	return true;
}

std::string Figure::getFullInfo() const
{
	std::string res;
	if (isFill)
		res = "1 ";
	else
		res = "0 ";

	res += std::to_string(drawTime) + " " + std::to_string(fillTime);
	for (auto& i : vertex)
	{
		res += " " + std::to_string(i.first) + " " + std::to_string(i.second);
	}

	return res;
}

void Figure::move(int x, int y)
{
	for (auto& i : vertex)
	{
		if(i.first + x < 0 || i.first + x >= Canvas::getVSIZE())
			throw std::invalid_argument("Invalid move argument. Figure vertices should located on OX 0.."+std::to_string(Canvas::getVSIZE()-1));
		if (i.second + y < 0 || i.second + y >= Canvas::getHSIZE())
			throw std::invalid_argument("Invalid move argument. Figure vertices should located on OY 0.." + std::to_string(Canvas::getHSIZE() - 1));
	}
	
	for (auto& i : vertex)
	{
		i.first += x;
		i.second += y;
	}
}

void Figure::setFill(bool s)
{
	isFill = s;
}

time_t Figure::getDrawTime()
{
	return drawTime;
}

time_t Figure::getFillTime()
{
	return fillTime;
}

void Figure::setFillTime()
{
	fillTime = clock();
}