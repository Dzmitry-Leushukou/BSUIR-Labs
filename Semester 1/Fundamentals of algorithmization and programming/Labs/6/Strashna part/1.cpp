#include <iostream>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long


short int get_min(short int a[], short int siz)
{
    short int ans = 30000;
    for (int i = 0; i < siz; i++)
    {
        if (a[i] != 0)
            ans = std::min(ans, a[i]);
    }
    return ans;
}
int main()
{
    while (true)
    {
        hat(2);
        std::cout << "In a string consisting of groups of zeros and ones, find and display the shortest group.\n";

        char a[100];
        short int ones[100] = {}, zeros[100] = {}, i = 0;


        while (true) {
            std::cout << "Write string: ";

            i = 0;
            bool fl = false;
            do {
                a[i] = getchar();


                if (a[i] == '1')
                    ones[i] = 1, zeros[i] = 0;


                if (a[i] == '0')
                    ones[i] = 0, zeros[i] = 1;


                if (a[i] != '1' && a[i] != '0')
                {
                    fl = true;
                    break;
                }


                if (i > 0)
                {

                    if (a[i] == '1')
                        ones[i] += ones[i - 1];


                    if (a[i] == '0')
                        zeros[i] += zeros[i - 1];

                }

                i++;
            } while (a[i - 1] != '\n');


            if (fl)
            {
                std::cout << "WRONG INPUT\n";
                continue;
            }


            short int siz = i;


            for (i = siz - 2; i >= 0; i--)
            {

                if (zeros[i] > 0 && zeros[i + 1] > 0)
                    zeros[i] = zeros[i + 1];


                if (ones[i] > 0 && ones[i + 1] > 0)
                    ones[i] = ones[i + 1];
                
            }

            std::cout << std::min(get_min(zeros, siz), get_min(ones, siz)) << '\n';
        }
    }
}
