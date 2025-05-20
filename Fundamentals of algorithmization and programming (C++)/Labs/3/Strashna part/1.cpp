#include <iostream>

void hat(short int variant)
{
	std::cout << "Completed the task : Dmitry Levshukov\n";

	if (variant != -1)
	{
		std::cout << "Variant: " << variant<<"\n";
	}

	std::cout << "The task: ";
}
int main()
{
	long long i = 1, a, b, sum = 0;

	hat(-1);
	std::cout << "Find sum (i = 1..30) of (a[i] - b[i]) ^ 2\n\n       i when i odd\na[i] = \n       i/2 when i even\n\n\n";
	std::cout << "       i ^ 2 when i odd\nb[i] = \n       i ^ 3 when i even\n\n";

	
	while (i <= 30)
	{
		if (i % 2 == 0)
		{
			a = i / 2;
			b = i * i * i;
		}
		else
		{
			a = i;
			b = i * i;
		}
		sum += (a - b) * (a - b);
		i++;
	}


	std::cout << "sum = " << sum;
}