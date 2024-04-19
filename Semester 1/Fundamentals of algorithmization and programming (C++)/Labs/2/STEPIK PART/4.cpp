#include <iostream>
#include <iomanip>

int main()
{
    long double a, b, c, d;
    std::cin >> a >> b >> c >> d;
    if (c == 0.0)
    {
        std::cout << "ERROR";
    }else
    if (c >= d && a < d)
    {
        std::cout << std::setprecision(20) << a + b / c;
    }
    else if (c < d && a >= d)
    {
     std::cout << std::setprecision(20) << a - b / c;
    }
    else std::cout << 0;
}
