#include <iostream>
#include <iomanip>

int main()
{
    long double a, b, c, k;
    long long x, y;
    std::cin >> x >> y >> a >> b >> c >> k;
    if (x > y)
        y = 0;
    else if (x < y)
        x = 0;
    else
        x = y = 0;
    std::cout << x<<" "<<y<<"\n";
    if (a > b && a > c)
    {
        a -= k;
    } else
    if (b > a && b > c)
    {
        b -= k;
    } else
    if (c > b && c > a)
    {
        c -= k;
    }
    std::cout << a << " " << b << " " << c;
}
