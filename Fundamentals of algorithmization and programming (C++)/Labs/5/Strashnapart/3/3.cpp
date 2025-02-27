#include <iostream>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long

ll *v = nullptr;
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


void check(ll x, ll i, ll j)
{
    if (i != j)
        return;
    if (x % 2 == 0)
        pb(x);
}


ll multiplication(ll *v, ll v_size)
{
    ll ans = 1;


    for (int i = 0; i < v_size; i++)
        ans*= v[i];

    if(v_size!=0)
    return ans;
    return 0;
}

int main()
{
    ll n, k, a;
    hat(-1);
    std::cout << "Create a dynamic array of elements, working on the main diagonals of the matrix n*k and have an even value.\n";
    std::cout << "Calculate the product of dynamic array elements\n\n";
    while (true)
    {
        //std::cin >> n >> k;
        std::cout << "Write n(push Enter after end of input):\n";
        n = cinllmy();
        std::cout << "Write k(push Enter after end of input):\n";
        k = cinllmy();

        std::cout << "Write matrix elements(push Enter after every end of input element):\n";
        for (int i = 0; i < n; i++)
            for (int j = 0; j < k; j++)
            {
                //std::cin >> a;

                a = cinllmy();
                check(a, i, j);
            }
        
        std::cout << "Dynamic array has "<<v_size<<" elements";
        
        if (v_size != 0) 
        {
            std::cout << "\nElements: ";
            for (int i = 0; i < v_size; i++)
                std::cout << v[i] << " ";
        }
           
        std::cout << '\n';
        std::cout <<"Multipliation = "<< multiplication(v, v_size) << '\n';
        cl();
    }
}
