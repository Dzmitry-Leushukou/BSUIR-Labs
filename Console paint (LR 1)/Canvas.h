#pragma once
#include <vector>

#include "Figure.h"

class Canvas
{
public:
	Canvas(int, int);
	std::vector<std::vector<char>> getCanva() const;
	void set(short ind, char c);
	std::vector<Figure*> getFigures() const;
private:
	void repaint();

	//Visual
	std::vector<std::vector<char>>canva;
	char symb[3] = { ' ', '#', '*' }; // 0 - back, 1 - fill, 2 - draw
	
	std::vector<Figure*>figures;
};

