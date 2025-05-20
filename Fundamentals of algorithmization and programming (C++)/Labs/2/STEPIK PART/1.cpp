#include <iostream>

int main()
{
    long long a, b, c;
    std::cin >> a >> b >> c;
    if (a + b > c && b + c > a && a + c > b)
    {
        std::cout << "YES";
    } else
    std::cout << "NO";
}