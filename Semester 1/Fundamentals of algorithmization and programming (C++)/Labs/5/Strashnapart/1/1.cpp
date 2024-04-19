#include <iostream>
#include "..\StaticLib\SLib.h"
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long

int main()
{
	ll n;
	hat(2);
	std::cout << "Generate two two-dimensional dynamic matrix arrays A and B with dimensions n x n.\nValues of array elements a[i][j] and b[i][j]:\n";
	std::cout << "a[i][j] = 3*i*j - 3; b[i][j] = 2*i*j - 2 at i = 0, 1, . . ., n, j = 0, 1, …, n.\n";
	std::cout<<"Determine the sums of the elements of the main diagonals of these matrix arrays.\n\n";
	while (true)
	{
		std::cout << "Write n:\n";
		n = cinllmy();
		ll** a = (ll**)malloc(n * sizeof(ll*)), ** b = (ll**)malloc(n * sizeof(ll*));
		for (int i = 0; i < n; i++)
		{
			a[i] = (ll*)malloc(n * sizeof(ll)), b[i] = (ll*)malloc(n * sizeof(ll));

			for (int j = 0; j < n; j++)
				a[i][j] = 3 * i * j - 3, b[i][j] = 2 * i * j - 2;
		}
		std::cout << "The sum of the elements of the main diagonals of first matrix: "<<get_sum(a, n) << '\n';
		std::cout << "First array:\n";
		cout_arr(a, n);
		std::cout << "The sum of the elements of the main diagonals of second matrix: " << get_sum(b, n) << '\n';
		std::cout << "Second array:\n";
		cout_arr(b, n);
	}
}
