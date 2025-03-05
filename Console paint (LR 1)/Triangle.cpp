#include "Triangle.h"


std::string Triangle::getFullInfo() const
{
	std::string res;
	if (isFill)
		res = "1 ";
	else
		res = "0 ";

	res += std::to_string(drawTime) + " " + std::to_string(fillTime);
	int ind = 0;
	for (auto& i : vertex)
	{
		if (ind == 3)
			break;
		res += " " + std::to_string(i.first) + " " + std::to_string(i.second);
		ind++;
	}

	return res;
}

Triangle::Triangle(bool fill, time_t dt, time_t ft, std::vector<std::pair<int, int>>v)
{
	this->isFill = fill;
	drawTime = dt;
	fillTime = ft;
	this->vertex = v;
	for (int i = 1; i <= v.size(); i++)
	{
		long double x1 = vertex.at(i - 1).first, y1 = vertex.at(i - 1).second;
		long double x2 = vertex.at(i % 3).first, y2 = vertex.at(i % 3).second;
		long double k = (x1 - x2) / (y1 - y2);
		long double b = x1 - k * y1;

		y1 = std::min(vertex.at(i - 1).second, vertex.at(i % 3).second);
		y2 = std::max(vertex.at(i - 1).second, vertex.at(i % 3).second);
		x1 = std::min(vertex.at(i - 1).first, vertex.at(i % 3).first);
		x2 = std::max(vertex.at(i - 1).first, vertex.at(i % 3).first);
		if (y1 == y2)
			for (int x = x1 + 1; x < x2; x++)
			{
				this->vertex.push_back({ x,y1 });
			}
		else if (x1 == x2)
			for (int y = y1 + 1; y < y2; y++)
			{
				this->vertex.push_back({ x1,y });
			}
		else if (y2 - y1 >= x2 - x1)
			for (int y = y1 + 1; y < y2; y++)
			{
				this->vertex.push_back({ std::round(k * (long double)y + b),y });
			}
		else
			for (int x = x1 + 1; x < x2; x++)
			{
				this->vertex.push_back({ x,std::round(((long double)x - b) / k) });
			}
	}



}

std::vector < std::pair<int, int>> Triangle::fill()
{
	if (isFill)
	{
		std::vector < std::pair<int, int>> fillCoords,vertex;
		vertex = this->vertex;
		std::sort(vertex.begin(), vertex.end());
		
		for (int i = 0; i < vertex.size()-1; i++)
		{
			if(vertex.at(i).first== vertex.at(i+1).first)
			for (int j = vertex.at(i).second + 1; j < vertex.at(i + 1).second; j++)
			{
				fillCoords.push_back({ vertex.at(i).first,j });
			}
			
		}
		return fillCoords;
	}
	else
		return {};
}
std::vector < std::pair<int, int>> Triangle::draw()
{
	return vertex;
}
std::string Triangle::toString() const
{

	std::string c1;
	c1 = " (" +std::to_string(vertex.at(0).first) + "; " + std::to_string(vertex.at(0).second) + ") " +
		 "(" +std::to_string(vertex.at(1).first) + "; " + std::to_string(vertex.at(1).second) + ")" +
		 "(" +std::to_string(vertex.at(2).first) + "; " + std::to_string(vertex.at(2).second) + ") ";

	std::string res;
	res = isFill ? "Filled t" : "T";
	res += "riangle: [" + c1 + "] ";
	return res;
}
Triangle::Triangle(std::vector<std::pair<int, int>> vertex)
{
	drawTime = clock();
	/* x1 = ky1 + b    (x1-x2)/(y1-y2) = k
	*				=> 
	*  x2 = ky2 + b    b=x1-ky1;
	*/
	this->vertex = vertex;
	for (int i = 1; i <= vertex.size(); i++)
	{
		long double x1 = vertex.at(i-1).first, y1= vertex.at(i - 1).second;
		long double x2 = vertex.at(i%3).first, y2 = vertex.at(i%3).second;
		long double k = (x1 - x2) / (y1 - y2);
		long double b = x1 - k * y1;

		y1 = std::min(vertex.at(i - 1).second, vertex.at(i % 3).second);
		y2 = std::max(vertex.at(i - 1).second, vertex.at(i % 3).second);
		x1 = std::min(vertex.at(i - 1).first, vertex.at(i % 3).first);
		x2 = std::max(vertex.at(i - 1).first, vertex.at(i % 3).first);
		if (y1 == y2)
			for (int x = x1 + 1; x < x2; x++)
			{
				this->vertex.push_back({ x,y1 });
			}
		else if(x1==x2)
			for (int y = y1 + 1; y < y2; y++)
			{
				this->vertex.push_back({ x1,y });
			}
		else if(y2-y1>=x2-x1)
			for (int y = y1 + 1; y < y2; y++)
			{
				this->vertex.push_back({ std::round(k * (long double)y + b),y});
			}
		else
			for (int x = x1 + 1; x < x2; x++)
			{
				this->vertex.push_back({ x,std::round(((long double)x - b)/k)});
			}
	}
	
	

}