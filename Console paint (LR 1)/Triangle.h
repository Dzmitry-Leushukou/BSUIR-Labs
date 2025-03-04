#pragma once
#include "Figure.h"
#include <string>
#include <cmath>
#include <algorithm>
class Triangle :
    public Figure
{
public:
	std::vector < std::pair<int, int>> fill();
	std::vector < std::pair<int, int>> draw();
	virtual std::string toString() const override;
	Triangle(std::vector<std::pair<int, int>> vertex);
private:
	int left_ind=0, right_ind=0;
	
};

