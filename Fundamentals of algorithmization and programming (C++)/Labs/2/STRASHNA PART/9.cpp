#include <iostream>
#include <cmath>
#include <iomanip>


//y = a*ln(1+x^(1/5))+cos^2(f(x)+1);
int main()
{
	long double a,x,f;
	long long z;
	short int type;
	std::cout << "y = a * ln(1 + x^(1/5)) + cos^2(f(x) + 1)\n";
	std::cout << "Enter a\n";
	std::cin >> a;
	std::cout << "Choose f(x):\n1. 2x\n2. x^3\n3. x/3\n";
	std::cin >> type;
	std::cout << "Enter integer z\n";
	std::cin >> z;
	switch (z<1)
	{
	case 1:
		x = z * z;
		break;
	default:
		x = z + 1;
	}
	switch (type)
	{
	case 1:
		f = x * 2.0;
		break;
	case 2: 
		f = x * x * x;
		break;
	default:
		f = x / 3.0;
	}
	f ++;
	std::cout << "y = " << logl(1 + powl(x, 1 / 5)) + cosl(f);
}
