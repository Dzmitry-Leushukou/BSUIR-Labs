#include <iostream>
#include <random>
#include <ctime>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long
#define ld long double

std::mt19937 gen(time(nullptr));
ld get(ll q, ll w)
{
    ld res;
    short int sign = 1 * (-1 * (gen() % 2));

    ll c = gen() % q, d = gen() % w;

    if (c == 0)
    {
        if (gen() % 2 == 0 || d == 0)
            return 0.0;
    }
    res = (ld)c;
    ld d1 = d;
    while (d1 >= (ld)1.0)
        d1 /= (ld)10.0;
    res += d1;
    res *= (ld)sign;
    return res;
}
ll get_kol(ld **a,ld q, ll n, ll m)
{
    ll kol = 0;
    for (ll i = 0; i < n; i++)
    {
        for (ll j = 0; j < m; j++)
        {
            if (a[i][j] == q)
                kol++;
        }
    }
    return kol;
   
}

void fullrev(ld **a, ll n, ll m)
{
    for (ll i = n - 1; i >= 0; i--)
    {
        for (ll j = m - 1; j >= 0; j--)
        {
            std::cout << a[i][j] << '\t';
        }
        std::cout << '\n';
    }
}
int main()
{
    
    ll n, m, q,w;
    
    hat(-1);
    std::cout << "Create a two-dimensional dynamic array of real numbers. Determine whether there are elements with a zero value among them.\n";
    std::cout << "If they occur, determine their indices and total number.\n";
    std::cout << "Rearrange the elements of this array in reverse order and display it on the screen.\n\n";
    while (true)
    {
        
        q = w = 20;
        n = gen() % q + 1;
        m = gen() % w + 1;
        std::cout << "Size of matrix: " << n << " x " << m << '\n';
        
        ld** a = nullptr;
            a = (ld**)malloc(n * sizeof(ld*));
        for (ll i = 0; i < n; i++)
            a[i] = (ld*)malloc(m * sizeof(ld));
        q = w = 1000;
        ll kol = 0;
        std::cout << "Matrix:\n";
        for (ll i = 0; i < n; i++)
        {
            for (ll j = 0; j < m; j++)
            {
                a[i][j] = get(q,w);
                std::cout << a[i][j] << '\t';
            }
            std::cout << '\n';
        }
        std::cout <<"Total number: "<< get_kol(a, (ll)0.0, n, m) << '\n'<<"Index(es):\n";
        for (ll i = 0; i < n; i++)
        {
            for (ll j = 0; j < m; j++)
            {
                if (a[i][j] == (ld)0.0)
                    std::cout << i << " " << j << '\n';
            }
        }
        std::cout << "Reverse matrix:\n";
        fullrev(a, n, m);
        for (ll i = n-1; i >=0 ; i--)
        {
            free(a[i]);
            a[i] = nullptr;
        }
        free(a);
        a = nullptr;
        system("pause");
    }
}