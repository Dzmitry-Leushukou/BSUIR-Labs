#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#define ll long long
typedef union location
{
	bool allrented;
	ll number;
};
typedef class worker
{
public:
	location rented;
	ll register_number;
	ll year_publishing;
	ll pages;
	char publoffice[60];
	char name[60];
	char author[60];
};
using namespace std;
void BookInput(worker& Koran);
bool Check_Book(worker Bible, worker Koran, ll index);
void Delete_Book(fstream& file, ll otstup, ll& size);
void See_Content(fstream& file, ll& size);
ll Find_Book(fstream& file, ll& size);
void Change_Book(fstream& file, ll otstup);
void Special_Task(fstream& file, ll& size);
bool Massive_check(char a[], char b[]);
ll ll_Enter();
void BookShow(worker Koran);
void String_Enter(char str[]);
void Enter_Books_1(fstream& file, ll& size);
void File_Enter(fstream& file, worker Koran);
void ShellSort(worker* arr, ll length);
int main()
{
	setlocale(LC_ALL, "");
	ll size = 0;
	fstream file;
	file.open("File.bin", ios::out);
	bool n = 1;
	file.close();
	{
		while (n) {
			cout << ("Вебирете функцию\n");
			cout << ("1 Просмотреть бибилиотеку\n");
			cout << ("2 Добавить книгу\n");
			cout << ("3 Найти книгу\n");
			cout << ("4 Изменить информацию о книгу\n");
			cout << ("5 Удалить книгу\n");
			cout << ("6 Вывести книги после заданного года\n");
			cout << ("7 Завершение программы\n");
			while (1)
			{
				ll k = 0;
				k = ll_Enter();
				if (k == 1)
				{
					See_Content(file, size);
					break;
				}
				if (k == 2)
				{
					Enter_Books_1(file, size);
					break;
				}
				if (k == 3)
				{
					Find_Book(file, size);
					break;
				}
				if (k == 4)
				{
					ll otstup = Find_Book(file, size);
					Change_Book(file, otstup);
					break;
				}
				if (k == 5)
				{
					ll otstup = Find_Book(file, size);
					if (otstup != -1)
					{
						Delete_Book(file, otstup, size);
					}
					break;
				}
				if (k == 6)
				{
					Special_Task(file, size);
					break;
				}
				if (k == 7)
				{
					cout << ("Программа завершена\n");
					n = 0;
					break;
				}
				else
					cout << ("Неверный ввод\n");
			}
		}
	}
}
ll ll_Enter()
{
	int ans = 0;
	char c;
	while (1)
	{
		scanf("%d", &ans);
		scanf("%c", &c);
		if (ans < 0)
			cout << ("Неверный ввод\n");
		else if (c != '\n')
		{
			while (cin.get() != '\n');
			cout << ("Неверный ввод\n");
		}
		else if (c == '\n')
			break;
	}
	return ans;
}
void String_Enter(char str[])
{
	string ans;
	while (1)
	{
		ans.clear();
		getline(cin, ans);
		if (ans.length() >= 60)
			cout << "Неверный ввод\n";
		else
			break;
	}
	for (ll i = 0; i < ans.length(); i++)
		str[i] = ans[i];
	str[ans.length()] = '\0';
}
void BookInput(worker& Koran)
{
	cout << ("Введите название книги\n");
	String_Enter(Koran.name);
	cout << ("Введите автора\n");
	String_Enter(Koran.author);
	cout << ("Введите издательство\n");
	String_Enter(Koran.publoffice);
	cout << ("Введите кол-во страниц\n");
	Koran.pages = ll_Enter();
	cout << ("Введите год издания\n");
	Koran.year_publishing = ll_Enter();
	cout << ("Введите регистрационный номер\n");
	Koran.register_number = ll_Enter();
	cout << "Есть ли еще такие книги в бибилиотеке?(yes,no)\n";
	bool there;
	while (1)
	{
		string ans;
		getline(cin, ans);
		if (ans == "Yes" or ans == "yes" or ans == "YES" or ans == "yES")
		{
			there = 1;
			break;
		}
		else if (ans == "No" or ans == "no" or ans == "NO" or ans == "nO")
		{
			there = 0;
			break;
		}
		else
			cout << "Неверный ввод\n";
	}
	if (there)
	{
		cout << "Введите кол-во книг\n";
		Koran.rented.number = ll_Enter();
		if (Koran.rented.number == 0)
		{
			Koran.rented.allrented = 1;
		}
	}
	else
	{
		Koran.rented.allrented = 1;
	}
}
void Enter_Books_1(fstream& file, ll& size)
{
	ll k = 0;
	cout << ("Введите кол-во книг которое хотите добавить \n");
	k = ll_Enter();
	for (ll i = 0; i < k; i++)
	{
		worker Koran;
		BookInput(Koran);
		File_Enter(file, Koran);
	}
	size += k;
}
void BookShow(worker Koran)
{
	cout << ("Название: ");
	cout << Koran.name << '\n';
	cout << ("Автор: ");
	cout << Koran.author << '\n';
	cout << ("Издательство: ");
	cout << Koran.publoffice << '\n';
	cout << ("Кол-во страниц: ");
	cout << Koran.pages << '\n';
	cout << ("Год издания: ");
	cout << Koran.year_publishing << '\n';
	cout << ("Регистрационный номер: ");
	cout << Koran.register_number << '\n';
}

void File_Enter(fstream& file, worker Koran)
{
	file.open("File.bin", ios::binary | ios::app);
	file.write((char*)&Koran, sizeof(worker));
	file.close();
}

bool Check_Book(worker Bible, worker Koran, ll index)
{
	switch (index)
	{
	case 1:
	{
		if (Massive_check(Bible.name, Koran.name))
			return 1;
		break;
	}
	case 2:
	{
		if (Massive_check(Bible.author, Koran.author))
			return 1;
		break;
	}
	case 3:
	{
		if (Massive_check(Bible.publoffice, Koran.publoffice))
			return 1;
		break;
	}
	case 4:
	{
		if (Bible.year_publishing == Koran.year_publishing)
			return 1;
		break;
	}
	case 5:
	{
		if (Bible.register_number == Koran.register_number)
			return 1;
		break;
	}
	case 6:
	{
		if (Bible.pages == Koran.pages)
			return 1;
		break;
	}
	case 7:
	{
		if (Bible.rented.allrented == Koran.rented.allrented or Bible.rented.number == Koran.rented.number)
			return 1;
		break;
	}
	default:
		return 0;
	}
	return 0;
}
bool Massive_check(char a[], char b[])
{
	bool ans = 1;
	for (ll i = 0; i < 60; i++)
	{
		if (a[i] != b[i])
		{
			ans = 0;
			break;
		}
		if (a[i] == '\0' and b[i] == '\0')
		{
			break;
		}
	}
	return ans;
}
void ShellSort(worker* arr, ll length)
{
	ll d = length / 2;

	while (d >= 1)
	{
		for (ll i = d; i < length; i++)
		{
			ll j = i;
			while ((j >= d) and (arr[j - d].year_publishing > arr[j].year_publishing))
			{
				swap(arr[j], arr[j - d]);
				j = j - d;
			}
		}
		d /= 2;
	}

}
void See_Content(fstream& file, ll& size)
{
	if (size == 0)
	{
		cout << ("Библиотека пуста :(\n");

	}
	else {
		file.open("File.bin", ios_base::binary | ios_base::in);
		for (ll i = 0; i < size; i++)
		{
			worker Koran;
			file.read((char*)&Koran, sizeof(Koran));
			BookShow(Koran);
			cout << '\n';
		}
		file.close();
	}
}
ll Find_Book(fstream& file, ll& size)
{
	cout << "Выберите характеристику для поиска книги\n";
	cout << "1 - по названию\n";
	cout << "2 - по автору\n";
	cout << "3 - по издательству\n";
	cout << "4 - по году издания\n";
	cout << "5 - по регистационному номеру\n";
	cout << "6 - по кол-ву страниц\n";
	cout << "7 - по номеру\n";
	ll index;
	do
	{
		index = ll_Enter();
		if (index < 1 or index > 7)
			cout << "Try again\n";
	} while (index < 1 or index > 7);
	worker Bible;
	{
		Bible.name[0] = '\0';
		Bible.author[0] = '\0';
		Bible.publoffice[0] = '\0';
		Bible.pages = 0;
		Bible.year_publishing = 0;
		Bible.register_number = 0;
		Bible.rented.allrented = 1;
	}
	switch (index)
	{
	case(1):
	{
		cout << ("Введите название книги\n");
		String_Enter(Bible.name);
		break;
	}
	case(2):
	{
		cout << ("Введите автора\n");
		String_Enter(Bible.author);
		break;
	}
	case (3):
	{
		cout << ("Введите издательство\n");
		String_Enter(Bible.publoffice);
		break;
	}
	case(4):
	{
		cout << ("Введите год издания\n");
		Bible.year_publishing = ll_Enter();
		break;
	}
	case(5):
	{
		cout << ("Введите регистрационный номер\n");
		Bible.register_number = ll_Enter();
		break;
	}
	case(6):
	{
		cout << ("Введите кол-во страниц\n");
		Bible.pages = ll_Enter();
		break;
	}
	case(7):
	{
		cout << "Эта книга есть в библиотеке?\n";
		bool there;
		while (1)
		{
			string ans;
			getline(cin, ans);
			if (ans == "Yes" or ans == "yes" or ans == "YES")
			{
				there = 1;
				break;
			}
			else if (ans == "No" or ans == "no" or ans == "NO")
			{
				there = 0;
				break;
			}
			else
				cout << "Неверный ввод\n";
		}
		if (!there)
			Bible.rented.allrented = 1;
		else
		{
			cout << "Кол-во таких книг\n";
			Bible.rented.number = ll_Enter();
		}
		break;
	}
	}
	bool ans = 0;
	file.open("File.bin", ios_base::binary | ios_base::in);
	worker HarryPotter;
	ll ret = 0;
	for (ll i = 0; i < size; i++)
	{
		file.read((char*)&HarryPotter, sizeof(HarryPotter));
		ans = Check_Book(HarryPotter, Bible, index);
		if (ans)
			break;
		ret++;
	}
	file.close();
	if (ans == 0)
	{
		cout << "Книга не найдена\n";
		return -1;
	}
	else if (ans)
	{
		BookShow(HarryPotter);
		cout << "\n";
		return ret;
	}
}
void Change_Book(fstream& file, ll otstup)
{
	cout << "Выберите что хотите изменить\n";
	cout << "1 - Изменить название\n";
	cout << "2 - Изменить автора\n";
	cout << "3 - Изменить издательство\n";
	cout << "4 - Изменить год издания\n";
	cout << "5 - Изменить регистрационный номер\n";
	cout << "6 - Изменить кол-во страниц\n";
	cout << "7 - Изменить номер\n";
	ll index;
	do
	{
		index = ll_Enter();
		if (index < 1 or index > 7)
			cout << "Try again\n";
	} while (index < 1 or index > 7);
	worker Koran;
	file.open("File.bin", ios_base::binary | ios_base::in);
	file.seekg(otstup * sizeof(worker), ios::beg);
	file.read((char*)&Koran, sizeof(Koran));
	file.close();

	if (index == 1)
	{
		char a[60];
		cout << ("Введите название книги\n");
		String_Enter(Koran.name);
	}
	if (index == 2)
	{
		cout << ("Введите автора\n");
		String_Enter(Koran.author);
	}
	if (index == 3)
	{
		cout << ("Введите издательство\n");
		String_Enter(Koran.publoffice);
	}
	if (index == 4)
	{
		ll a;
		cout << ("Введите год издания\n");
		Koran.year_publishing = ll_Enter();
	}
	if (index == 5)
	{
		ll a;
		cout << ("Введите регистрационный номер\n");
		Koran.register_number = ll_Enter();
	}
	if (index == 6)
	{
		ll a;
		cout << ("Введите кол-во страниц\n");
		Koran.pages = ll_Enter();
	}
	if (index == 7)
	{
		location a;
		cout << "Такие книги еще есть в бибилиотеке?(yes,no)\n";
		bool there;
		while (1)
		{
			string ans;
			getline(cin, ans);
			if (ans == "Yes" or ans == "yes" or ans == "YES")
			{
				there = 1;
				break;
			}
			else if (ans == "No" or ans == "no" or ans == "NO")
			{
				there = 0;
				break;
			}
			else
				cout << "Неверный ввод\n";
		}
		if (there)
		{
			cout << "Введите кол-во книг\n";
			Koran.rented.number = ll_Enter();
		}
		else
			Koran.rented.allrented = 1;
	}
	FILE* file2;
	fopen_s(&file2, "File.bin", "rb+");
	fseek(file2, sizeof(worker) * otstup, 0);
	fwrite((char*)&Koran, sizeof(worker), 1, file2);
	fclose(file2);
}
void Delete_Book(fstream& file, ll otstup, ll& size)
{
	FILE* file2;
	fopen_s(&file2, "File.bin", "rb+");
	worker Buffer;
	for (ll i = otstup; i < size - 1; i++)
	{
		fseek(file2, (i + 1) * sizeof(worker), 0);
		fread(&Buffer, sizeof(worker), 1, file2);
		fseek(file2, i * sizeof(worker), 0);
		fwrite(&Buffer, sizeof(worker), 1, file2);
	}
	size--;
	fclose(file2);
}
void Special_Task(fstream& file, ll& size)
{
	file.open("File.bin", ios::binary | ios::in);
	worker* arr = new worker[size];
	for (ll i = 0; i < size; i++)
	{
		file.read((char*)&arr[i], sizeof(worker));
	}
	file.close();

	ShellSort(arr, size);
	ll year;
	cout << "Введите год\n";
	year = ll_Enter();
	ll kostil = 0;
	cout << '\n';
	for (ll i = 0; i < size; i++)
	{
		if (arr[i].year_publishing > year)
		{
			kostil++;
			BookShow(arr[i]);
			cout << '\n';
		}
	}
	if (kostil == 0)
		cout << "Таких книг нет\n";
}
