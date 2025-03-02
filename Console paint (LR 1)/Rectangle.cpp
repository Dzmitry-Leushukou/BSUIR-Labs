#include "Rectangle.h"

std::vector < std::pair<int, int>> Rectangle::fill()
{
	if (isFill)
	{
		fillTime = clock();
		std::vector < std::pair<int, int>> fillCoords;

		for (int x = vertex[0].first + 1; x < vertex[1].first; x++)
			for (int y = vertex[0].second + 1; y < vertex[1].second; y++)
				fillCoords.push_back({ x,y });

		return fillCoords;
	}
	else
		return {};
}
std::vector < std::pair<int, int>> Rectangle::draw()
{
	drawTime = clock();
	std::vector < std::pair<int, int>> drawCoords;

	for (int x = vertex[0].first; x <= vertex[1].first; x++)
	{
		drawCoords.push_back({ x,vertex[0].second });
		drawCoords.push_back({ x,vertex[1].second });
	}
		
	for (int y = vertex[0].second; y <= vertex[1].second; y++)
	{
		drawCoords.push_back({ vertex[0].first,y });
		drawCoords.push_back({ vertex[1].first,y });
	}
	return drawCoords;
}
std::string Rectangle::toString() const 
{
	std::string c1, c2, c3, c4;
	c1 = std::to_string(vertex[0].first) + "; " + std::to_string(vertex[0].second);
	c2 = std::to_string(vertex[0].first) + "; " + std::to_string(vertex[1].second);
	c3 = std::to_string(vertex[1].first) + "; " + std::to_string(vertex[0].second);
	c4 = std::to_string(vertex[1].first) + "; " + std::to_string(vertex[1].second);

	std::string res;
	res = isFill ? "Filled r" : "R";
	res +="ectangle: [" + c1 + "] [" + c2 + "] [" + c3 + "] [" + c4 + "]";
	return res;
}
Rectangle::Rectangle(std::vector<std::pair<int, int>> vertex)
{
	std::sort(vertex.begin(), vertex.end());
	this->vertex = vertex;
}