#pragma once
#include <vector>
class Figure
{
public:
	void move();
	void fill();
private:
	std::vector<std::pair<int, int>>vertex;
	bool isFill=false;

};

