#include <iostream>
#include <iomanip>

int main()
{
    long long a1, a100;
    std::cin >> a1 >> a100;
    //a100=a1+99d
    //
    long double d = (a100 - a1) / 99;
    // s = (2*a1 + d * (n-1)) * n / 2
    long double s = (2.0 * a1 + d * 69.0) * 35.0;
    std::cout << std::setprecision(10) << d << "\n" << s;


}