#include <iostream>

int main()
{
	long long k;
	std::cin >> k;
	std::setlocale(LC_ALL, "");
	if (k == 1 || (k % 10 == 1 && k % 100 != 11)) //гриб
		std::cout << "Мы нашли "<<k<<" гриб в лесу";
	else
	if(k>1&&k<5||(k % 10 > 1 && k % 10 <5 && (k % 100 < 11 || k % 100 >15)))///гриба
		std::cout << "Мы нашли " << k << " гриба в лесу";
	else
		std::cout << "Мы нашли " << k << " грибов в лесу";
}
