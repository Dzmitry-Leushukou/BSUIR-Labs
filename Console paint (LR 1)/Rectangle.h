#pragma once
#include "Figure.h"
#include <time.h>
#include <algorithm>
#include <string>
class Rectangle :
    public Figure
{
public:
	Rectangle(std::vector < std::pair<int, int>> vertex);
	virtual  std::vector < std::pair<int, int>> fill() override;
	virtual  std::vector < std::pair<int, int>> draw() override;
	virtual std::string toString() const override;
};

