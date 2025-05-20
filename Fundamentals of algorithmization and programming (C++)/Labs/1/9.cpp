#include <iostream>

int main()
{
	long long t1, m1, all_in_minutes1;
	long long t2, m2, all_in_minutes2;
	long long alltime;
	std::cin >> t1 >> m1 >> t2 >> m2;
	all_in_minutes1 = t1 * 60 + m1;
	all_in_minutes2 = t2 * 60 + m2;
	if (all_in_minutes1 >= all_in_minutes2)
	{
		alltime = 1440 - all_in_minutes1 + all_in_minutes2;
	}
	else
	{
		alltime = all_in_minutes2 - all_in_minutes1;
	}
	std::cout << "Was spent " << alltime << " minutes or "<<alltime/60 <<" hours and "<<alltime%60<<" minutes";
}