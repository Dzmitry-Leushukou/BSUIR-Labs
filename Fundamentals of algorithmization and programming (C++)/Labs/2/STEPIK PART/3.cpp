#include <iostream>
#include <cmath>
#include <iomanip>

int main()
{
    long double k, x, y, ans = 2*1e18;
    std::cin >> k >> x >> y;
    if (k < 0.0)
    {
        k *= (-1.0);
        x *= (-1.0);
        y *= (-1.0);
    }
    if (x >= 0.0 && x <= k)
    {
        if (ans > fabsl(y))
            ans = fabsl(y);
        if (ans > fabsl(y - k))
            ans = fabsl(y - k);
    }
    if (y >= 0.0 && y <= k)
    {
        if (ans > fabsl(x))
            ans = fabsl(x);
        if (ans > fabsl(x - k))
            ans = fabsl(x - k);
    }
    if (ans > sqrtl(x * x + y * y))
        ans = sqrtl(x * x + y * y);

    if (ans > sqrtl((x - k) * (x - k) + (y * y)))
        ans = sqrtl((x - k) * (x - k) + (y * y));

    if (ans > sqrtl(x * x + (y - k) * (y - k)))
        ans = sqrtl(x * x + (y - k) * (y - k));

    if (ans > sqrtl((x - k) * (x - k) + (y - k) * (y - k)))
        ans = sqrtl((x - k) * (x - k) + (y - k) * (y - k));
  
    std::cout << std::setprecision(22) << ans;
}

/*

0;0                          0;k




k;0                          k;k
*/