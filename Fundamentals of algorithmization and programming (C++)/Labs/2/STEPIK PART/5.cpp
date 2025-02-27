#include <iostream>

int main()
{
	long double a, b, c, x, y;
	std::cin >> a >> b >> c >> x >> y;
	if (a > 0 && b > 0 && c > 0 && x > 0 && y > 0) {
		if ((a <= x && b <= y) || (b <= x && a <= y) || (a <= x && c <= y) || (c <= x && a <= y) || (c <= x && b <= y) || (b <= x && c <= y))
			std::cout << "YES";
		else std::cout << "NO";
	}
	else std::cout << "Incorrect input";
}