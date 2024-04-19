#include<iostream>
#include<string>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long


bool bad_input(std::string a)
{
	ll n = a.size();
	for (int i = 0; i < n; i++)//O(n)
		if (a[i] != '0' && a[i] != '1')
			return true;
	return false;
}
int main()
{
	std::string s;
	hat(2);
	std::cout << "From complementary code in reverse direction\n\n\n";
	while (true)
	{
		std::cout << "Write complemntary code:\n";
		std::cin >> s;
		while (bad_input(s))
		{
			std::cin >> s;
		}
		if (s[0] != '0')
		{
			if (s.back() == '1')
			{
				s.back() = '0';
			}
			else
			{
				ll n = s.size() - 1,m=s.size();
				for (n; n >= 0; n--)//(O(n)
				{
					if (s[n] == '1')break;
				}
				s[n] = '0';
				n++;
				for (n; n < m; n++)//(O(n)
					s[n] = '1';
			}
		}
		std::cout << s<<'\n';
	}
}