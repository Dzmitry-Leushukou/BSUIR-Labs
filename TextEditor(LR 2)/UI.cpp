#include "UI.h"

UI::UI()
{
	users = Serializer::loadUsers("users.txt");
}

void UI::inputHandler()
{
	std::string s;
	userMenu();
	help();
	while (std::getline(std::cin, s))
	{
		process(s);
		system("cls");
	}
}

void UI::userMenu()
{
	showUsersList();
	std::cout << "Choose id of user (write -1 to create new): ";
	int id = getNumber(-1, users.size() - 1);
	if (id == -1)//create
	{
		system("cls");
		std::cout << "Write username:";
		std::string s;
		std::cin >> s;
		users.push_back(new User(s));
	}
	else
	{
		std::cout << "Choose operation with user:\n0. Choose\n1. Delete\n";
		int op = getNumber(0, 1);
		if (op == 1)
		{
			delete users[id];
			users[id] = nullptr;
			users.erase(users.begin() + id);
		}
		else
			user = users[id];
	}

	Serializer::saveUsers(users, "users.txt");
	if (!user)
		userMenu();
}

void UI::showUsersList()
{
	int numb = 0;
	for (auto& i : users)
	{
		std::cout << numb << ". " << i->getName()<<'\n';
		numb++;
	}
}

void UI::process(std::string s)
{
	try 
	{
		if (s == "user")
		{
			userMenu();
			return;
		}

		if (s=="open")
		{
			open();
			return;
		}

		if (s == "help")
		{
			help();
			return;
		}

		if (s == "create")
		{
			create();
			return;
		}

		if (s == "edit")
		{
			create();
			return;
		}
		wrong();
	}
	catch (const std::exception& e)
	{
		wrong(e.what());
	}
}

void UI::open()
{
	
}


void UI::show(char sec)
{
	std::vector<std::vector<std::pair<std::string, char>>> p = user->getPermissions();
	
	std::cout << "Allowed files:\n";

	for (char i = sec; i < p.size(); i++)
	{
		if (i == 0)
			std::cout << "\n==As spectator==\n";
		if (i == 1)
			std::cout << "\n==As editor==\n";
		if (i == 2)
			std::cout << "\n==As admin==\n";
		for (auto& j : p[i])
		{
			std::string st_type = "local";
			if (j.second == 1)
				st_type = "cloud";

			std::cout << j.first << " " << st_type << "\n";
		}
		
	}
}

void UI::wrong(std::string s)
{
	if (s != "")
		std::cout << s << '\n';
	std::cout << "Invalid command. Try again or check help (Write help)\n";
	system("pause");
	system("cls");
}

void UI::help()
{
	system("cls");
	std::cout << "==Commands==\n";
	std::cout << "\"user\" - userMenu\n";
	std::cout << "\"create\" - create file\n";
	std::cout << "\"open\" - open file\n";
	std::cout << "\"help\" - to show this menu again\n";
	system("pause");
}

void UI::simulateConsoleInput(const std::string& text) 
{	
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	if (hInput == INVALID_HANDLE_VALUE) 
	{
		std::cerr << "Error: GetStdHandle(STD_INPUT_HANDLE)" << std::endl;
		return;
	}
	std::vector<INPUT_RECORD> records;
	for (char c : text)
	{
		INPUT_RECORD rec = { 0 };
		rec.EventType = KEY_EVENT;

		rec.Event.KeyEvent.bKeyDown = TRUE;
		rec.Event.KeyEvent.wRepeatCount = 1;
		rec.Event.KeyEvent.wVirtualKeyCode = 0; // Unicode
		rec.Event.KeyEvent.uChar.UnicodeChar = c;
		records.push_back(rec);

		rec.Event.KeyEvent.bKeyDown = FALSE;
		records.push_back(rec);
	}

	DWORD written;
	if (!WriteConsoleInput(hInput, records.data(), (DWORD)records.size(), &written)) 
	{
		std::cerr << "WriteConsoleInput failed, error: " << GetLastError() << std::endl;
	}
	
}

void UI::create()
{
	short n = getStorageType();
	if (n == 0)
	{
		std::string filepath = getFilePath(false);
		std::ofstream cr(filepath);
		cr.close();
		std::cout << "File("<<filepath<<") created successfully\n";
		user->addPermission(2, filepath, 0);
		system("pause");
	}

	Serializer::saveUsers(users,"users.txt");
}

int UI::getStorageType()
{
	std::cout << "Choose storage type (0 - local, 1 - cloud):\n";
	return getNumber(0, 1);
}

int UI::getNumber(int min,int max)
{
	int n;
	while (true) 
	{
		if (std::cin >> n)
		{
			if (n < min || n>max)
			{
				std::cout << "Wrong number\n";
			}
			else return n;
		}
		else std::cout << "Wrong input\n";
	}
}

std::string UI::getFilePath(bool need)
{
	std::string filepath;
	while (true)
	{
		std::cout << "Write filepath: ";
		std::cin >> filepath;
		if (need == std::filesystem::exists(filepath))
		{
			return filepath;
		}
		if (need)
			std::cout << "File doesn`t exist\n";
		else
			std::cout << "File already exist\n";
	
	}
	
}