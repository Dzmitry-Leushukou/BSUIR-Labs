#include "Serializer.h"

void Serializer::Serialize(char symb[3], std::vector<const Figure*>figures, std::string filename)
{
	std::ofstream fout;
	fout.open(filename, std::ios_base::out);

	if (!fout.is_open())
		throw std::invalid_argument("Invalid file path");

	fout << symb[0] << " " << symb[1] << " " << symb[2] << '\n';
	for (auto& figure : figures)
	{
		if (const Triangle* triangle = dynamic_cast<const Triangle*>(figure))
		{
			fout << "Triangle ";
		}
		if (const Circle* triangle = dynamic_cast<const Circle*>(figure))
		{
			fout << "Circle ";
		}
		if (const Rectangle* triangle = dynamic_cast<const Rectangle*>(figure))
		{
			fout << "Rectangle ";
		}
		fout<<figure->getFullInfo() << " |\n";
	}

	fout.close();
}


Canvas* Serializer::Deserialize(std::string filename)
{
	std::ifstream fin;
	fin.open(filename, std::ios_base::in);

	if (!fin.is_open())
		throw std::invalid_argument("Invalid file path");

	char symb[3];
	std::vector<Figure*>figures;
	std::string s;
	std::getline(fin, s);
	
	if(s.size()!=5)
		throw std::invalid_argument("Wrong format line: 1");

	symb[0] = s.at(0);
	symb[1] = s.at(2);
	symb[2] = s.at(4);
	int line = 1;
	while(std::getline(fin,s))
	{
		line++;
		int i = 0;
		//Type
		std::string type;
		while (s.at(i) != ' ')
		{
			type += s.at(i);
			i++;
		}
		i++;
		if (type == "Circle")
		{
			bool fill = Serializer::readNumber(s, i, line);
			time_t drawTime = Serializer::readNumber(s, i, line);
			time_t fillTime = Serializer::readNumber(s, i, line);
			int R = Serializer::readInt(s, i, line);
			int cx = Serializer::readInt(s, i, line);
			int cy = Serializer::readInt(s, i, line);
			int left_ind = Serializer::readInt(s, i, line);
			int right_ind = Serializer::readInt(s, i, line);
			std::vector<std::pair<int, int>>v;
			while (s.at(i) != '|')
			{
				v.push_back({ Serializer::readInt(s, i, line), Serializer::readInt(s, i, line) });
			}
			figures.push_back(new Circle(fill, drawTime, fillTime, R, cx, cy,left_ind,right_ind,v));
		}
		else
			if (type == "Triangle"||type=="Rectangle")
			{
				bool fill = Serializer::readNumber(s, i, line);
				time_t drawTime = Serializer::readNumber(s, i, line);
				time_t fillTime = Serializer::readNumber(s, i, line);
				std::vector<std::pair<int, int>>v;
				while (s.at(i)!='|')
				{
					v.push_back({ Serializer::readInt(s, i, line), Serializer::readInt(s, i, line)});
				}
				if (v.size() < 3 && type == "Triangle")
					throw std::invalid_argument("Wrong info line:" + std::to_string(line));
				else 
					if (v.size() < 2 && type == "Rectangle")
						throw std::invalid_argument("Wrong info line:" + std::to_string(line));

				if (type == "Triangle")
				{
					figures.push_back(new Triangle(fill, drawTime, fillTime, v));
				}
				else
					figures.push_back(new Rectangle(fill, drawTime, fillTime, v));
			}
			else 
				throw std::invalid_argument("Wrong info line:"+std::to_string(line));
	}

	fin.close();

	Canvas* canva = new Canvas(symb, figures);
	if (checkCanvasFigures(canva))
		return canva;
	throw std::invalid_argument("Wrong info about figures" + std::to_string(line));
}

long long Serializer::readNumber(std::string s, int& i,int line)
{
	std::string ch;
	while (s.at(i)>='0'&& s.at(i) <= '9')
	{
		ch += s.at(i);
		i++;
	}
	if(ch.size()==0)
		throw std::invalid_argument("Wrong info line:" + std::to_string(line));
	i++;
	return stoll(ch);
}

int Serializer::readInt(std::string s, int& i, int line)
{
	std::string ch;
	while (s.at(i) >= '0' && s.at(i) <= '9')
	{
		ch += s.at(i);
		i++;
	}
	if (ch.size() == 0)
		throw std::invalid_argument("Wrong info line:" + std::to_string(line));
	i++;
	return stoi(ch);
}

bool Serializer::checkCanvasFigures(Canvas* canva)
{
	for (auto& i : canva->getFigures())
	{
		if (!i->checkFields())
		{
			return false;
		}
	}
	return true;
}