#include<string>
#define ll long long
#define ld long double
struct t
{
	std::string name;
	ll kol = 0;
	ld price = 0;
	std::string dat;
	ll expiration = 0;


	bool operator == (t& b)
	{
		return (name == b.name && kol == b.kol && price == b.price && dat == b.dat && expiration == b.expiration);
	}
};