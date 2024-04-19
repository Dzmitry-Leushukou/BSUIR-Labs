#include <iostream>
#include <string>
#include <fstream>

#define ll long long
#define ld long double



const char del[120] = "deleteddeleteddeleteddeleteddeleteddeleteddeleteddeleteddeleteddeleteddeleteddeleteddeleted090e902e9u012hi21shiohdasd";
union q {
	ll amount;
};
class t {
public:
	char name[120];
	q kol;
	ld price;
	char dat[100];



};
ll v_size = 0;
t* goods = nullptr;
ll days[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };

ll input(ll l, ll r)
{
	ll x;
	while (true)
	{
		std::cin >> x;
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


ll input_day(ll m, ll y)
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


std::string dat_input()
{
	ll y, m, d;
	std::cout << "Year of adding:\n";
	y = input(0, LLONG_MAX);
	std::cout << "Month of adding:\n";
	m = input(1, 12);
	std::cout << "Day of adding:\n";
	d = input_day(m, y);
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


void show(std::fstream& file)
{
	std::cout << "ID      Name\t\t\tAmount\t\t\tPrice\t\t\tData\n";


	//std::cout<<"s="<<v_size<<'\n';
	for (ll i = 0; i < v_size; i++)
	{
		t Koran;
		file.open("data.bin", std::ios_base::binary | std::ios_base::in);
		file.read((char*)&Koran, sizeof(Koran));
		//break;
		file.close();

		if (Koran.name != del)
			std::cout << i << "      " << Koran.name << "\t\t\t" << Koran.kol.amount << "\t\t\t" << Koran.price << "\t\t\t" << Koran.dat << '\n';
		//exit(0);
		return;
		if (i + 1 == v_size)return;
		else
			std::cout << "dsds";
		exit(0);
	}

}


void show1()
{
	std::cout << "ID      Name\t\t\tAmount\t\t\tPrice\t\t\tData\n";

	for (int i = 0; i < v_size; i++)
	{
		if (goods[i].name != del)
			std::cout << "[" << i << "]   " << goods[i].name << "\t\t\t" << goods[i].kol.amount << "\t\t\t" << goods[i].price << "\t\t\t" << goods[i].dat << '\n';

	}
}

void pop(ll ind)
{
	t* newar = new t[v_size - 1];
	ll j = 0;
	//std::ofstream fout("temp.txt");
	for (int i = 0; i < v_size; i++)
	{
		if (i == ind)
		{
			for(int q=0;q< 120;q++)
			newar[j].name[q] = del[q];
			//fout << newar[j].group << "|" << newar[j].name << "|" << newar[j].dat << "|" << newar[j].ready << '\n';
//			fopen_s(&file1, "out.bin", "rb+");
//			fseek(file1, sizeof(t)*i, 0);
//			fwrite(&newar[j], sizeof(t), 1, file1);
//			(file1);
		}
		else j++;
	}
}


void push(std::fstream& file, t x)
{
	t* newar = new t[v_size + 1];
	for (int i = 0; i < v_size; i++)
		newar[i] = goods[i];
	newar[v_size] = x;
	delete[] goods;
	goods = newar;
	v_size++;
	system("pause");
	file.open("data.bin", std::ios::binary | std::ios::app);
	file.write((char*)&x, sizeof(t));
	file.close();
	//std::cout << "1  " << x.name << "\t\t\t" << x.kol.amount << "\t\t\t" << x.price << "\t\t\t" << x.dat<<'\n';


}


t add(ll flag)
{
	std::string  s;
	t res;
	///Name
	std::cout << "Write name of good:\n";
	s = cins();
	s[0] = toupper(s[0]);
	for (int i = 0; i < s.size(); i++)
		res.name[i] = s[i];
	///Amount
	std::cout << "Write amount of good:\n";
	std::cin >> res.kol.amount;
	///price
	std::cout << "Write price of good:\n";
	std::cin >> res.price;
	///Data
	std::cout << "Write data of good`s adding:\n";
	s=dat_input();
	for (int i = 0; i < s.size(); i++)
	res.dat[i] = s[i];
	return res;
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
void main_menu(std::fstream& file)
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
				push(file, add(1));
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
					push(file, add(1));
				}
			}
		return;
	}
	else
		if (x == 2)//SHOW
		{
			show(file);
		}
		else
			if (x == 3)//EDIT
			{
				show(file);
				std::cout << "Write id of line for editing: ";
				x = input(0, v_size - 1);
				std::cout << "Write new information:\n";
				goods[x] = add(0);
				FILE* file2;
                fopen_s(&file2, "data.bin", "rb+");
                fseek(file2, sizeof(t) * x, 0);
                fwrite((char*)&goods[x], sizeof(t), 1, file2);
                fclose(file2);

			}
			else
				if (x == 4)///DELETE
				{
					show(file);
					std::cout << "Write id of line for deleting: ";
					x = input(0, v_size - 1);
					pop(x);
				}
				else
					if (x == 5)
					{

						std::string s[(ll)1e3];
						ll ind = 0;

					}
}


int main()
{
	//Init
	std::fstream file;
	file.open("data.bin", std::ios::out);
	file.close();
	push(file, { "D",2323,2.11,"q" });

//	show(file);
//	show1();
	push(file, { "Dima",1,1.11,"dsda" });
	show(file);
//	show1();
	//std::cout<<"\n"<<sizeof(goods[0])<<"  = "<<sizeof(goods[1])<<std::endl;

	while (true)
	{
		main_menu(file);
	}
	delete[] goods;
	goods = nullptr;
}
