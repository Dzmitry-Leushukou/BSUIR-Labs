#include <iostream>
#include<string>
#include "D:\Programming\BSUIR\Labs\Hat.h"
#define ll long long

const ll ll_lenght = 64;

std::string to_ob(ll a)
{
	std::string res = "";
	ll b = abs(a);
	while (b != 0)//O(log2(b))
	{
		res = (char)('0' + b % 2) + res;
		b /= 2;
	}
	ll n = ll_lenght-res.size();
	
	if (a >= 0)
	{
		std::string zeroes(n, '0');
		return zeroes + res;
	}
	std::string zeroes(n, '1');
	n = res.size();
	for (int i = 0; i < n; i++)//O(n)
	{
		res[i] = ((res[i] - '0' + 1) % 2 + '0');
	}
	return zeroes + res;
}
std::string to_d(std::string s)
{
	if (s[0] == '0')
		return s;
	ll n = s.size();
	for (; n >= 0; n--)//O(n)
		if (s[n] == '0')
			break;
	s[n] = '1';
	n++;
	ll m = s.size();
	for (n; n < m; n++)//O(n)
	{
		s[n] = '0';
	}
	return s;
}
std::string d_to_p(std::string s)
{
	if (s[0] != '0')
	{
		if (s.back() == '1')
		{
			s.back() = '0';
		}
		else
		{
			ll n = s.size() - 1, m = s.size();
			for (n; n >= 0; n--)//O(n)
			{
				if (s[n] == '1')break;
			}
			s[n] = '0';
			n++;

			for (n; n < m; n++)
				s[n] = '1';
		}
		for (int i = 1; i < ll_lenght; i++)//O(63)
			s[i] = (s[i] - '0' + 1) % 2 + '0';
	}
	return s;
}
std::string add(std::string a, std::string b)
{
	ll z = 0,now;
	for (ll i = ll_lenght-1; i >= 0; i--)//O(64)
	{
		now = (a[i] - '0' )+ (b[i] - '0') + z;
		//std::cout<<now<<" " << a[i] <<" "<<b[i] << '\n';

		a[i] = (char)(now % 2 + '0');
		z = now / 2;
	}
	return d_to_p(a);
	//return a;
}
int main()
{
	ll n,m;
	std::string a, b;
	hat(2);
	std::cout << "Find the sum of binary numbers given in natural form.\n";
	std::cout << "Perform addition in two's complement code. Express your answer directly.\n\n";
	while (true)
	{
		std::cout << "Write a and b:\n";
		n = cinll();
		m = cinll();
		//std::cout << m<<'\n';
		a = to_d(to_ob(n));
		b = to_d(to_ob(m));

		std::cout << a << '\n' << b << '\n' <<"a + b = ";
		std::cout << add(a, b);
    }
}