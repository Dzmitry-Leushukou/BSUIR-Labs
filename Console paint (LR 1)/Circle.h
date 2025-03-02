#pragma once
#include "Figure.h"
#include <string>
#include <vector>
#include <cmath>

class Circle :
    public Figure
{
public:
	std::vector < std::pair<int, int>> fill();
	std::vector < std::pair<int, int>> draw();
	virtual std::string toString() const override;
	Circle(std::vector<std::pair<int, int>> vertex);
private:
	int R,cx,cy,left_ind=-1,right_ind=-1;
};

