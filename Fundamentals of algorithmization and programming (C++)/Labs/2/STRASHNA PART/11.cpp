#include <iostream>
#include <cmath>
int main()
{	
	setlocale(LC_ALL, "");
	const long double pi = 3.14159265359;
	long double r, p, q, rmax;
	std::cin >> r >> p >> q;
	
	//c*c=a*a+b*b+2*a*b*cos(f)
	//   |   V   |     V     |
	//       f         s

	long double f = p*p+p*p;
	long double s = 2*p*p*cos(q*pi/180.0);

	// d1, d2 - diag
	long double d1 = sqrtl(f + s), d2 = sqrtl(f - s);
	rmax = d1 * d2 / (4.0 * p);
	rmax <= r ? std::cout << "Сможет" : std::cout << "Не сможет";
}