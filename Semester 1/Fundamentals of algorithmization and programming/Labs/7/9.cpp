#include <iostream>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long
std::string to3(ll a)
{
    std::string s;
    while (a != 0)
    {
        s = (char)('0' + a % 3) + s;
        a /= 3;
    }
    while (s.size() < 5)
        s = "0" + s;
    return s;
}
bool check(std::string s,ll i)
{
    return s[i] == '0';
}
bool check1(std::string s, ll i, short int* live)
{
    if (s[i] == '1')
    {
        for (ll j = 0; j < s.size(); j++)
        {
            if (live[j+1] == 0 && s[j] != '0')
                return false;
        }
        return true;
    }
    return false;
}
int main()
{
    std::string nums[241];
    bool used[241];
    short int live[6] = { 1,1,1,1,1,1 };
    for (int i = 1; i <= 240; i++)
        nums[i] = to3(i),used[i]=false;//std::cout<<i<<" " << nums[i] << std::endl;
    //std::cout << nums[i] << '\n';
    std::cout << to3(240)<<'\n';
    ll n, s = 81;
    n = cinllmy();
    while (n < 1 || n>240)
    {
        n = cinllmy();
    }
    //1 day
    std::string after;
    std::cout << "DAY 1:\n";
    for (int i = 1; i <= 5; i++) 
    {
        std::cout << i << " drunk: ";
        for (int j = 1; j <= 240; j++)
        {
            if (check(nums[j],i-1))
            {
                std::cout << j << " ";
                if (j == n) {
                    live[i] = 0;
                    after += '0';
                    //break;
                }
            }
   
        }
        std::cout << '\n';
        if (live[i])
            live[i]=2,after += '1';
       
    }
    for (int i = 1; i <= 5; i++)
    {
        std::cout << i <<" "<< ((after[i-1] == '0') ? "Dead\n" : "Alive\n");
    }
    ///DAY 2 - where nig**s dead we put 2 else 1
    std::cout << "Day 2:\n";
    std::cout << "Bits: " << after << '\n';
    ll found = 0;
    for (int i = 1; i <= 5; i++, s /=3)
    {
        std::cout << i << " drunk: ";
        if (after[i - 1] == '0')
        {
            //std::cout << live[i];
            std::cout << '\n';
            continue;
        }
        for (int j = 1; j <= 240; j++)
        {
            if (check1(nums[j], i - 1,live))
            {
                std::cout << j << " ";
                if (j == n) {
                    live[i] = 1;
                  //  break;
                }
            }

        }
        std::cout << '\n';
        found += live[i] * s;
        //std::cout << live[i];
    }
    for (int i = 1; i <= 5; i++)
    {
        std::cout << i << " ";
        if(live[i]==1||after[i-1])
            std::cout << "Dead\n";
        else
            std::cout<<"Alive\n";
    }
    std::cout << '\n'<<"=" << found;
}
/*
22220
1: 1 2
2: 3 4 5 6 7 8
3: 9 - 26
4: 27 - 80 
5: 81 - 242
*/