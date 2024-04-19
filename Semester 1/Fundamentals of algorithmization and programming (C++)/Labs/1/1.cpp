#include <iostream>
#include <iomanip>

int main()
{
	long double x;
	std::cin >> x;
	long double a = 23 * x * x;
	long double st = a * 3 + 8;
	long double nst = x * (a + 32);
	std::cout << std::setprecision(20) << st + nst << " " << st - nst;

}