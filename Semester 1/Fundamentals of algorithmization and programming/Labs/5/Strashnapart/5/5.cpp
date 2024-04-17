#include <iostream>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long
#define ld long double


ll* v = nullptr;
ll v_size = 0;


void pb(ll x)
{
    v_size++;
    if (v_size == 1)
        v = (ll*)malloc(sizeof(ll));
    else
        v = (ll*)realloc(v, v_size * sizeof(ll));
    v[v_size - 1] = x;
}


void cl()
{
    free(v);
    v = nullptr;
    v_size = 0;
}

ld average(ll* v, ll n)
{
    ld u=0.0;
    for (int i = 0; i < n; i++)
        u += v[i];
    return (ld)(u/n);
}

int main()
{
    ll n, m;
    pb(1);
    hat(-1);
    std::cout << "Given a matrix of integers. Enter the values of the elements of this array from the keyboard.\n";
    std::cout << "Create a dynamic array of elements located in the even columns of the given array and having an odd value.\n";
    std::cout << "Calculate the arithmetic mean of dynamic elements array.\n\n";

    while (true)
    {
        std::cout << "Write n (Push Enter after end of input):\n";
        n = cinllmy();
        std::cout << "Write m (Push Enter after end of input):\n";
        m = cinllmy();


        ll** a = nullptr;
        a = (ll**)malloc(n * sizeof(ll*));
        for (ll i = 0; i < n; i++)
            a[i] = (ll*)malloc(m * sizeof(ll));

        std::cout << "Write matrix elements (Push Enter after every end of input of element):\n";
        for (ll i = 0; i < n; i++)
            for (ll j = 0; j < m; j++)
            {
                a[i][j] = cinllmy();
                if (j % 2 == 0 && a[i][j] % 2 != 0)
                    pb(a[i][j]);
            }
        ld u = 1;
        std::cout << "Dynamic array of elements located in the even columns of the given array and having an odd value:\n";
        for (ll i = 0; i < v_size; i++)
            std::cout << v[i] << " ";
        std::cout<<'\n'<<"The arithmetic mean of the elements of a dynamic array: "<<average(v, v_size) << '\n';
        cl();

    }
}
