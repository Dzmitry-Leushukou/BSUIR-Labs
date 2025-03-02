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
	figures.at(id)->move(x, y);
}

void Canvas::fill(int id)
{
	figures.at(id)->setFill(1);
	figures.at(id)->setFillTime();
}

void Canvas::erase(int id)
{
	delete figures.at(id);
	figures.at(id) = nullptr;
	figures.erase(figures.begin() + id);
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
			canva.at(i).at(j) = symb[0];
	}

	std::vector<std::vector<time_t>>time;
	time.resize(canva.size());
	for (auto& i : time)
	{
		i.resize(canva.at(0).size());
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
			if (time.at(j.first).at(j.second) < t)
			{
				canva.at(j.first).at(j.second) = symb[2];
				time.at(j.first).at(j.second) = t;
			}
		}

		c = i->fill();
		t = i->getFillTime();
		for (auto& j : c)
		{
			if (time.at(j.first).at(j.second) < t)
			{
				canva.at(j.first).at(j.second) = symb[1];
				time.at(j.first).at(j.second) = t;
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
			throw std::invalid_argument("So few coordinates for rectangle");
		figures.push_back(new Rectangle(points));
		break;

	case 1: //
		break;

	case 2: //circle
		if (points.size() > 2)
			throw std::invalid_argument("So many coordinates for circle");
		if (points.size() < 2)
			throw std::invalid_argument("So few coordinates for circle");
		figures.push_back(new Circle(points));
		if (!(check(figures.back()->draw())))
		{
			erase(figures.size()-1);
			throw std::invalid_argument("Incorrect coordinates");
		}
		break;

	default:
		throw std::invalid_argument("Incorrect type");
	}
}

bool Canvas::check(const std::vector<std::pair<int, int>>&v)
{
	for (auto& i : v)
	{
		if (!(i.first >= 0 && i.first < canva.size()&& i.second >= 0 && i.second < canva.at(0).size()))
			return false;
	}
	return true;
}