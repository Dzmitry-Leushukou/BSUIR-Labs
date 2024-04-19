#include<fstream>
#include<iostream>
#include<string>

#include "T.h"


#define ll long long
#define ld long double


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


ll input(ll l, ll r)
{
	ll x;
	while (true)
	{
		x = cinll();
		if (x > r || x < l)
		{
			std::cout << "Wrong input. Try again\n";
			continue;
		}
		break;
	}
	return x;
}


std::string cins()
{
	std::string ch;
	char a;
	a = getchar();
	if (a != 10)
		ch += a;
	while (true)
	{
		a = getchar();
		if (a == 10)
		{
			break;
		}
		ch += a;
	}
	return ch;
}


ll input_day(ll m, ll y, const long long days[])
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


std::string dat_input(const long long days[])
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


void show(std::fstream& file, const char del[30], long long& v_size)
{
	std::cout << "ID      Name\t\t\tAmount\t\tPrice\t\tData\n";


	//std::cout<<"s="<<v_size<<'\n';
	file.open("data.bin", std::ios_base::binary | std::ios_base::in);
	for (ll i = 0; i < v_size; i++)
	{
		T Koran;

		file.read((char*)&Koran, sizeof(Koran));
		//break;



		bool fl = true;
		for (int i = 0; i < 30; i++)
		{
			if (Koran.name[i] != del[i])
			{
				fl = false;
				break;
			}
		}
		if (!fl)
		{
			std::cout << i << "      ";
			for (int i = 0; i < 30; i++)
				std::cout << Koran.name[i];
			std::cout << Koran.kol.amount << "\t\t" << Koran.price << "\t\t";
			for (int i = 0; i < 30; i++)
				std::cout << Koran.dat[i];
			std::cout << '\n';
		}
	}
	file.close();

}


void show1(const char del[30], long long& v_size, T* &goods)
{
	std::cout << "ID      Name\t\tAmount\t\tPrice\t\tData\n";

	for (int i = 0; i < v_size; i++)
	{
		if (goods[i].name != del)
			std::cout << "[" << i << "]   " << goods[i].name << goods[i].kol.amount << "\t\t" << goods[i].price << "\t\t" << goods[i].dat << '\n';

	}
}


void pop(long long ind, const char del[30], long long& v_size, T* &goods)
{
	//std::ofstream fout("temp.txt");
	for (int i = 0; i < v_size; i++)
	{
		if (i == ind)
		{
			for (int q = 0; q < 30; q++)
				goods[i].name[q] = del[q];
			FILE* file2;
			fopen_s(&file2, "data.bin", "rb+");
			fseek(file2, sizeof(T) * i, 0);
			fwrite((char*)&goods[i], sizeof(T), 1, file2);
			fclose(file2);
		}

	}
}


void push(std::fstream& file, T x, T* &goods, long long& v_size)
{
	T* newar = new T[v_size + 1];
	//system("pause");
	for (int i = 0; i < v_size; i++)
	{
		for (int j = 0; j < 30; j++)
			newar[i].name[j] = goods[i].name[j], newar[i].dat[j] = goods[i].dat[j];

		newar[i].kol.amount = goods[i].kol.amount;
		newar[i].price = goods[i].price;
		
	}
	//system("pause");
	newar[v_size] = x;
	//system("pause");
	delete[] goods;
	goods = newar;
	//std::cout << goods[0].kol.amount << "!\n";
	v_size++;
	
	file.open("data.bin", std::ios::binary | std::ios::app);
	file.write((char*)&x, sizeof(T));
	file.close();
	//std::cout<< "1  " << x.name << "\t\t" << x.kol.amounT << "\t\t" << x.price << "\t\t" << x.dat<<'\n';


}


T add(long long flag, const long long days[])
{
	std::string  s;
	T res;
	///Name
	std::cout << "Write name of good:\n";
	s = cins();
	s[0] = toupper(s[0]);
	for (int i = 0; i < 30; i++)
		res.name[i] = ' ';
	for (int i = 0; i < std::min(30, (int)s.size()); i++)
		res.name[i] = s[i];
	///Amount
	std::cout << "Write amount of good:\n";
	 res.kol.amount=cinll();

	///price
	std::cout << "Write price of good:\n";
	res.price = cinld();
	///Data
	std::cout << "Write data of good`s adding:\n";
	s = dat_input(days);

	for (int i = 0; i < 30; i++)
		res.dat[i] = ' ';
	for (int i = 0; i < s.size(); i++)
		res.dat[i] = s[i];

	return res;
}


long long find(std::string s[], std::string a, long long n)
{
	for (int i = 0; i < n; i++)
	{
		if (s[i] == a)
			return i;
	}
	return n;
}

