#pragma once
#include "Figure.h"
#include <string>
#include <cmath>
#include <algorithm>
class Triangle :
    public Figure
{
public:
	virtual  std::vector < std::pair<int, int>> fill() override;
	virtual  std::vector < std::pair<int, int>> draw() override;
	virtual std::string toString() const override;
	Triangle(std::vector<std::pair<int, int>> vertex);
};

