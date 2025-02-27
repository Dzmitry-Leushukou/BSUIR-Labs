#include <iostream>
#include "D:\Programming\BSUIR\Labs\Hat.h"
#define ll long long

char base[] = { '0','1' };
const ll siz = 64;
ll geti(char a)
{
    for (int i = 0; i < 36; i++)
        if (base[i] == a)
            return i;
}
std::string to_base(ll n, ll a)
{
    std::string res = "";
    ll b = abs(a);
    while (b != 0)
    {
        res = base[b % n] + res;
        b /= n;
    }

    if (a >= 0)
    {
        std::string zeroes(siz - res.size(), '0');
        return zeroes + res;
    }
    std::string zeroes(siz - res.size(), base[n - 1]);
    ll m = res.size();
    //std::cout<<"!" << res << '\n';
    for (int i = 0; i < m; i++)
    {
        //std::cout << (n - geti(res[i]) - 1) % n << '\n';
        res[i] = base[(n - geti(res[i]) - 1) % n];
    }
    return zeroes + res;
}
ll pl(ll a, ll b)
{
    while (b)
    {
        //std::cout << to_base(2,a) << "\n" << to_base(2, b) << '\n';
        ll c = a & b;
        //std::cout << "=" << to_base(2, c) << "\n";
        a = a ^ b;
        //std::cout << a << " ";
        b = c << 1;
        //std::cout << b << "\n";
    }
    return a;
}
ll mp(ll a,ll b)
{
    ll res= 0;
    while (b != 0)
    {
        if (b & 1)
        {
            res = pl(res, a);  
        }
        a <<= 1;
        b >>= 1;
    }
    return res;
}
ll dv(ll a, ll b)
{
    ll c = 0;

    while (a >= b) {
        ll i = 0, d = b;
        while (a >= (d << 1)) {
            i = pl(i,1);
            d <<= 1;
        }
        c |= 1 << i;
        a = pl(a, (-1)*d);
    }
    return c;
}
ll mod(ll a, ll b)
{
    return pl(a, (-1)*mp(dv(a, b), b));
}
int main()
{
    ll n;
    //std::cin >> n;
    n = cinll();
    //std::cout << mod(n, 5);
    if (!mod(n, 5))
    {
        std::cout << 5 << " ";
    }
    if (!mod(n, 47))
    {
        std::cout << 47 << " ";
    }
    if (!mod(n, 89))
    {
        std::cout << 89 << " ";
    }
}
