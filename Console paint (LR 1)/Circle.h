#pragma once
#include "Figure.h"
#include <string>
#include <vector>
#include <cmath>

class Circle :
    public Figure
{
public:
	virtual std::vector < std::pair<int, int>> fill() override;
	virtual std::vector < std::pair<int, int>> draw() override;
	virtual std::string toString() const override;
	Circle(std::vector<std::pair<int, int>> vertex);
	Circle(bool,time_t,time_t, int,int,int);
	virtual std::string getFullInfo() const override;
	bool checkFields() const;
private:
	int R,cx,cy,left_ind=0,right_ind=0;
};

