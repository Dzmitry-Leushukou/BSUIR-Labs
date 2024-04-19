#include <iostream>
#include <random>
#include <ctime>
//#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long


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


std::mt19937 gen(time(nullptr));
ll f()
{
	ll a = gen(), b = gen();

	if (a % 2 == 0)
		return b;
	return (-1) * b;

}
int main()
{
	
	hat(2);
	std::cout << "Initialize upon declaration a static two-dimensional array of integers of size NXM.\n";
	std::cout << "          Determine the number of negative elements located above the main diagonal of the matrix.\n\n";
	
	while (true)
	{
		ll a[(ll)10][(ll)10] = { {f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()},
								{f(),f(),f(),f(),f(),f(),f(),f(),f(),f()}, };
		std::cout << "Our matrix 10x10:\n\n";
		for (int i = 0; i < (ll)10; i++) {
			for (int j = 0; j < (ll)10; j++)
			{
				std::cout << a[i][j] << " ";
			}
			std::cout << '\n';
		}
		std::cout << '\n';

		ll i = 0, kol=0;
		while (i < 10)
		{
			i++;
			for (int j = i-1; j >= 0; j--)
			{
				if (a[j][i] < 0)
					kol++;
			}
		}
		
		std::cout << "The number of negative elements located above the main diagonal of the matrix: " << kol << "\n\n";
		system("pause");
	}
	
}
