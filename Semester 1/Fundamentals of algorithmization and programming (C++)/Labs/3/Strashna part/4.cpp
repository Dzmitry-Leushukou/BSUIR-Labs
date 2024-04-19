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


        while (std::cin.get() != '\n');


		std::cout << "Not correct input. Write again\n";
	}


	return number;
}

int main()
{
    long long n, sum = 0, i = 0;

    hat(-1);
    std::cout << "Calculate the sum of even numbers in two ways (with and without a loop)\n          on the interval from 1 to the number entered by the user (n).\n\n";
    std::cout << "Write n:\n";

    n = cinll();


    for (i = 1; i <= n; i++)
    {
        if (i % 2 == 0)
            sum += i;
    }


    std::cout << "Sum (with loop) = " << sum << "\nFormula (wihtout loop) = " << n * (n + 1) / 2 - (n / 2 + n % 2) * (n / 2 + n % 2);
    //n^2 - sum first n odd numbers
    //n*(n+1)/2  - sum first n numbers
}