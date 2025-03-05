#include "Canvas.h"

int Canvas::HSIZE = 60;
int Canvas::VSIZE = 20;

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
	VSIZE = VSize;
	HSIZE = HSize;
	canva.resize(VSize);
	for (int i = 0; i < canva.size(); i++)
	{
		canva.at(i).resize(HSize);
	}
	repaint();
}
Canvas::Canvas(char symb[3], std::vector<Figure*>figures, int v, int h)
{
	for(int i=0;i<=2;i++)
	this->symb[i] = symb[i];
	this->figures = figures;
	VSIZE = v;
	HSIZE = h;
	canva.resize(VSIZE);
	for (int i = 0; i < VSIZE; i++)
	{
		canva.at(i).resize(HSIZE);
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
	for (int i = 0; i < VSIZE; i++)
	{
		for (int j = 0; j < HSIZE; j++)
			canva.at(i).at(j) = symb[0];
	}

	std::vector<std::vector<time_t>>time;
	time.resize(VSIZE);
	for (auto& i : time)
	{
		i.resize(HSIZE);
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

	case 1: // triangle
		if (points.size() > 3)
			throw std::invalid_argument("So many coordinates for triangle");
		if (points.size() < 3)
			throw std::invalid_argument("So few coordinates for triangle");
		if(points[0].first * (points[1].second - points[2].second) + points[1].first * (points[2].second - points[0].second) 
			+ points[2].first * (points[0].second - points[1].second)==0)
			throw std::invalid_argument("Three vertices cannot lie on the same line");
		if(points[0]==points[1] || points[2] == points[1] || points[0] == points[2])
			throw std::invalid_argument("2 vertices can`t locate on the same coordinates");
		figures.push_back(new Triangle(points));
		break;

	case 2: //circle
		if (points.size() > 2)
			throw std::invalid_argument("So many coordinates for circle");
		if (points.size() < 2)
			throw std::invalid_argument("So few coordinates for circle");
		figures.push_back(new Circle(points));
		if (!figures.back()->checkFields()||!(check(figures.back()->draw())))
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

char * Canvas::getSymb()
{
	return symb;
}

int Canvas::getVSIZE()
{
	return VSIZE;
}

int Canvas::getHSIZE()
{
	return HSIZE;
}