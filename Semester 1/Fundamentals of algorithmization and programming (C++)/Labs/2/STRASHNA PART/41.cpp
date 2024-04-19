#include <iostream>

int main()
{
	long long x, y;
	std::cin >> x >> y;
	bool c= (x > y);
	std::cout << (c ? x : y);
}