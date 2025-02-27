#include <iostream>
#include<string.h>
#include<cmath>
#include "D:\Programming\BSUIR\Labs\Hat.h"
#define ll long long

void get_kol(char* (&s), short int* (&a))
{
	short int n = strlen(s);
	a[0] = 1;


	for (int i = 1; i < n; i++)
		if (s[i] == s[i - 1])
			a[i] = 1 + a[i - 1];
		else a[i] = 1;


	for (int i = n - 2; i >= 0; i--)
		if (s[i] == s[i + 1])
			a[i] = a[i + 1];

}

ll siz_of_int(int a)
{
	if (a >= 100)return 3;
	if (a >= 10)return 2;
	return 1;
}

ll ssize(char* (&s), short int* (&a),int n)
{
	ll ans = 0, i = 0;
	for (i; i < n; i++)
	{
		if ((i==0&&a[i]>1) ||(a[i] > 1 && s[i] != s[i - 1] ))
			ans += siz_of_int(a[i]) +1 + siz_of_int(s[i]);
		else if(a[i]==1)
			ans++;
	}
	return ans;
}


void int_to_s(char* (&s), ll& j, int n)
{
	bool fl100, fl10;
	fl100 = fl10 =  false;
	if (n / 100 != 0)
		s[j] = (n / 100) + '0', fl100 = true, j++;
	if ((n % 100) / 10 != 0 || ((n % 100) / 10 == 0 && fl100))
		s[j] = (n % 100) / 10 + '0', j++, fl10 = true;
		s[j] = (n % 10) + '0',j++;
}


int main()
{
	hat(2);
	std::cout << "Process an array of strings. If a string contains a sequence of identical characters, replace them with code 255,\n";
	std::cout<<"followed by the code of this character and the number of identical characters.\n\n\n";
	while (true)
	{

		ll siz, i, j;
		char c;


		while (true)
		{
			siz = 0;
			char* s = (char*)malloc(sizeof(char));
			j = 0;


			while (true)
			{
				char* a = (char*)malloc(101 * sizeof(char));
				for (int ind = 0; ind < 101; ind++)
					a[ind] = '\0';
				i = 0;

				std::cout << "Write text:\n";


				while (true)
				{
					c = getchar();
					a[i] = c;
					i++;
					if (c == '\n' || c == -1)
					{
						break;
					}

				}


				short int* kol = (short int*)malloc((i + 1) * sizeof(short int*));
				get_kol(a, kol);


				ll n = ssize(a, kol, strlen(a));
				s = (char*)realloc(s, sizeof(char) * n + j * sizeof(char));


				for (int i = 0; i < strlen(a); i++)
				{


					if (i + 1 == strlen(a) && a[i + 1] == -1)
						break;


					if (kol[i] > 1)
					{


						if ((i > 0 && a[i] != a[i - 1]) || (i == 0))
						{
							s[j] = 255;
							j++;
							int_to_s(s, j, (int)a[i]);
							int_to_s(s, j, (int)kol[i]);
						}


					}
					else s[j] = a[i], j++;
				}

				free(a);
				a = nullptr;


				if (c == -1)break;
			}


			for (int i = 0; i < j; i++)
				std::cout << s[i];
			std::cout << "\n";


			free(s);
			s = nullptr;
		}
	}
}
