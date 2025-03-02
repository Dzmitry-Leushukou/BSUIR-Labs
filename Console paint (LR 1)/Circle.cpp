#include "Circle.h"

std::vector < std::pair<int, int>> Circle::fill()
{
	if (isFill)
	{
		std::vector < std::pair<int, int>> fillCoords;

	
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
	cx = vertex[0].first, cy = vertex[0].second;
	int x = cx;
	int y = cy-R;

	while (x!=cx-R)//UL
	{
		for (y = 0; y <= cy; y++)
		{
			if (std::round(std::sqrtl((x - cx) * (x - cx) + (y - cy) * (y - cy))) <= R)
				break;
		}
		this->vertex.push_back({ x,y });
		x--;
	}
	this->vertex.push_back({ x++,cy });
	
	while (x != cx)//UR
	{
		for (y = 59; y >= cy; y--)
		{
			if (std::round(std::sqrtl((x - cx) * (x - cx) + (y - cy) * (y - cy))) <= R)
				break;
		}
		this->vertex.push_back({ x,y });
		x++;
	}
	this->vertex.push_back({x++,cy + R});

	while (x!=cx+R)//DR
	{
		for (y = 59; y >= cy; y--)
		{
			if (std::round(std::sqrtl((x - cx) * (x - cx) + (y - cy) * (y - cy))) <= R)
				break;
		}
		this->vertex.push_back({ x,y });
		x++;
	}
	this->vertex.push_back({ x--,cy });
	while (x!=cx)//DL
	{
		for (y = 0; y <= cy; y++)
		{
			if (std::round(std::sqrtl((x - cx) * (x - cx) + (y - cy) * (y - cy))) <= R)
				break;
		}
		this->vertex.push_back({ x,y });
		x--;
	}
	this->vertex.push_back({ x,cy - R});

}