#pragma once
#include <vector>
#include <stdexcept>
#include <ctime>
#include <string>

class Figure
{
public:
	void move(int,int);
	virtual std::vector<std::pair<int,int>> fill() = 0;
	virtual  std::vector < std::pair<int, int>> draw() = 0;
	virtual std::string toString() const = 0;
	void setFill(bool);
	time_t getFillTime();
	time_t getDrawTime();
	void setFillTime();
	virtual std::string getFullInfo() const;
	virtual bool checkFields() const;
protected:
	std::vector<std::pair<int, int>>vertex;
	bool isFill=false;
	time_t fillTime=0, drawTime=0;

};

