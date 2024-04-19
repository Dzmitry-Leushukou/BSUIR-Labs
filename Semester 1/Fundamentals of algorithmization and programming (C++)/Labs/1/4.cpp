#include <iostream>
#include <iomanip>
int main()
{
    long long b1, n = 17;
    std::cin >> b1;
    long double q = 1 / (n + 1),s;
    s = (long double)b1 / (1 - q);
    std::cout << std::setprecision(10) << std::fixed << s;

}