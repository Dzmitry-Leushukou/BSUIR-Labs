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
