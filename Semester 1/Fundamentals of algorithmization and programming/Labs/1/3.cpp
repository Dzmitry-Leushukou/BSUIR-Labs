#include <iostream>
#include <iomanip>

int main()
{
	long double t, a, u0;
	std::cin >> a >> t >> u0;
	std::cout << std::setprecision(20) << u0 * t + a * t * t / 2;
}
