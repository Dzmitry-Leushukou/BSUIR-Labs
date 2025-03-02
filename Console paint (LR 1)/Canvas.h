#pragma once

#include <vector>

class Canvas
{
public:
	Canvas(int, int);
	std::vector<std::vector<char>> getCanva() const;
	void set(short ind, char c);
	
private:
	void repaint();

	std::vector<std::vector<char>>canva;
	char symb[3] = { ' ', '#', '*' }; // 0 - back, 1 - fill, 2 - draw

};

