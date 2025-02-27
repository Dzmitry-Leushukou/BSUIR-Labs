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


int main()
{
    ll n;

    hat(-1);
    std::cout << "It is necessary to determine the diagonal with the largest sum of numbers\n";
    std::cout << "          in a three - dimensional dynamic array of size n ^ 3 consisting of non - negative integers.\n\n";


    while (true) 
    {
        std::cout << "Write n:\n";
        n = cinll();

        std::cout << "Array elements (After wrote element push Enter):\n";
        ll ***a = nullptr;
        a = (ll***)malloc(n * sizeof(ll**));
        for (ll i = 0; i < n; ++i) 
        {
            a[i] = (ll**)malloc(n * sizeof(ll*));
            for (ll j = 0; j < n; ++j)
            {
                a[i][j] = (ll*)malloc(n * sizeof(ll));
                for (ll k = 0; k < n; k++)
                {
                    a[i][j][k] = cinll();
                }
            }
        }
 


        ll dd = 0, j = 0, k = 0, du = n - 1, sum1 = 0, sum2 = 0, sum = 0, sum3 = 0, sum4 = 0;
        while (j < n)
        {
            sum1 += a[dd][j][k];
            sum2 += a[dd][n - j - 1][n - k - 1];
            sum3 += a[du][j][k];
            sum4 += a[du][n - j - 1][n - k - 1];
            j++;
            k++;
        }
        sum = std::max(sum1, std::max(sum2, std::max(sum3, sum4)));

        dd = 0, j = 0, k = 0, du = n - 1, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
        while (j < n)
        {
            sum1 += a[j][dd][k];
            sum2 += a[n - j - 1][dd][n - k - 1];
            sum3 += a[j][du][k];
            sum4 += a[n - j - 1][du][n - k - 1];
            j++;
            k++;
        }
        sum = std::max(sum, std::max(sum1, std::max(sum2, std::max(sum3, sum4))));

        dd = 0, j = 0, k = 0, du = n - 1, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
        while (j < n)
        {
            sum1 += a[j][k][dd];
            sum2 += a[n - j - 1][n - k - 1][dd];
            sum3 += a[j][k][du];
            sum4 += a[n - j - 1][n - k - 1][du];
            j++;
            k++;
        }
        sum = std::max(sum, std::max(sum1, std::max(sum2, std::max(sum3, sum4))));

        dd = 0, j = 0, k = 0, du = n - 1, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
        while (j < n)
        {
            sum1 += a[j][k][dd];
            sum2 += a[n - j - 1][n - k - 1][dd];
            sum3 += a[j][k][du];
            sum4 += a[n - j - 1][n - k - 1][du];
            j++;
            k++;
            dd++;
            du--;
        }
        sum = std::max(sum, std::max(sum1, std::max(sum2, std::max(sum3, sum4))));


        std::cout << "Largest sum = " << sum << '\n';
        
        for (ll i = 0; i < n; ++i)
        {
          
            for (ll j = 0; j < n; ++j)
            {
                free(a[i][j]);
                a[i][j] = nullptr;
            }

            free(a[i]);
            a[i] = nullptr;
        }

        free(a);
        a = nullptr;
    }
}
