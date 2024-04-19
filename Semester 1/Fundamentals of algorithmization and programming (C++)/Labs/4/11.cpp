#include <iostream>
#include <stdlib.h>
//#include <D:\Programming\BSUIR\Labs\Hat.h>

#define ll long long


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
    ll n,m;

    hat(-1);
    std::cout << "It is necessary to determine the diagonal with the largest sum of numbers\n";
    std::cout << "          in a three - dimensional dynamic array of size n ^ 3 consisting of non - negative integers.\n\n";

    while (true)
    {
        std::cout << "Write n:\n";
        n = cinll();
        m = cinll();

        std::cout << "Array elements:\n";
        char** a = nullptr;
        a = (char**)malloc(n * sizeof(char*));
        for (ll i = 0; i < n; ++i)
        {
            a[i] = (char*)malloc(m * sizeof(char));
            for (ll j = 0; j < m; ++j)
            {
                std::cin >> a[i][j];
                if (a[i][j] != '.' && a[i][j] != '*')
                {
                    std::cout << "Wrong input. Write this element again\n";
                    continue;
                }
                if (a[i][j] == '.')
                    a[i][j] = '0';
            }
        }

       

        for(ll i=0;i<n;i++)
            for (ll j = 0; j < m; j++)
            {
                if (a[i][j] == '*')
                {
                    for(int q=-1;q<=1;q++)
                        for (int w = -1; w <= 1; w++)
                        {
                            if (q == w && q == 0)continue;
                            if (a[i+q][j+w] != '*')
                                a[i+q][j+w]+=1;
                        }
                }
            }
        

        for (ll i = 0; i < n; ++i)
        {
           
            for (ll j = 0; j < m; ++j)
            {
                if (a[i][j] != '0')
                    std::cout << a[i][j];
                else
                    std::cout << '.';
            }
            std::cout << '\n';
        }
        return 0;
       
        for (ll i = 0; i < n; ++i)
        {
            
                free(a[i]);
                a[i] = nullptr;
        }
        free(a);
        a = nullptr;
    }
}
