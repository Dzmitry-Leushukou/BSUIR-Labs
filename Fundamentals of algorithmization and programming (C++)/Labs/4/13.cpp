#include <iostream>
#include <stdlib.h>
//#include <D:\Programming\BSUIR\Labs\Hat.h>
#define ll long long



#pragma once
#include<iostream>

void hat(short int variant)
{
    std::cout << "Completed the task : Dmitry Levshukov\n";

    if (variant != -1)
    {
        std::cout << "Variant: " << variant << "\n";
    }

    std::cout << "The task: ";
}

long double cinld()
{
    long double number;


    while (!(std::cin >> number) || (std::cin.peek() != '\n'))
    {
        std::cin.clear();


        while (std::cin.get() != '\n');


        std::cout << "Not correct input. Write again\n";
    }


    return number;
}


long long cinll()
{
    long long number;


    while (!(std::cin >> number) || (std::cin.peek() != '\n'))
    {
        std::cin.clear();


        while (std::cin.get() != '\n');


        std::cout << "Not correct input. Write again\n";
    }


    return number;
}


bool out(ll i, ll j, ll n)
{
    //up and down
    ll jL = n / 2 - (n / 4), jR = n - n / 4 - 1;


    if (i < n / 4 && j >= jL && j <= jR)
    {
        return false;
    }


    if (i >= n - n / 4 && j >= jL && j <= jR)
    {
        return false;
    }


    //left and right
    ll iL = n / 2 - (n / 4), iR = n - n / 4 - 1;


    if (j < n / 4 && i >= iL && i<= iR)
    {
        return false;
    }


    if (j >= n - n / 4 && i >= iL && i <= iR)
    {
        return false;
    }

    //Else
    return true;
}
void sq(ll n)
{
    
    ll x = n * n, r = 1;


    for (int i = 0; i < n; i++)
    {

        for (int j = 0; j < n; j++, x--, r++)
        {

            if (out(i, j, n))
            {
                std::cout << x << '\t';
            }
            else
                std::cout << r << '\t';

        }

        std::cout << '\n';

    }
 
}

void nch(ll n, ll sx, ll fx)
{
    ll** a = nullptr;
    a = (ll**)malloc(n * sizeof(ll*));

    for (ll i = 0; i < n; ++i)
    {
        a[i] = (ll*)malloc(n * sizeof(ll));

        for (ll j = 0; j < n; j++)
            a[i][j] = 0;
    }

    ll i = 0, j = n / 2;


    while (sx <= fx)
    {
        a[i][j] = sx;
        sx++;
        i--;
        j++;

        //up line
        if (i < 0 && j < n)
        {
            
            if (a[n - 1][j] != 0)
            {
                i += 2;
                j--;
            }
            else
                i = n - 1;
            continue;
        }

        //right col
        if (i >= 0 && j >= n)
        {

            if (a[i][0] != 0)
            {
                i += 2;
                j--;
            }
            else
                j=0;
            continue;
        }

        //right and up;
        if (j >= n && i < 0)
        {
            i += 2;
            j--;
            continue;
        }


        if (a[i][j] != 0)
        {
            i += 2;
            j--;
        }


    }


    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            
        std::cout << a[i][j] << "\t";
        }
        std::cout << '\n';
    }


    for (ll i = 0; i < n; ++i)
    {
        free(a[i]);
        a[i] = nullptr;
    }

    free(a);
    a = nullptr;
}

void d(ll n)
{
    ll nn = n;
    n /= 2;

    ll** a = nullptr;
    ll** b = nullptr;
    ll** c = nullptr;
    ll** d = nullptr;

    a = (ll**)malloc(n * sizeof(ll*));
    b = (ll**)malloc(n * sizeof(ll*));
    c = (ll**)malloc(n * sizeof(ll*));
    d = (ll**)malloc(n * sizeof(ll*));


    for (ll i = 0; i < n; ++i)
    {
        a[i] = (ll*)malloc(n * sizeof(ll));
        b[i] = (ll*)malloc(n * sizeof(ll));
        c[i] = (ll*)malloc(n * sizeof(ll));
        d[i] = (ll*)malloc(n * sizeof(ll));
        for (ll j = 0; j < n; j++)
            a[i][j] = b[i][j] = c[i][j] = d[i][j] = 0;
    }

    ll i = 0;
    ll j = n / 2;
    ll sx = 1;
    ll fx = n*n;


    while (sx <= fx)
    {
        a[i][j] = sx;

        sx++;
        i--;
        j++;

        //up line
        if (i < 0 && j < n)
        {

            if (a[n - 1][j] != 0)
            {
                i += 2;
                j--;
            }
            else
                i = n - 1;
            continue;
        }

        //right col
        if (i >= 0 && j >= n)
        {

            if (a[i][0] != 0)
            {
                i += 2;
                j--;
            }
            else
                j = 0;
            continue;
        }

        //right and up;
        if (j >= n && i < 0)
        {
            i += 2;
            j--;
            continue;
        }


        if (a[i][j] != 0)
        {
            i += 2;
            j--;
        }
    }
    fx = n*n+n*n;
    i = 0;
    j = n / 2;
    
    while (sx <= fx)
    {
        b[i][j] = sx;

        sx++;
        i--;
        j++;

        //up line
        if (i < 0 && j < n)
        {

            if (b[n - 1][j] != 0)
            {
                i += 2;
                j--;
            }
            else
                i = n - 1;
            continue;
        }

        //right col
        if (i >= 0 && j >= n)
        {

            if (b[i][0] != 0)
            {
                i += 2;
                j--;
            }
            else
                j = 0;
            continue;
        }

        //right and up;
        if (j >= n && i < 0)
        {
            i += 2;
            j--;
            continue;
        }


        if (b[i][j] != 0)
        {
            i += 2;
            j--;
        }
    }
    fx = n*n*3;
    i = 0;
    j = n / 2;
    while (sx <= fx)
    {
        c[i][j] = sx;

        sx++;
        i--;
        j++;

        //up line
        if (i < 0 && j < n)
        {

            if (c[n - 1][j] != 0)
            {
                i += 2;
                j--;
            }
            else
                i = n - 1;
            continue;
        }

        //right col
        if (i >= 0 && j >= n)
        {

            if (c[i][0] != 0)
            {
                i += 2;
                j--;
            }
            else
                j = 0;
            continue;
        }

        //right and up;
        if (j >= n && i < 0)
        {
            i += 2;
            j--;
            continue;
        }


        if (c[i][j] != 0)
        {
            i += 2;
            j--;
        }
    }
    fx = 4*n * n;
    i = 0;
    j = n / 2;


    while (sx <= fx)
    {
        d[i][j] = sx;

        sx++;
        i--;
        j++;

        //up line
        if (i < 0 && j < n)
        {

            if (d[n - 1][j] != 0)
            {
                i += 2;
                j--;
            }
            else
                i = n - 1;
            continue;
        }

        //right col
        if (i >= 0 && j >= n)
        {

            if (d[i][0] != 0)
            {
                i += 2;
                j--;
            }
            else
                j = 0;
            continue;
        }

        //right and up;
        if (j >= n && i < 0)
        {
            i += 2;
            j--;
            continue;
        }


        if (d[i][j] != 0)
        {
            i += 2;
            j--;
        }
    }


    ///work with it
    
    n *= 2;
    ll w = n / 4;//wide of left swap figure
    ll fl = 0;
    i = 0;
    j = 0;
    n /= 2;


    for ( i = 0; i < n; i++)
    {

        for (j = 0; j < w+fl; j++)
        {
            if (i == n / 2 && j == 0)
            {
                fl = 1;
                continue;
            }
            std::swap(a[i][j], d[i][j]);
        }

        fl = 0;
    }
    i = 0;
    j = 0;


    for ( i = 0; i < n; i++)
    {
        for ( j = n-n/4-1; j < n; j++)
        {
            std::swap(b[i][j], c[i][j]);
        }
    }

    for (int i = 0; i < n; i++)
    {
        
        for (int j = 0; j < n; j++)
        {

            std::cout << a[i][j] << "\t";
        }

        for (int j = 0; j < n; j++)
        {

            std::cout << c[i][j] << "\t";
        }

        std::cout << '\n';
    }
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {

            std::cout << d[i][j] << "\t";
        }
        for (int j = 0; j < n; j++)
        {

            std::cout << b[i][j] << "\t";
        }
        std::cout << '\n';
    }
    for (ll i = 0; i < n; ++i)
    {
        free(a[i]);
        free(b[i]);
        free(c[i]);
        free(d[i]);
        a[i] = b[i] = c[i] = d[i] = nullptr;
    }

    free(a);
    a = nullptr;
    free(b);
    b = nullptr;
    free(c);
    c = nullptr;
    free(d);
    d = nullptr;
}
void ms(ll n)
{
    if (n % 4 == 0)
        return sq(n);

    if (n % 2 == 0)
        return d(n);

        return nch(n, 1, n * n);
}

int main()
{
    ll n;

    hat(-1);
    std::cout << "Build magic square with n*n size (n<65537).\n\n";


    while (true)
    {
        std::cout << "Write n:\n";

        n = cinll();


        std::cout << "Magic square:\n";

       /* if (n > 65536)
        {
            std::cout << "Wrong input try again\n";
            continue;
        }
        */
        if (n == 2)
        {
            std::cout << "Does not exist\n";
            continue;
        }
        
        ms(n);
        
    }
}
