#include <iostream>
#include <cmath>
#include <iomanip>

int main()
{   
    long double h, B, x, p, A, K, C, D, Y;
    std::cin >> x >> p >> h >> K >> C >> D;
    B = log(h);
    A = x - p;
    A = A * A * A;
    Y = 0.78 * B + A / (K * C * D);

    std::cout << std::setprecision(20) << Y;
}
/// variant 7