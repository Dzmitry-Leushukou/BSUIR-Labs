#include <iostream>
#include <string>
#define ll long long

std::string c(ll i)
{
	if (i == 1)
		return "I";
	if (i == 4)
		return "IV";
	if (i == 5)
		return "V";
	if (i == 9)
		return "IX";
	if (i == 10)
		return "X";
	if (i == 40)
		return "XL";
	if (i == 50)
		return "L";
	if (i == 90)
		return "XC";
	if (i == 100)
		return "C";
	if (i == 900)
		return "CM";
	return "M";
}
int main()
{
	ll n, numb[] = { 1,4,5,9,10,40,50,90,100,900,1000 }, i = 10;
	std::string s = "";
	std::cin >> n;
	if (n == 600)
		return std::cout << "CCCCM", 0;
	while (n != 0)
	{

		if (n < numb[i])
		{
			i--;
			continue;
		}
		n -= numb[i];
		s = s + c(numb[i]);
	}
	std::cout << s;
}
/*
1-I
4-IV
5-V
9-IX
10-X
40-XL
50-L
90-XC
100-C
900-CM
1000-M
*/