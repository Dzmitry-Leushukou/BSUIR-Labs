#pragma once
#include <vector>
#include <stdexcept>

class Figure
{
public:
	void move(int,int);
	virtual std::vector<std::pair<int,int>> fill() = 0;
	bool getIsFill();
	virtual  std::vector < std::pair<int, int>> draw() = 0;
	virtual std::string toString() const = 0;
protected:
	std::vector<std::pair<int, int>>vertex;
	bool isFill=false;
	time_t fillTime, drawTime;

};

