#include <iostream>
#include <iomanip>
#include <cmath>


void hat(short int variant)
{
    std::cout << "Completed the task : Dmitry Levshukov\n";

    if (variant != -1)
    {
        std::cout << "Variant: " << variant << "\n";
    }

    std::cout << "The task: ";
}

long double sint(long double x)
{
    long double fact = 1, sx = x, z = 0, q = 2, f = 0;


    for (int j = 1; j <= 10; j++, sx *= x * x, fact *= q * (q + 1), q += 2)
    {
        if ((j + 1) % 2 == 0)
        {
            z = 1.0;
        }
        else z = -1.0;
        //std::cout << f << " + " << z << " * " << sx << " / " << fact << " ";
        f = f + z * sx / fact;
        //std::cout << " = " << f<<'\n';
    }


    return f;
}

long double cost(long double x)
{
    long double fact = 1, sx = x*x, z = 0, q = 2, f = 0;


    for (int j = 1; j <= 10; j++, sx *= x * x, fact *= q * (q - 1), q += 2)
    {
        if ((j + 1) % 2 == 0)
        {
            z = 1.0;
        }
        else z = -1.0;
        //std::cout << f << " + " << z << " * " << sx << " / " << fact << " ";
        f = f + z * sx / fact;
        //std::cout << " = " << f<<'\n';
    }


    return f;
}

long double ln(long double a)
{
    //(1+x)/(1-x)=a
    //1+x=a-x*a
    //x=a-1/1+a
    long long k = 0;
    long double f = 0;
    const long double LN2 = 0.693147180559945309417232121458176568;


    while (a > 2.0)
    {
        a /= 2.0;
        k++;
    }


    long double x = (a - 1.0) / (a + 1.0), b = x;


    for (int j = 1; j <= 10000000; j++, b *= x*x)
    {
       
        f = f + b / (2 * j - 1);
        //std::cout << " = " << f<<'\n';
    }


    return 2*f + k*LN2;
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
    const long double pi = 3.141592653589793238462643383279502884197;
    long double x, lnm = 0;


    hat(-1);
    std::cout << "Find the minimum of functions a,b,c\na = sin(x)\nb = cos(x)\nc = ln(x)\n\n\n";


    std::cout << "Write x: ";
    x = cinld();


    std::cout << std::setprecision(20);

    
    if (x != 0)
        lnm = ln(fabsl(x));


    x = fmodl(x, 2.0 * pi);
    long double sinm = sint(x), cosm = cost(x);

    std::cout << "sin(x) = " << sinm << "\n";
    std::cout << "cos(x) = " << sqrtl(1 - sinm * sinm) << " " << cosm << "\n";
    std::cout << "Trigonometry         and        Teylor's line\n";


    if (x != 0)
        std::cout << "ln(x) = " << lnm << "\n";
    else
    {
        std::cout << "ln(x) undefined when x=0\n";
        lnm = 1e300;
    }


    std::cout << "Minimum value: ";


    if (lnm <= sinm && lnm <= cosm)
    {
        std::cout << "ln(x) = " << lnm;
    }
    else
    if (sinm <= lnm && sinm <= cosm)
    {
        std::cout << "sin(x) = " << sinm;
    }
    else
    if (cosm <= sinm && cosm <= lnm)
    {
        std::cout << "cos(x) = " << cosm;
    }
}