#include <iostream>
#include <string>

#define ll long long
#define ld long double

struct t
{
	std::string name;
	ll kol = 0;
	ld price = 0;
	std::string dat;
	ll expiration = 0;


	bool operator == (t& b)
	{
		return (name == b.name && kol == b.kol && price == b.price && dat == b.dat && expiration == b.expiration);
	}
};


void newname(std::string s, std::string names[], ll& names_size)
{
	for (int i = 0; i < names_size; i++)
	{
		if (s == names[i])
			return;
	}
	names[names_size++] = s;
}

ll input(ll l, ll r)
{
	ll x;
	while (true)
	{
		x = 0;
		scanf_s("%ld", &x);
		if (x > r || x < l)
		{
			printf("Wrong input. Try again\n");
			continue;
		}
		break;
	}
	return x;
}


std::string cins()
{
	std::string ch;
	char q = 0;
	scanf_s("%c", &q);
	q = 0;
	while (true)
	{
		q = 0;
		scanf_s("%c", &q);
		if (q != 10)
			ch += q;
		else
			break;
	}
	return ch;
}


ll input_day(ll m, ll y, ll days[])
{
	ll x;
	while (true)
	{
		x = input(1, 31);
		if (x <= days[m] || (((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) && (m == 2) && x <= days[m] + 1))
		{
			return x;
		}
		printf("Wrong input. Try again\n");
	}
}


std::string dat_input(ll days[])
{
	ll y, m, d;
	printf("Year of adding:\n");
	y = input(0, LLONG_MAX);
	printf("Month of adding:\n");
	m = input(1, 12);
	printf("Day of adding:\n");
	d = input_day(m, y, days);
	std::string s;
	int i = 0;
	if (d < 10)
		s = s + "0", s = s + (char)(d + '0');
	else
		s = s + (char)(d / 10 + '0'), s = s + (char)(d % 10 + '0');
	s = s + ".";

	if (m < 10)
		s = s + "0", s = s + (char)(m + '0');
	else
		s = s + (char)(m / 10 + '0'), s = s + (char)(m % 10 + '0');
	s = s + ".";
	if (y < 1000)
		s = s + "0";
	if (y < 100)
		s = s + "0";
	if (y < 10)
		s = s + "0";
	s = s + std::to_string(y);
	return s;
}


void show(t*& goods, ll& v_size)
{
	printf("ID      Name\t\tAmount\t\tPrice(some numbers may be invisible)\t\tData\t\tExpiration\n");
	//std::cout << goods[0].name << '\n';
	for (int i = 0; i < v_size; i++)
	{
		printf("[%ld", i);
		printf("]     ");
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


void pop(ll ind, t*& goods, ll& v_size)
{
	t* newar = new t[v_size - 1];
	ll j = 0;
	for (int i = 0; i < v_size; i++)
	{
		if (i != ind)
		{
			newar[j] = goods[i];
			j++;
		}
	}
	delete[] goods;
	goods = newar;
	v_size--;
}


void push(t x, t*& goods, ll& v_size)
{
	t* newar = new t[v_size + 1];
	for (int i = 0; i < v_size; i++)
		newar[i] = goods[i];
	newar[v_size] = x;
	delete[] goods;
	goods = newar;
	v_size++;
	//std::cout << goods << '\n';
	//delete[] newar;
	//newar = nullptr;
	//exit(0);
}


void sort(t*& goods, ll& v_size)
{
	for (int i = 1; i < v_size; i++)
	{
		std::string now = goods[i].name;
		t temp = goods[i];
		for (int j = i - 1; j >= 0; j--)
		{
			if (now < goods[j].name)break;
			goods[j + 1] = goods[j];
			goods[j] = temp;
		}
	}
	show(goods, v_size);
}


t add(ll flag, ll& names_size, std::string names[], ll days[])
{
	std::string name, dat;
	ll kol, expiration = 0;
	ld price = 0;
	///NAME
	printf("Write name of good or select from the list below(write:/s id).\n");
	printf("ID\tNames\n");
	for (int i = 0; i < names_size; i++)
	{
		printf("%i", i);
		printf("\t");
		for (int j = 0; j < names[i].size(); j++)
			printf("%c", names[i][j]);
		printf("\n");
	}
	printf("\nName: ");
	std::string s;
	while (true)
	{
		s = cins();
		s[0] = toupper(s[0]);
		if (s.size() > 2 && s[0] == '/' && s[1] == 's')
		{
			std::string ch = s.substr(3);
			if (s.size() - 3 > 5)
			{
				printf("Wrong input\n");
				continue;
			}
			int id = stoi(ch);
			if (id >= 100000 || id < 0)
			{
				printf("Wrong input\n");
				continue;
			}
			name = names[id];
		}
		else name = s, newname(s, names, names_size);
		break;
	}
	///AMOUNT
	printf("Write amount of good:\n");
	kol = input(-1, LLONG_MAX);
	///PRICE
	printf("Write price of good:\n");
	scanf_s("%lf", &price);
	///data
	printf("Write dat of good`s adding:\n");
	dat = dat_input(days);
	///Expiration
	printf("Write expiration of good(in Days):\n");
	expiration = input(-1, LLONG_MAX);
	return { name,kol,price,dat,expiration };
}


void main_menu(t*& goods, ll& v_size, ll& names_size, std::string names[], ll days[])
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
				t fl, check;
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
	t* goods = nullptr;

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
		main_menu(goods, v_size, names_size, names, days);
	}
}
