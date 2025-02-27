#include<fstream>
#include<iostream>
#include<string>

#include "T.h"


#define ll long long
#define ld long double


std::string cins()
{
	std::string ch;
	char q = 0;
	scanf_s("%c", &q);
	if (q != 10)
		ch += q;
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


ll input_day(long long m, long long y, long long days[])
{
	ll x;
	while (true)
	{
		x = input(1, 31);
		if (x <= days[m] || (((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) && (m == 2) && x <= days[m] + 1))
		{
			return x;
		}
		std::cout << "Wrong input\n";
	}
}


std::string dat_input(long long days[])
{
	ll y, m, d;
	std::cout << "Year of adding:\n";
	y = input(0, LLONG_MAX);
	std::cout << "Month of adding:\n";
	m = input(1, 12);
	std::cout << "Day of adding:\n";
	d = input_day(m, y,days);
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


void show(T*& goods, long long& v_size)
{
	std::cout << "ID      Group\t\t\tModel\t\t\tData\t\t\tReady\n";
	for (int i = 0; i < v_size; i++)
	{
		std::cout << "[" << i << "]   " << goods[i].group << "\t\t\t" << goods[i].name << "\t\t\t" << goods[i].dat << "\t\t";
		if (goods[i].ready == true)
			std::cout << "Ready\n";
		else std::cout << "Not ready" << "\n";
	}
}


void pop(long long ind, T*& goods, long long& v_size)
{
	T* newar = new T[v_size - 1];
	ll j = 0;
	std::ofstream fout("data.txt");
	for (int i = 0; i < v_size; i++)
	{
		if (i != ind)
		{
			newar[j] = goods[i];
			fout << " " << goods[i].group << " " << goods[i].name << " " << goods[i].dat << " " << goods[i].ready;
			j++;
		}
	}
	delete[] goods;
	goods = newar;
	v_size--;

}


void push(std::fstream& file, T x, T*& goods, long long& v_size)
{
	T* newar = new T[v_size + 1];
	for (int i = 0; i < v_size; i++)
		newar[i] = goods[i];
	newar[v_size] = x;
	delete[] goods;
	goods = newar;
	v_size++;
	std::fstream fs("data.txt", std::ios::app);
		fs <<" "<< x.group << " " << x.name << " " << x.dat << " " << x.ready;


	//delete[] newar;
	//newar = nullptr;
	//exit(0);
}


T add(long long flag, long long days[])
{
	std::string group, name, dat;
	ll kol, expiration;
	ld price;
	///NAME
	std::cout << "Write group of good:\n";
	std::string s;
	s = cins();
	s[0] = toupper(s[0]);
	group = s;
	///Name
	std::cout << "Write name of good:\n";
	s = cins();
	s[0] = toupper(s[0]);
	name = s;
	///Data
	std::cout << "Write data of good`s adding:\n";
	dat = dat_input(days);
	///Expiration
	std::cout << "Good is ready(Y/N Y if yes, N if no):\n";
	bool ready;
	while (true)
	{
		char a;
		std::cin >> a;
		if (a == 'Y' || a == 'N')
		{
			if (a == 'Y')
				ready = true;
			else
				ready = false;
			break;
		}
		std::cout << "Wrong input.\n";
	}
	return { group,name,dat,ready };
}


ll find(std::string s[], std::string a, ll n)
{
	for (int i = 0; i < n; i++)
	{
		if (s[i] == a)
			return i;
	}
	return n;
}