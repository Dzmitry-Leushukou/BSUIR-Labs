#pragma once
#include <vector>
#include <stdexcept>

class Figure
{
public:
	void move(int,int);
	virtual void fill() = 0;
	bool getIsFill();
	virtual void draw() = 0;
	virtual std::string toString() const = 0;
private:
	std::vector<std::pair<int, int>>vertex;
	bool isFill=false;

};

