#include <fstream>
#include <iostream>
#include <string>

#include "T.h"
#include "Func.h"


#define ll long long
#define ld long double


void main_menu(std::fstream& file,T* &goods,ll days[],ll &v_size)
{
	
	ll x;
	std::cout << "What do you want to do(write number)?\n1. Add new line(s)\n2. Show list\n3. Edit line\n4. Delete line\n5. Get info\n";
	x = input(1, 7);
	if (x == 1)//ADD
	{
		std::cout << "Choose type of input:\n1. Add n elements\n2. Add before the stop word\n0. Exit\n";
		x = input(0, 2);

		if (x == 1)
		{
			std::cout << "Write n: ";
			ll n = input(-1, LLONG_MAX);


			while (n--)
			{
				push(file,add(1,days),goods,v_size);
			}


		}
		else
			if (x == 2)
			{
				char a;


				while (true)
				{
					std::cout << "Do you want stop input?(Write Y if you want to stop writing, otherwise any symbol)\n";

					std::cin >> a;
					if (a == 'Y')break;
					push(file, add(1,days), goods, v_size);
				}


			}
		return;
	}
	else
		if (x == 2)//SHOW
		{
			show(goods,v_size);
		}
		else
			if (x == 3)//EDIT
			{
					show(goods, v_size);
					std::cout << "Write id of line for editing: ";
					x = input(0, v_size - 1);
					std::cout << "Write new information:\n";
					goods[x] = add(0,days);
					FILE* file1;
					fopen_s(&file1, "data.txt", "r+");
					ll siz = 0;
					for (int i = 0; i < x; i++)
					{
						siz += 4 + goods[i].group.size() + goods[i].name.size() + goods[i].dat.size() + 1;
					}
					fseek(file1,siz, 0);
					char* a= new char[4 + goods[x].group.size() + goods[x].name.size() + goods[x].dat.size() + 1];
					ll j = 0;
					a[j] = ' ';
					j++;
					for (int i = 0; i < goods[x].group.size(); i++, j++)
						a[j] = goods[x].group[i];
					a[j] = ' ';
					j++;
					
					for (int i = 0; i < goods[x].name.size(); i++, j++)
						a[j] = goods[x].name[i];
					a[j] = ' ';
					j++;

					for (int i = 0; i < goods[x].dat.size(); i++, j++)
						a[j] = goods[x].dat[i];
					a[j] = ' ';
					j++;

					if (goods[x].ready == true)
						a[j] = '1';
					else
						a[j] = '0';
					//std::cout <<(ll) j << '\n';
					//char a[] = x.group + " " + x.name + " " + x.dat + " " + (x.ready == 1 ? '1' : '0')
					//char* a = new char[goods[x].group.size()];
					//for (int i = 0; i < goods[x].group.size(); i++)
						//a[i] = goods[x][i];
					fwrite(a, 4 + goods[x].group.size() + goods[x].name.size() + goods[x].dat.size() + 1, 1, file1);
					fclose(file1);


			}
			else
					if (x == 4)///DELETE
					{
						show(goods, v_size);
						std::cout << "Write id of line for deleting: ";
						x = input(0, v_size - 1);
						pop(x,goods,v_size);
					}
					else
						if (x == 5)
							{
								
								std::string s[(ll)1e3];
								ll ind = 0;
								for (int i = 0; i < v_size; i++)
								{
									if (find(s,goods[i].group,ind+1)>ind) 
									{
										s[ind] = goods[i].group;
										ind++;
										std::cout << s[ind - 1] << ":\n";
										std::cout << "Model\t\tData\t\tReady\n";
										for (int j = i; j < v_size; j++)
										{
											if (goods[j].group == s[ind - 1])
											{
												std::cout << goods[j].name << "\t\t" << goods[j].dat << "\t\t";
												if (goods[j].ready == true)
													std::cout << "Ready\n";
												else std::cout << "Not ready" << "\n";
											}
										}
									}
								}
							}
}


int main()
{
	std::fstream file;
	file.open("data.txt", std::ios::out);
	file.close();
	ll v_size = 0;
	T* goods = nullptr;
	ll days[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
	while (true)
	{
		main_menu(file,goods,days,v_size);
	}
}
