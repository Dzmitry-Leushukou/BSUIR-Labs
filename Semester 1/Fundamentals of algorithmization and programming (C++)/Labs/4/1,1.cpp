#include <iostream>
#include <cmath>
//#include "D:\Programming\BSUIR\Labs\Hat.h"

long long a[(long long)1e8];


void hat(short int variant)
{
	std::cout << "Completed the task : Dmitry Levshukov\n";

	if (variant != -1)
	{
		std::cout << "Variant: " << variant << "\n";
	}

	std::cout << "The task: ";
}

long double cinld()
{
	long double number;


	while (!(std::cin >> number) || (std::cin.peek() != '\n'))
	{
		std::cin.clear();


		while (std::cin.get() != '\n');


		std::cout << "Not correct input. Write again\n";
	}


	return number;
}


long long cinll()
{
	long long number;


	while (!(std::cin >> number) || (std::cin.peek() != '\n'))
	{
		std::cin.clear();


		while (std::cin.get() != '\n');


		std::cout << "Not correct input. Write again\n";
	}


	return number;
}


int main()
{
	long long k;
	

	hat(2);
	std::cout << "Introduce a one-dimensional static array of k elements and arrange its elements in reverse order\n\n";

	while (true)
	{
		std::cout << "Write k: ";
		k = cinll();
		std::cout << "Write array elements (Tap Enter after every element):\n";

		for (int i = 0; i < k; i++)
		{
			a[i] = cinll();
		}

		for (int i = 0; i < k / 2; i++)
		{
			a[i] ^= a[k - 1 - i];
			a[k - 1 - i] ^= a[i];
			a[i] ^= a[k - 1 - i];
		}

		for (int i = 0; i < k; i++)
		{
			std::cout << a[i] << " ";
		}
	}

}