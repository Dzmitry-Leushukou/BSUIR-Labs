#pragma once

#include<iostream>
#define ll long long

ll get_sum(ll **a, ll size)
{
	ll res = 0;
	for (int i = 0; i < size; i++)
		res += a[i][i];
	return res;
}
void cout_arr(ll **a, ll size)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
			std::cout << a[i][j]<<'\t';
		std::cout << '\n';
	}
}