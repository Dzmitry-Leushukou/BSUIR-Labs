#include <iostream>
#include <iomanip>


void hat(short int variant)
{
	std::cout << "Completed the task : Dmitry Levshukov\n";

	if (variant != -1)
	{
		std::cout << "Variant: " << variant << "\n";
	}

	std::cout << "The task: ";
}

int main()
{
	long double sum = 0, a = 1.0 / 2.0, b = 1.0 / 3.0, e = 1e-3;
	

	hat(-1);
	std::cout << "Find the sum of the series with accuracy 10 ^ (-3)\n          The common term of the series = (1 / 2) ^ n + (1 / 3) ^ n\n\n";
	std::cout << std::setprecision(10);

	//long long n=0;

	do {
		sum += a + b;
		a *= (1.0 / 2.0);
		b *= (1.0 / 3.0);
		//n++;
		//std::cout << a << " " << b << "\n";
	} while (a + b >= e);

	std::cout << "sum = " << sum<<" | ";
}