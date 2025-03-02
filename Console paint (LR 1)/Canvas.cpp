#include "Canvas.h"

std::vector<std::vector<char>> Canvas::getCanva() const
{
	return canva;
}
Canvas::Canvas(int VSize, int HSize)
{
	canva.resize(VSize);
	for (int i = 0; i < canva.size(); i++)
	{
		canva[i].resize(HSize);
	}
	repaint();
}

void Canvas::set(short ind, char c)
{
	symb[ind] = c;
	repaint();
}

void Canvas::repaint()
{
	for (int i = 0; i < canva.size(); i++)
	{
		for (int j = 0; j < canva[i].size(); j++)
			canva[i][j] = symb[0];
	}
}