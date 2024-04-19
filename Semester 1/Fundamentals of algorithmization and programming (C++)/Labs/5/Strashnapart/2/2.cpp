#include <iostream>
#include <ctime>
#include <random>
#include <Windows.h>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long


int main()
{
    HINSTANCE load;
    load = LoadLibrary(L"DynamicLib.dll");
    typedef ll(*ch)(ll*, ll, ll);
    ch Check;
    Check = (ch)GetProcAddress(load, "check");
    std::mt19937 gen(time(nullptr));
    ll n, sign;
    hat(2);
    std::cout << "For a given one-dimensional array X of N elements, check\nthat the condition -10 < x[i]^3 < 20 is satisfied for all elements\n\n";
    while (true) 
    {
        
        std::cout << "Write n:\n";
        n = cinllmy();
        ll* x = (ll*)malloc(n * sizeof(ll));
        std::cout << "Array:\n";
        for (int i = 0; i < n; i++)
        {
            if (gen() % 2 == 0)
                sign = 1;
            else
                sign = -1;

            x[i] = sign * (gen() % 30);
            std::cout << x[i] << " ";

        }
        ll incorrect = Check(x, 0, n - 1);
        std::cout << '\n' << "Total of incorrect elements: "<<incorrect<<" --> ";
        if (incorrect == 0)
            std::cout << "condition is met\n";
        else
            std::cout << "condition not met\n";
    }
    FreeLibrary(load);
}
