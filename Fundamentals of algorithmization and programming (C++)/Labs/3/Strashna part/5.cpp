#include <iostream>
#include <cmath>
#include <iomanip>



long double cinld()
{
    long double number;
    while (!(std::cin >> number) || (std::cin.peek() != '\n'))
    {
        std::cin.clear();
        while (std::cin.get() != '\n')
            ;
        std::cout << "Not correct input. Write again\n";
    }
    return number;
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

void hat(short int variant)
{
    std::cout << "Completed the task : Dmitry Levshukov\n";

    if (variant != -1)
    {
        std::cout << "Variant: " << variant << "\n";
    }

    std::cout << "The task: ";
}

int main()
{
    long double x, f = 0, b, fact = 1, z = 0, q = 2;
    long long n;


    hat(1);
    std::cout << "It is necessary to expand the function Y(x) into the series S(x),\n";
    std::cout << "          use it to find the value of the function and compare it with the value calculated using standard functions.\n";
    std::cout << "Y(x) = sin(x)\nS(x)=x - ( (x ^ 3) / (3!) ) - ... + ((-1) ^ n) * (x ^ (2n + 1)) / ((2n + 1)!)\n\n";


    std::cout << "Write number of series members: ";
    b = cinld();

    std::cout << "Write number of requests: ";
    n = cinll();

    std::cout << "\n";
    std::cout << std::setprecision(20);


    for (int i = 0; i < n; i++)
    {
        std::cout << "\t\t=" << i + 1 << "=\n\n";
        std::cout << "Write your request (0.1 <= x <= 1): ";

        x = cinld();


        if (x < 0.1 || x>1)
        {
            std::cout << "INCORRECT\n";
            i--;
            continue;
        }


        std::cout << "When we use standart function ( Y(x) ) sin(x) = " << sin(x) << '\n';

        long double sx = x;
        

        for (int j = 1; j <= b; j++, sx *= x * x, fact *= q * (q + 1), q += 2)
        {
            if ((j+1) % 2 == 0)
            {
                z = 1.0;
            }
            else z = -1.0;
            //std::cout << f << " + " << z << " * " << sx << " / " << fact << " ";
            f = f + z * sx / fact;
            //std::cout << " = " << f<<'\n';
        }


        std::cout << "When we use Taylor's series ( S(x) ) sin(x) = " << f << '\n' << '\n';

    }
    

}