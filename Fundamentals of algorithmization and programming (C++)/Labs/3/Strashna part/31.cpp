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


int main()
{
    const long double pi = 3.141592653589793238462643383279502884197;
    long double a = 0.0, b = pi / 2, m = 20.0, h = (b - a) / m, x = a;

    hat(-1);
    std::cout << "Find y = sin(x) - cos(x), x belongs to the segment [A;B], where A = 0, B = pi/2\n\n";
    std::cout << std::setprecision(12);


    for (int i = 0; x <= b; i++, x = a + i * h)
    {
        std::cout << "[" << i << "] y(" << x << ") = " << sin(x) - cos(x) << '\n';
    }

}