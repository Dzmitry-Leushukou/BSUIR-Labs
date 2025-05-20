#include <iostream>


int main()
{
	std::setlocale(LC_ALL, "");

	long double x1, x2, y1, y2, r1, r2;
	std::cin >> x1 >> y1 >> r1 >> x2 >> y2 >> r2;
	long double d = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

	//Per
	if (r1 + r2 > d && r1 + d > r2 && r2 + d > r1)
	{
		std::cout << "Фигуры пересекаются";
		return 0;
	}

	//Pogl
	if (r1 + d < r2 || r1 - r2 == d)
	{
		std::cout << "Да";
		return 0;
	}
	if (r2 + d < r1 || r2 - r1 == d)
	{

		std::cout << "Да, но справедливо обратное для двух фигур";
		return 0;
	}

	std::cout << "Ни одно условие не выполнено";
}