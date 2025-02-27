#include<iostream>
#include<string>

#include "T.h"

#define ll long long
#define ld long double


void newname(std::string s, std::string names[], ll& names_size)
{
	for (int i = 0; i < names_size; i++)
	{
		if (s == names[i])
			return;
	}
	names[names_size++] = s;
}


std::string cins()
{
	std::string ch;
	char q = 0;
	//scanf_s("%c", &q);
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


long long  cinll() {
	int ans = 0;
	char c;
	while (1)
	{
		scanf_s("%d", &ans);
		scanf_s("%c", &c);
		if (ans < 0)
			printf("Wrong answer\n");
		else if (c != '\n')
		{
			while (std::cin.get() != '\n');
			printf("Wrong answer\n");
		}
		else if (c == '\n')
			break;
	}
	return ans;

	
}


ll input(ll l, ll r)
{
	ll x;
	while (true)
	{
		x = cinll();
		//std::cout << "LL" << x << '\n';
		//scanf_s("%ld", &x);
		if (x > r || x < l)
		{
			printf("Wrong input. Try again\n");
			continue;
		}
		break;
	}
	return x;
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


void show(T* & goods, ll& v_size)
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


void pop(ll ind, T* & goods, ll& v_size)
{
	T*  newar = new T[v_size - 1];
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


void push(T x, T* & goods, ll& v_size)
{
	T*  newar = new T[v_size + 1];
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


void sort(T* & goods, ll& v_size)
{
	for (int i = 1; i < v_size; i++)
	{
		std::string now = goods[i].name;
		T temp = goods[i];
		for (int j = i - 1; j >= 0; j--)
		{
			if (now < goods[j].name)break;
			goods[j + 1] = goods[j];
			goods[j] = temp;
		}
	}
	show(goods, v_size);
}


T add(ll flag, ll& names_size, std::string names[], ll days[])
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