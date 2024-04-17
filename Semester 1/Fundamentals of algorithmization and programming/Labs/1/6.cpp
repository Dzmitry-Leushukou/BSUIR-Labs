#include <iostream>

int main()
{
    long long a, b;
    std::cin >> a >> b;
    if (a == b && a == 0)
    {
        std::cout << "ERROR";
    }
    else
        if ((a != 0 && b % a == 0) || (b != 0 && a % b == 0))
            std::cout << "YES";
        else
            std::cout << "NO";
}