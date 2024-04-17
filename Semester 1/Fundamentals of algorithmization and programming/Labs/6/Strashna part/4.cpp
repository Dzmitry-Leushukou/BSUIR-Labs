#include <iostream>
#include <string>
#include <string.h>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long
void add(std::string& s, char a)
{
    //std::cout << "Check " << a << " " << (char)((int)a - (int)'a' + (int)'A')<<" " << (char)((int)a - (int)'A' + (int)'a')<<'\n';
    if (s.size()==0||(s.back() !=a-'a'+ 'A'&& s.back() != a -'A' +'a'&&s.back()!=a))
       s += a;
}
int main()
{

    hat(-1);
    std::cout << "Instead of c before the letters e, i, y you will need to write the letter s, and in other cases - the letter k.\n";
    std::cout << "Instead of the letter q you will need to write the letter k, instead of the combination qu - kv, instead of x - ks,\n";
    std::cout << " and instead of w - v.Secondly, the combination of letters ph will be written as f, you and oo - as u, ee - as i, th - as z.\n";
    std::cout << "In addition, all double consonants(including those formed after replacements), which cause great difficulties for students,\n";
    std::cout << "will become single consonants, for example, apple after the reform should be written as aple.In connection with the reform,\n";
    std::cout << "a huge number of texts need to be revised.\n\n\n";
    //while (true)
   // {
        ll n;
        std::string s, ans;

        ans = "";
        std::cout<<"Write text:\n";


        while (std::getline(std::cin, s))
        {
            n = s.size();


            for (int i = 0; i < n; i++)
            {

                if (s[i] == 'c')
                {


                    if (i + 1 < n && (s[i + 1] == 'i' || s[i + 1] == 'e' || s[i + 1] == 'y'))
                    {
                        add(ans, 's');
                    }
                    else
                        add(ans, 'k');
                    continue;

                }
                if (s[i] == 'C')
                {

                    if (i + 1 < n && (s[i + 1] == 'i' || s[i + 1] == 'e' || s[i + 1] == 'y'))
                    {
                        add(ans, 'S');
                    }
                    else
                        add(ans, 'K');
                    continue;

                }
                if (s[i] == 'q')
                {
                    add(ans, 'k');


                    if (i + 1 < n && s[i + 1] == 'u')
                        ans += 'v', i++;


                    continue;
                }
                if (s[i] == 'Q')
                {
                    add(ans, 'K');


                    if (i + 1 < n && s[i + 1] == 'u')
                        ans += 'v', i++;


                    continue;
                }
                if (s[i] == 'x')
                {
                    add(ans, 'k');
                    ans += 's';
                    continue;
                }
                if (s[i] == 'X')
                {
                    add(ans, 'K');
                    ans += 's';
                    continue;
                }
                if (s[i] == 'w')
                {
                    add(ans, 'v');
                    continue;
                }
                if (s[i] == 'W')
                {
                    add(ans, 'V');
                    continue;
                }
                if (s[i] == 'p')
                {

                    if (i + 1 < n && s[i + 1] == 'h')
                    {
                        add(ans, 'f');
                        i++;
                        continue;
                    }

                }
                if (s[i] == 'P')
                {

                    if (i + 1 < n && s[i + 1] == 'h')
                    {
                        add(ans, 'F');
                        i++;
                        continue;
                    }

                }
                if (s[i] == 'y')
                {

                    if (i + 2 < n && s[i + 1] == 'o' && s[i + 2] == 'u')
                    {
                        add(ans, 'u');
                        i += 2;
                        continue;
                    }

                }
                if (s[i] == 'Y')
                {

                    if (i + 2 < n && s[i + 1] == 'o' && s[i + 2] == 'u')
                    {
                        add(ans, 'U');
                        i += 2;
                        continue;
                    }

                }
                if (s[i] == 'o')
                {

                    if (i + 1 < n && s[i + 1] == 'o')
                    {
                        add(ans, 'u');
                        i++;
                        continue;
                    }

                }
                if (s[i] == 'O')
                {

                    if (i + 1 < n && s[i + 1] == 'o')
                    {
                        add(ans, 'U');
                        i++;
                        continue;
                    }

                }
                if (s[i] == 'e')
                {
                    
                    if (i + 1 < n && s[i + 1] == 'e')
                    {
                        add(ans, 'i');
                        i++;
                        continue;
                    }

                }
                if (s[i] == 'E')
                {

                    if (i + 1 < n && s[i + 1] == 'e')
                    {
                        add(ans, 'I');
                        i++;
                        continue;
                    }

                }
                if (s[i] == 't')
                {

                    if (i + 1 < n && s[i + 1] == 'h')
                    {
                        add(ans, 'z');
                        i++;
                        continue;
                    }

                }
                if (s[i] == 'T')
                {

                    if (i + 1 < n && s[i + 1] == 'h')
                    {
                        add(ans, 'Z');
                        i++;
                        continue;
                    }

                }
                add(ans, s[i]);
            }
            ans += '\n';
       // }
        std::cout << ans;
    }
}