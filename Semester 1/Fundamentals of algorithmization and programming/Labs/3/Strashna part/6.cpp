#include <iostream>
#include <cmath>
#include <iomanip>



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
    //lnx+3*tg(x)+sqrt(x)
    long long kol;
    long double step = 2.0, x = 2.0, ans = 1e18, ansx, f = 0.0;

    hat(2);
    std::cout << "Approximately find the root of the equation ln(x) + tan(x) + sqrt(x) = 0, x = [2; 4]\n          using the following algorithm:\n";
    std::cout << "          1. Iterate through the values from the beginning to the end of the interval with a certain step\n";
    std::cout << "          2. Find the value of the function that is minimal in absolute value.\n";
    std::cout << "          The argument by which it is achieved is we consider it to be the root of the equation.\n\n";

  
    std::cout << "Write the number of parts into which the range of acceptable values x must be divided: ";
    kol = cinll();
    
    step /= kol;


    while (x <= 4.0)
    {
        f = logl(x) + 3.0 * tanl(x) + sqrtl(x);


        if (fabsl(f) < ans)
            ans = fabsl(f), ansx = x;


        x += step;
    }


    std::cout << "Solution of the equation: x = " << std::setprecision(20) << ansx;
}