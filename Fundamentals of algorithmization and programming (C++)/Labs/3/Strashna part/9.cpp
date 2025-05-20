#include <iostream>


void hat(short int variant)
{
	std::cout << "Completed the task : Dmitry Levshukov\n";

	if (variant != -1)
	{
		std::cout << "Variant: " << variant << "\n";
	}

	std::cout << "The task: ";
}

long long cinll() 
{
	long long number;
	while (!(std::cin >> number) || (std::cin.peek() != '\n'))
	{
		std::cin.clear();

		while (std::cin.get() != '\n')
			;

		std::cout << "Not correct input. Write again\n";
	}
	return number;
}

long double cinld()
{
	long double number;
	while (!(std::cin >> number) || (std::cin.peek() != '\n'))
	{
		std::cin.clear();

		while (std::cin.get() != '\n');
			
		std::cout << "Not correct input. Write again\n";
	}
	return number;
}
int main()
{
	long long a, n, s1, s2, q = 4, b1 = 1, b2 = -2, qn = 1;
	
	hat(-1);
	std::cout << "Calculate a - 2a + 4a - 8a + ... + ( 2 ^ (n - 1) ) * ( (−1) ^ (n - 1) ) * a\n\n";


	std::cout << "Write a:\n";
	a = cinll();
	std::cout << "Write number of members:\n";
	n = cinll();
	

	
	for (int i = 1; i <= n / 2 + n % 2; ++i)
		qn *= q;

	s1 = (b1 * (qn - 1)) / (q - 1);

	//std::cout << s1<<" qn = "<<qn << '\n';

	qn = 1; 
	for (int i = 1; i <= n/2; ++i)
		qn *= q;

	s2 = (b2 * (qn - 1)) / (q - 1);

	//std::cout << s2 << " qn = " << qn << '\n';

	std::cout << "Result: " << (s1 + s2) * a;
}
