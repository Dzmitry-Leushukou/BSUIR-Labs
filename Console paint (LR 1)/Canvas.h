#pragma once
#include <vector>

#include "Figure.h"
#include "Rectangle.h"

class Canvas
{
public:
	Canvas(int, int);
	std::vector<std::vector<char>> getCanva();
	void set(short ind, char c);
	std::vector<const Figure*> getFigures() const;
	void move(int, int, int);
	void draw(int, std::vector<std::pair<int,int>>);
private:
	void repaint();
	bool check(const std::vector<std::pair<int, int>>& v);
	//Visual
	std::vector<std::vector<char>>canva;
	char symb[3] = { ' ', '#', '*' }; // 0 - back, 1 - fill, 2 - draw

	std::vector<Figure*>figures;
};

