#include <iostream>

int main()
{
    long double a = -13.8, b = 8.9, c = 25, y;
    long long n;
    std::cin >> n;
    switch (n)
    {
        case 2:
            y = b * c - a * a;
            break;
        case 56:
            y = b * c;
            break;
        case 7:
            y = a * a + c;
            break;
        case 3:
            y = a - b * c;
            break;
        default:
            y = (a + b) * (a + b) * (a + b);

    }
    std::cout << y;
}