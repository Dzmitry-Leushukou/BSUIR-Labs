#include "Canvas.h"

std::vector<std::vector<char>> Canvas::getCanva() const
{
	return canva;
}

std::vector<const Figure*> Canvas::getFigures() const
{
	std::vector<const Figure*>f;
	for (auto& i : figures)
	{
		f.push_back(i);
	}
	return f;
}

void Canvas::move(int id, int x, int y)
{
	figures[id]->move(x, y);
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

void Canvas::draw(int id, std::vector<std::pair<int, int>>points)
{
	switch (id)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	default:
		throw std::invalid_argument("Incorrect type");
	}
}