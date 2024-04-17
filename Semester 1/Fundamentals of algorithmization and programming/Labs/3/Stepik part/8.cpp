#include <iostream>

long long binpow(long long a, short int step)
{
	if (step == 0)
	{
		return 1;
	}
	if (step % 2 == 0)
	{
		long long x = binpow(a, step / 2);
		return x * x;
	}
	return binpow(a, step - 1) * a;
}
long long arm(long long x)
{
	short int step = 0;
	long long y = x;
	while (x != 0)
	{
		x /= 10;
		step++;
	}
	x = y;
	long long sum = 0;
	while (x != 0 && sum <= y)
	{
		sum += binpow(x % 10, step);
		x /= 10;
	}
	return sum;
}
bool check(long long x)
{
	return arm(x) == x;
}
int main()
{
	long long n;

	//n = (long long)1e8;
	std::cin >> n;
	for (long long i = 1; i < n; i++)
	{
		if (check(i))
		{
			std::cout << i << "\n";
		}
	}
}