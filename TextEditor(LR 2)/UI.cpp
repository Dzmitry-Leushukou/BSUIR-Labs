#include "UI.h"

UI::UI()
{
	users = Serializer::loadUsers("users.txt");
}

void UI::inputHandler()
{
	std::string s;
	help();
	while (true)
	{
		system("cls");
		if (!user)
		{
			userMenu();
			continue;
		}
		show(0);
		std::getline(std::cin, s);
		process(s);
	}
}

void UI::userMenu()
{
	system("cls");
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
		system("cls");
	}
	else
	{
		std::cout << "Choose operation with user:\n0. Choose\n1. Delete\n";
		int op = getNumber(0, 1);
		if (op == 1)
		{
			if (user == users[id])
				user = nullptr;
			delete users[id];
			users[id] = nullptr;
			users.erase(users.begin() + id);
		}
		else
			user = users[id],user->setStyles();
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

		if (s == "style")
		{
			style();
			return;
		}
		wrong();
	}
	catch (const std::exception& e)
	{
		wrong(e.what());
	}
}

void UI::style()
{
	system("cls");
	std::cout << "Choose what you wanna change:\n0. Color\n1. Font\n-1. Exit\n";
	int n = getNumber(-1, 1);
	if (n == -1)
		return;
	if (n == 0)
	{
		changeColor();
		return;
	}
	changeFont();
}

void UI::changeFont()
{
	std::cout << "Write wanted size (1..72): ";
	int size = getNumber(1, 72);

	user->addStyle(std::to_string(size));
	Serializer::saveUsers(users, "users.txt");
}

void UI::changeColor()
{
	std::cout << "Choose color of background:" <<
		"\n0. Black" <<
		"\n1. Dark blue" <<
		"\n2. Green" <<
		"\n3. Blue" <<
		"\n4. Red" <<
		"\n5. Violet" <<
		"\n6. Yellow" <<
		"\n7. White" <<
		"\n8. Grey" <<
		"\n9. Light dark blue" <<
		"\n10. Light green" <<
		"\n11. Light blue" <<
		"\n12. Light red" <<
		"\n13. Light violet" <<
		"\n14. Light yellow" <<
		"\n15. Bright white" << "\n";
	int f = getNumber(0,15);

	std::cout << "Choose color of text:" <<
		"\n0. Black" <<
		"\n1. Dark blue" <<
		"\n2. Green" <<
		"\n3. Blue" <<
		"\n4. Red" <<
		"\n5. Violet" <<
		"\n6. Yellow" <<
		"\n7. White" <<
		"\n8. Grey" <<
		"\n9. Light dark blue" <<
		"\n10. Light green" <<
		"\n11. Light blue" <<
		"\n12. Light red" <<
		"\n13. Light violet" <<
		"\n14. Light yellow" <<
		"\n15. Bright white" << "\n";
	int s = getNumber(0, 15);
	
	std::string cmd = "color ";
	char a='0'+f, b='0'+s;
	if (f >= 10)
	{
		a = 'A' + (f - 10);
	}
	if (s >= 10)
	{
		b = 'A' + (f - 10);
	}
	cmd = cmd + a + b;
	user->addStyle(cmd);
	Serializer::saveUsers(users, "users.txt");
}

void UI::open()
{
	system("cls");
	show(0);
	std::cout << "Choose id of file or write -1 to go back\n";
	int id = getNumber(-1, files.size() - 1);
	if (id == -1)
		return;
	
	file = new File(files[id]);

	fileMenu();
}

void UI::fileMenu()
{
	int id = 0;
	while (id != -1)
	{
		system("cls");
		std::vector<std::string> text = file->to_string();

		for (auto& i : text)
			std::cout << i << "\n";

		std::cout << "====================================\nChoose type of operations:\n0. Edit\n1. Preview (good for markdown)\n2. Save\n3. Save as\n4. Cut\n"<<
					 "5. Find\n-1. Exit\n";
		id = getNumber(-1, 5);

		if (id == 0)
		{
			if (user->getPermission(file->getPath()) >= 1)
				editMenu();
			else
			{
				std::cout << "You do not have permission to edit this file.\n";
				system("pause");
			}
			continue;
		}
		if (id == 2)
		{
			if (user->getPermission(file->getPath()) < 1)
			{
				std::cout << "You do not have permission to edit this file.\n";
				system("pause");
				continue;
			}
			file->save();
			continue;
		}
		if (id == 3)
		{
			if (user->getPermission(file->getPath()) < 1)
			{
				std::cout << "You do not have permission to edit this file.\n";
				system("pause");
				continue;
			}
			std::cout << "Choose save format:\n0. txt\n1. md\n2. json\n3. xml\n-1. cancel\n";
			int n = getNumber(-1, 3);
			switch (n)
			{
			case -1:
				break;
			case 0:
				file->saveAs(txt_saver,"txt");
				break;
			case 1:
				file->saveAs(md_saver,"md");
				break;
			case 2:
				file->saveAs(json_saver, "json");
				break;
			case 3:
				file->saveAs(xml_saver, "xml");
				break;
			}

			continue;
		}

		if (id == 4) 
		{
			if (user->getPermission(file->getPath()) < 1)
			{
				std::cout << "You do not have permission to edit this file.\n";
				system("pause");
				continue;
			}
			cutMenu();
			continue;
		}
	}
}

void UI::cutMenu()
{
	std::vector<std::string>v = file->to_string();
	for (int i = 0; i < v.size(); i++)
	{
		std::cout << "[" << i << "]" << v[i] << '\n';
	}
	std::cout << "From line number(write -1 to exit): ";
	int ll = getNumber(-1, v.size() - 1);
	if (ll == -1)
		return;
	std::cout << "From number of character: ";
	int lc = getNumber(0, v[ll].size() - 1);
	std::cout << "To line number(line number >= from): ";
	int rl = getNumber(ll, v.size() - 1);
	int rc;
	if (ll == rl)
	{
		std::cout << "To number of character(number >= from): ";
		rc = getNumber(lc, v[rl].size() - 1);
	}
	else 
	{
		std::cout << "To number of character: ";
		rc = getNumber(0, v[rl].size() - 1);
	}

	file->cut(ll, lc, rl, rc);

}

void UI::editMenu()
{
	std::vector<std::string> text = file->to_string();
	std::string s;
	for (auto& i : text)
		s += i + "*\\n*";
	
	std::cout << "For set \'\\n\' write \"*\\n*\"\n";
	simulateConsoleInput(s);
	std::string new_text;
	std::getline(std::cin, new_text);
	file->update(new_text);
}

void UI::show(char sec)
{
	std::vector<std::vector<std::pair<std::string, char>>> p = user->getPermissions();
	
	files.clear();

	std::cout << "Allowed files:";
	int id = 0;
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

			std::cout << id << ": " << j.first << " " << st_type << "\n";
			files.push_back(j.first);
			id++;
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
	std::cout << "==Commands==\n";
	std::cout << "\"user\" - userMenu\n";
	std::cout << "\"create\" - create file\n";
	std::cout << "\"open\" - id of file to open it (to edit you also need open the file)\n";
	std::cout << "\"style\" - to set color or font\n";
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
		std::cin.ignore();
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
		std::string s;
		std::getline(std::cin, s);
		try
		{
			n = std::stoi(s);
			if (n < min || n>max)
			{
				std::cout << "Wrong number\n";
			}
			else return n;
		}
		catch (...)
		{
			std::cout << "Wrong input\n";
		}
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