#include <iostream>
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

int main()
{
	ll n, k, q, w;

	hat(2);
	std::cout << "Raise a square real matrix A of size N to the Kth power\n\n";

	while (true)
	{
		std::cout << "Write N:\n";
		n = cinll();


		long double** a = new long double* [n];
		long double* sum = new long double[n];
		long double** c = new long double* [n];

		for (int i = 0; i < n; ++i)
			a[i] = new long double[n], c[i] = new long double[n];


		std::cout << "Write every element of matrix A (after every elemnt use Enter button):\n";
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
					a[i][j] = cinld(), c[i][j] = a[i][j];
				
			

		std::cout << "Write K\n";
		k = cinll();
		k--;

		while (k--)
		{
			for (int i = 0; i < n; i++)
			{
				for (int j = 0; j < n; j++)
				{
					q = 0, w = 0, sum[j] = 0;
					while (q < n && w < n)
					{
						sum[j] += c[i][q] * a[w][j];
						q++;
						w++;
					}
					//std::cout<<sum[j] << " ";


				}
				for (int r = 0; r < n; r++)
				{
					c[i][r] = sum[r];
				}

			}

		}


		std::cout << "Matrix A to the Kth power:\n";
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
				std::cout << c[i][j] << " ";
			std::cout << '\n';
		}


		delete[] sum;
		sum = nullptr;

		for (int i = 0; i < n; ++i)
		{
			delete[] a[i];
			a[i] = nullptr;

			delete[] c[i];
			c[i] = nullptr;
		}

		delete[] a;
		a = nullptr;
		delete[] c;
		c = nullptr;
	}
}