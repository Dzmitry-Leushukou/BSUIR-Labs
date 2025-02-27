#include <fstream>
#include <iostream>
#include <string>
#include <string.h>

#include "T.h"
#include "Func.h"

#define ll long long
#define ld long double
#define _CRT_SECURE_NO_WARNINGS




void main_menu(std::fstream& file, T* &goods, const ll days[], const char del[30], ll &v_size)
{

	ll x;
	std::cout<< "What do you want to do(write number)?\n1. Add new line(s)\n2. Show list\n3. Edit line\n4. Delete line\n5. Get info\n";
	x = input(1, 7);
	if (x == 1)//ADD
	{
		std::cout<< "Choose type of input:\n1. Add n elements\n2. Add before the stop word\n0. Exit\n";
		x = input(0, 2);

		if (x == 1)
		{
			std::cout<< "Write n: ";
			ll n = input(-1, LLONG_MAX);
			while (n--)
			{
				push(file, add(1, days), goods, v_size);
			}
		}
		else
			if (x == 2)
			{
				char a;
				while (true)
				{
					std::cout<< "Do you want stop input?(Write Y if you want to stop writing, otherwise any symbol)\n";

					std::cin >> a;
					if (a == 'Y')break;
					push(file, add(1,days),goods,v_size);
				}
			}
		return;
	}
	else
		if (x == 2)//SHOW
		{
			show(file,del,v_size);
		}
		else
			if (x == 3)//EDIT
			{
				show(file, del,v_size);
				std::cout<< "Write id of line for editing: ";
				x = input(0, v_size - 1);
				std::cout<< "Write new information:\n";
				goods[x] = add(0,days);
				FILE* file2;
				fopen_s(&file2, "data.bin", "rb+");
				fseek(file2, sizeof(T) * x, 0);
				fwrite((char*)&goods[x], sizeof(T), 1, file2);
				fclose(file2);

			}
			else
				if (x == 4)///DELETE
				{
					show(file,del,v_size);
					std::cout<< "Write id of line for deleting: ";
					x = input(0, v_size - 1);
					pop(x,del,v_size, goods);
				}
				else
					if (x == 5)
					{
						ll kol = 0;
						for (int i = 0; i < v_size; i++)
						{
							if (goods[i].price > (ld)10.574)
							{
								kol++;
							}
						}
						T* for_sort = new T[kol]; 
						ll j = 0;
						for (int i = 0; i < v_size; i++)
						{
							if (goods[i].price > (ld)10.574)
							{
								for_sort[j] = goods[i];
								j++;
							}
						}
						for (int i = 1; i < kol; i++)
						{
							std::string now = for_sort[i].name;
							T temp = for_sort[i];
							for (int j = i - 1; j >= 0; j--)
							{
								if (now > for_sort[j].name)break;
								for_sort[j + 1] = for_sort[j];
								for_sort[j] = temp;
							}
						}
						std::cout << "Name\t\tAmount\t\tPrice\t\tData\n";

						for (int i = 0; i < kol; i++)
						{
							if (for_sort[i].name != del)
								std::cout << for_sort[i].name << for_sort[i].kol.amount << "\t\t" << for_sort[i].price << "\t\t" << for_sort[i].dat << '\n';

						}
					}
}


int main()
{
	//Init
	std::fstream file;
	T* goods = nullptr;
	file.open("data.bin", std::ios::out);
	file.close();
	const char del[30] = "deleteddeleteddeleteddeletdel";
	ll v_size = 0;
	const ll days[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
	while (true)
	{
		main_menu(file, goods,days,del, v_size);
	}
	delete[] goods;
	goods = nullptr;
}
