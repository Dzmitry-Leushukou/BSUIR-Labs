#include <iostream>
#include <string>

#include "T.h"
#include "Func.h"


#define ll long long
#define ld long double


void main_menu(T*& goods, ll& v_size, ll& names_size, std::string names[], ll days[])
{
	ll x;
	printf("What do you want to do(write number)?\n1. Add new line(s)\n2. Show list\n3. Find line\n4. Edit line\n5. Delete line\n6. Sort list by the names\n7. Goods with price > 100.341\n");
	x = input(1, 7);
	if (x == 1)
	{
		printf("Choose type of input:\n1. Add n elements\n2. Add element while not found key line\n3. Add before the stop word\n0. Exit\n");
		x = input(0, 3);

		if (x == 1)
		{
			printf("Write n: ");
			ll n = input(-1, LLONG_MAX);
			while (n--)
			{
				push(add(1, names_size, names, days), goods, v_size);
			}
		}
		else
			if (x == 2)
			{
				T fl, check;
				printf("Key line to stop input\n");
				fl = add(-1, names_size, names, days);
				printf("Write regular lines:\n");
				while (true)
				{
					check = add(1, names_size, names, days);
					if (check == fl)break;
					push(check, goods, v_size);
				}

			}
			else if (x == 3)
			{
				char a;
				while (true)
				{
					printf("Do you want stop input?(Write Y if you want to stop writing, otherwise any symbol)\n");

					a = 0;
					scanf_s("%c", &a);
					if (a == 'Y')break;
					push(add(1, names_size, names, days), goods, v_size);
				}
			}
		return;
	}
	else
		if (x == 2)//SHOW
		{
			//std::cout << goods << '\n';
			show(goods, v_size);
		}
		else
			if (x == 3)//FIND
			{
				printf("Which column is needed to search for an element?\n1. Name\n2. Amount\n3. Price\n4. Data\n5. Expiration\n");
				x = input(0, 5);
				if (x == 1)
				{
					printf("What name should find?\n");
					std::string s;
					s = cins();
					printf("Name\t\tAmount\t\tPrice(some numbers may be invisible)\t\tData\t\tExpiration\n");
					for (int i = 0; i < v_size; i++)
					{
						if (goods[i].name == s)
						{
							for (int j = 0; j < goods[i].name.size(); j++)
								printf("%c", goods[i].name[j]);
							printf("\t\t%ld", goods[i].kol);
							printf("\t\t%f", goods[i].price);
							printf("\t\t\t\t\t");
							for (int j = 0; j < goods[i].dat.size(); j++)
								printf("%c", goods[i].dat[j]);
							printf("\t\t%ld", goods[i].expiration);
							printf("\n");
						}
					}
				}
				else
					if (x == 2)
					{
						printf("What amount should find?\n");
						ll x = input(-1, LLONG_MAX);
						printf("Name\t\tAmount\t\tPrice(some numbers may be invisible)\t\tData\t\tExpiration\n");
						for (int i = 0; i < v_size; i++)
						{
							if (goods[i].kol == x)
							{
								for (int j = 0; j < goods[i].name.size(); j++)
									printf("%c", goods[i].name[j]);
								printf("\t\t%ld", goods[i].kol);
								printf("\t\t%f", goods[i].price);
								printf("\t\t\t\t\t");
								for (int j = 0; j < goods[i].dat.size(); j++)
									printf("%c", goods[i].dat[j]);
								printf("\t\t%ld", goods[i].expiration);
								printf("\n");
							}
						}
					}
					else
						if (x == 3)
						{
							printf("What price should find?\n");
							ld x = 0;
							scanf_s("%lf", &x);
							printf("Name\t\tAmount\t\tPrice(some numbers may be invisible)\t\tData\t\tExpiration\n");
							for (int i = 0; i < v_size; i++)
							{
								if (goods[i].price == x)
								{
									for (int j = 0; j < goods[i].name.size(); j++)
										printf("%c", goods[i].name[j]);
									printf("\t\t%ld", goods[i].kol);
									printf("\t\t%f", goods[i].price);
									printf("\t\t\t\t\t");
									for (int j = 0; j < goods[i].dat.size(); j++)
										printf("%c", goods[i].dat[j]);
									printf("\t\t%ld", goods[i].expiration);
									printf("\n");
								}
							}
						}
						else
							if (x == 4)
							{
								printf("What adding data should find?\n");
								std::string s;
								s = cins();
								printf("Name\t\tAmount\t\tPrice(some numbers may be invisible)\t\tData\t\tExpiration\n");
								for (int i = 0; i < v_size; i++)
								{
									if (goods[i].dat == s)
									{
										for (int j = 0; j < goods[i].name.size(); j++)
											printf("%c", goods[i].name[j]);
										printf("\t\t%ld", goods[i].kol);
										printf("\t\t%f", goods[i].price);
										printf("\t\t\t\t\t");
										for (int j = 0; j < goods[i].dat.size(); j++)
											printf("%c", goods[i].dat[j]);
										printf("\t\t%ld", goods[i].expiration);
										printf("\n");
									}
								}
							}
							else
								if (x == 5)
								{
									printf("What expiration in days should find?\n");
									ll x = input(-1, LLONG_MAX);
									printf("Name\t\tAmount\t\tPrice(some numbers may be invisible)\t\tData\t\tExpiration\n");
									for (int i = 0; i < v_size; i++)
									{
										if (goods[i].expiration == x)
										{
											for (int j = 0; j < goods[i].name.size(); j++)
												printf("%c", goods[i].name[j]);
											printf("\t\t%ld", goods[i].kol);
											printf("\t\t%f", goods[i].price);
											printf("\t\t\t\t\t");
											for (int j = 0; j < goods[i].dat.size(); j++)
												printf("%c", goods[i].dat[j]);
											printf("\t\t%ld", goods[i].expiration);
											printf("\n");
										}
									}
								}
				return;
			}
			else
				if (x == 4)//Edit
				{
					show(goods, v_size);
					printf("Write id of line for editing: ");
					x = input(0, v_size - 1);
					printf("Write new information:\n");
					goods[x] = add(0, names_size, names, days);
				}
				else
					if (x == 5)
					{
						show(goods, v_size);
						printf("Write id of line for deleting: ");
						x = input(0, v_size - 1);
						pop(x, goods, v_size);
					}
					else
						if (x == 6)//sort
						{
							sort(goods, v_size);
						}
						else
							if (x == 7)
							{
								printf("Name\t\tAmount\t\tPrice(some numbers may be invisible)\t\tData\t\tExpiration\n");
								for (int i = 0; i < v_size; i++)
								{
									if (goods[i].price > (ld)100.341)
									{
										for (int j = 0; j < goods[i].name.size(); j++)
											printf("%c", goods[i].name[j]);
										printf("\t\t%ld", goods[i].kol);
										printf("\t\t%f", goods[i].price);
										printf("\t\t\t\t\t");
										for (int j = 0; j < goods[i].dat.size(); j++)
											printf("%c", goods[i].dat[j]);
										printf("\t\t%ld", goods[i].expiration);
										printf("\n");
									}
								}
							}
}


int main()
{
	ll v_size = 0;
	T* goods = nullptr;
	std::string names[(ll)1e3];
	ll names_size = 0, days[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
	/*
	push({ "Pen",2,100.432322,"10.01.0010",1 }, goods, v_size);
	push({ "Car",2,100.432322,"10.01.0010",1 }, goods, v_size);
	push({ "Pencil",2,100.341111111111,"10.01.0010",1 }, goods, v_size);
	push({ "Hand",2,100.342,"10.01.0010",1 }, goods, v_size);
	push({ "Head",2,100.340,"10.01.0010",1 }, goods, v_size);
	*/
	while (true)
	{
		main_menu(goods,v_size, names_size,names,days);
	}
}
