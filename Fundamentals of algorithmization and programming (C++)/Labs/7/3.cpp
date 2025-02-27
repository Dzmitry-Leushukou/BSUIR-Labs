#include <iostream>
#include <string>
#include "D:\Programming\BSUIR\Labs\Hat.h"
#define ll long long


char base[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const short int siz = 64;
ll geti(char a)
{
	for (int i = 0; i < 36; i++)
		if (base[i] == a)
			return i;
}
std::string to_base(ll n, ll a)
{
	std::string res = "";
	ll b = abs(a);
	while (b != 0)
	{
		res = base[b % n] + res;
		b /= n;
	}

	if (a >= 0)
	{
		std::string zeroes(siz - res.size(), '0');
		return zeroes + res;
	}
	std::string zeroes(siz - res.size(), base[n-1]);
	ll m = res.size();
	//std::cout<<"!" << res << '\n';
	for (int i = 0; i < m; i++)
	{
		//std::cout << (n - geti(res[i]) - 1) % n << '\n';
		res[i] = base[(n- geti(res[i]) -1) % n];
	}
	return zeroes + res;
}
std::string to_d(std::string s, ll n)
{
	if (s[0] == '0')
		return s;
	ll q= s.size()-1;
	for (; q >= 0; q--)
		if (s[q] != base[n-1])
			break;
	//std::cout<<'\n' << "FOUND: " << q << '\n';
	s[q] = s[q]+1;
	q++;
	ll m = s.size();
	for (q; q < m; q++)
	{
		s[q] = '0';
	}
	return s;		
}

std::string sum(std::string a, std::string b,ll n)
{
	ll z = 0, now=0;
	for (ll i = siz - 1; i >= 0; i--)
	{
		now = geti(a[i])+geti(b[i]) + z;
		//std::cout<<now<<" " << a[i] <<" "<<b[i] << '\n';

		a[i] = base[now%n];
		z = now / n;
	}
	return a;
}
std::string d_to_p(std::string s, ll n)
{
	if (s[0] == '0')
		return s;
	int i = s.size()-1;
	while (s[i]=='0'&&i>=0)
	{
		i--;
	}
	s[i]--;
	i++;
	int m = s.size();
	while (i < m)
	{
		s[i] = base[n - 1];
		i++;
	}
	for (i = 1; i < m; i++)
		s[i] = base[(n - 1 - geti(s[i]))%n];
	return s;
}
ll p_to_e(std::string s,ll n)
{
	ll m = 1;
	ll res = 0;
	for (int i = siz-1; i > 0; i--,m*=n)//o(63)
	{
		res += m * geti(s[i]);
	}
	if (s[0] != 0)
		res *= (-1);
	return res;
	
}
int main()
{
	ll n, q, w;
	hat(-1);
	//std::cin >> n >> q >> w;
	while (true)
	{
		std::cout << "Write base:\n";
		n = cinll();
		if (n < 2)
		{
			std::cout << "Base must be > 2\n";
			continue;
		}
		std::cout << "Write 2 numbers in base\n";
		q = cinll();
		w = cinll();
		std::string a, b;
		//std::cout <<"!"<< to_base(n, q) << '\n';
		std::cout << to_d(to_base(n, q), n) << "\n" << to_d(to_base(n, w), n) << "\n" << "SUMS:\n";

		std::string s = sum(to_d(to_base(n, q), n), to_d(to_base(n, w), n), n);
		std::cout << "a + b = " << s << "\nor " << d_to_p(s, n) << "\nor " << p_to_e(d_to_p(s, n), n) << '\n';

		s = sum(to_d(to_base(n, q * (-1)), n), to_d(to_base(n, w), n), n);
		std::cout << "b - a = " << s << "\nor " << d_to_p(s, n) << "\nor " << p_to_e(d_to_p(s, n), n) << '\n';

		s = sum(to_d(to_base(n, q), n), to_d(to_base(n, w * (-1)), n), n);
		std::cout << "a - b = " << s << "\nor " << d_to_p(s, n) << "\nor " << p_to_e(d_to_p(s, n), n) << '\n';
		//std::cout << a << "\n" << b <<" to d: "<<to_d(b,n);
	}
}