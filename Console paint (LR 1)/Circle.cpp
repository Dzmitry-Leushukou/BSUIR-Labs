#include "Circle.h"
#include "Canvas.h"

bool Circle::checkFields() const
{
	if (cx - R >= Canvas::getVSIZE() || cx - R < 0 || cx + R >= Canvas::getVSIZE() || cx + R < 0 || R < 0 ||
		cy - R >= Canvas::getHSIZE() || cy - R < 0 || cy + R >= Canvas::getHSIZE() || cy + R < 0)
		return false;
	return true;
}

std::string Circle::getFullInfo() const
{
	std::string res;
	if (isFill)
		res = "1 ";
	else
		res = "0 ";

	res += std::to_string(drawTime) + " " +
		std::to_string(fillTime) + " " +
		std::to_string(R) + " " +
		std::to_string(cx) + " " +
		std::to_string(cy);

	return res;
}

std::vector < std::pair<int, int>> Circle::fill()
{
	if (isFill)
	{
		std::vector < std::pair<int, int>> fillCoords;
		
		for (int i = left_ind; i < right_ind; i++)
		{
			if(vertex.at(i).first== vertex.at(i+1).first)
			for (int j = vertex.at(i).second+1; j < vertex.at(i + 1).second; j++)
			{
				fillCoords.push_back({ vertex.at(i).first,j });
			}
		}
		return fillCoords;
	}
	else
		return {};
}
std::vector < std::pair<int, int>> Circle::draw()
{	
	return vertex;
}
std::string Circle::toString() const
{
	std::string c1, c2;
	c1 = std::to_string(cx) + "; " + std::to_string(cy);
	c2 = std::to_string(R);

	std::string res;
	res = isFill ? "Filled c" : "C";
	res += "ircle: [" + c1 + "] R = " + c2;
	return res;
}
Circle::Circle(std::vector<std::pair<int, int>> vertex)
{
	drawTime = clock();
	R = vertex.at(1).first;
	this->vertex.clear();
	cx = vertex.at(0).first, cy = vertex.at(0).second;
	for (int x = cx - R; x <= cx + R; x++)
	{
		if (x == cx - R + 1)
			left_ind = this->vertex.size();
		if (x == cx + R)
			right_ind = this->vertex.size()-1;
		for (int y = cy - R; y <= cy + R; y++)
		{
			if (std::round(std::sqrtl((cx - x) * (cx - x) + (cy - y) * (cy - y))) == R)
			{
				this->vertex.push_back({ x,y });
			}
		}
	}

}

Circle::Circle(bool f, time_t d, time_t fill, int r, int cx,
	           int cy)
{
	this->isFill = f;
	this->drawTime = d;
	this->fillTime = fill;
	this->R = r;
	this->cx = cx;
	this->cy = cy;

	this->vertex.clear();
	for (int x = cx - R; x <= cx + R; x++)
	{
		if (x == cx - R + 1)
			left_ind = this->vertex.size();
		if (x == cx + R)
			right_ind = this->vertex.size() - 1;
		for (int y = cy - R; y <= cy + R; y++)
		{
			if (std::round(std::sqrtl((cx - x) * (cx - x) + (cy - y) * (cy - y))) == R)
			{
				this->vertex.push_back({ x,y });
			}
		}
	}
}