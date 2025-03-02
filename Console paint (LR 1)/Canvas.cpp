#include "Canvas.h"

std::vector<std::vector<char>> Canvas::getCanva()
{
	repaint();
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
		canva.at(i).resize(HSize);
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
		for (int j = 0; j < canva.at(i).size(); j++)
			canva.at(i)[j] = symb[0];
	}

	std::vector<std::vector<time_t>>time;
	time.resize(canva.size());
	for (auto& i : time)
	{
		i.resize(canva[0].size());
		for (auto& j : i)
		{
			j = 0;
		}
	}
	for (auto& i : figures)
	{
		std::vector<std::pair<int, int>> c=i->draw();
		time_t t = i->getDrawTime();
		for (auto& j : c)
		{
			if (time[j.first][j.second] < t)
			{
				canva[j.first][j.second] = symb[2];
			}
		}

		c = i->fill();
		t = i->getFillTime();
		for (auto& j : c)
		{
			if (time[j.first][j.second] < t)
			{
				canva[j.first][j.second] = symb[1];
			}
		}
	}

}

void Canvas::draw(int id, std::vector<std::pair<int, int>>points)
{
	if (!check(points))
		throw std::invalid_argument("Incorrect coordinates");
	switch (id)
	{
	case 0: //rectangle
		if(points.size() > 2)
			throw std::invalid_argument("So many coordinates for rectangle");
		if (points.size() < 2)
			throw std::invalid_argument("So many coordinates for rectangle");
		figures.push_back(new Rectangle(points));
		break;
	case 1:
		break;
	case 2:
		break;
	default:
		throw std::invalid_argument("Incorrect type");
	}
}

bool Canvas::check(const std::vector<std::pair<int, int>>&v)
{
	for (auto& i : v)
	{
		if (!(i.first >= 0 && i.first < canva.size()&& i.second >= 0 && i.second < canva[0].size()))
			return false;
	}
	return true;
}