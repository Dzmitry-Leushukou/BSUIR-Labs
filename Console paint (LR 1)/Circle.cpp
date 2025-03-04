#include "Circle.h"

std::string Circle::getFullInfo() const
{
	std::string res;
	if (isFill)
		res = "1 ";
	else
		res = "0 ";

	res += std::to_string(drawTime) + " " + 
		   std::to_string(fillTime) + " " +
		   std::to_string(R)+ " " +
		   std::to_string(cx)+ " " +
		   std::to_string(cy) + " "  +
		   std::to_string(left_ind) + " " +
		   std::to_string(right_ind);

	for (auto& i : vertex)
	{
		res += " " + std::to_string(i.first) + " " + std::to_string(i.second);
	}

	return res;
}

std::vector < std::pair<int, int>> Circle::fill()
{
	if (isFill)
	{
		std::vector < std::pair<int, int>> fillCoords;
		
		for (int i = left_ind; i <= right_ind; i++)
		{
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