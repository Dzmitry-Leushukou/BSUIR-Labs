#pragma once


#include <iostream>
#include <cstdio>


#define ll long long


ll cinllmy()
{
    ll ch;
    ll notzero = 0, kolzero = 0;
    while (true)
    {
        char a[21];
        for (int i = 0; i < 20; i++)a[i] = ' ';
        bool ban = false;
        ll i = 0, c = 0;
        do
        {
            if (i == 21)break;
            c = getchar();
            a[i] = c;
            if (a[i] >= '0' && a[i] <= '9')
            {
                notzero += a[i] - '0';
                if (a[i] == '0')
                    kolzero++;
            }
            else
            {
                if (i != 0 || (i == 0 && c != '+' && c != '-'))
                    break;
            }

            if (notzero != 0)
                i++;
        } while (c != '\n');
        if (c != '\n')
        {
            while (c != '\n')
                c = getchar();
            std::cout << "Not correct input. Write again\n";
            continue;
        }

        ll ans = atoll(a);
        //std::cout<<(ll)a[0]<<" "<<(ll)a[1]<<"*";
        bool emp = true;
        if (kolzero != 0)
            a[i] = '0';
        for (int i = 0; i < 20; i++)if (a[i] != ' ' && a[i] != '\n')emp = false;
        if (!emp)
            return ans;
        else
        {
            std::cout << "Not correct input. Write again\n";
            continue;
        }
    }
}