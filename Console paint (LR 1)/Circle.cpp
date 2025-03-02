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
	for (int x = cx - R; x <= cx + R; x++)
	{
		for (int y = cy - R; y <= cy + R; y++)
		{
			if (std::round(std::sqrtl((cx - x) * (cx - x) + (cy - y) * (cy - y))) == R)
			{
				this->vertex.push_back({ x,y });
			}
		}
	}

}