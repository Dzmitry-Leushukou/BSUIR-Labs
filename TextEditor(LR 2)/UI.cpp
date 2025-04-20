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
			user = users[id],styler.setUserPresets(user->getStyles());
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

		if (s == "delete")
		{
			deleteFile();
			return;
		}
		wrong();
	}
	catch (const std::exception& e)
	{
		wrong(e.what());
	}
}

void UI::deleteFile()
{
	show(2);
	std::cout << "Choose id of file or write -1 to go back\n";
	int id = getNumber(-1, files.size() - 1);
	if (id == -1)
		return;

	std::pair<std::string, char>del = user->getFile(id);

	if (del.second == 0)
	{
		const char * path = del.first.c_str();

		if (remove(path) != 0)
		{
			std::cout << "Can`t remove this file\n";
			return;
		}
		for (auto& i : users)
		{
			i->deleteFile(del);
		}
		Serializer::saveUsers(users, "users.txt");
	}
	else
	{
		std::cout << "Can`t delete file on cloud\n";
		system("pause");
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
	styler.setUserPresets(user->getStyles());
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
	styler.setUserPresets(user->getStyles());
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
	std::pair<std::string, char>del = user->getFile(id);
	if (del.second == 1)
	{
		std::cout << "Can`t open file on cloud\n";
		return;
	}
	file = new File(files[id]);

	system("cls");
	std::cout << History::getNotify(file->getPath());
	system("pause");

	act->clear(file->getRaw());
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
					 "5. Find\n6. Get permission\n7. Redo\n8. Undo\n-1. Exit\n";
		id = getNumber(-1, 8);

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

		if (id == 1)
		{
			preview();
			continue;
		}

		if (id == 2)
		{
			if (user->getPermission(file->getPath()) < 1)
			{
				std::cout << "You do not have permission to save this file.\n";
				system("pause");
				continue;
			}
			std::cout << "Choose storage type:\n0. Local\n1. Cloud\nType: ";
			int type = getNumber(0,1);
			if (type == 0)
				act->add(file->getRaw()), file->save();
			else
			{
				CloudService::save(file->getPath());
				std::string filename = file->getPath();
				int i = filename.size() - 1;
				std::string tmp;
				while (filename[i] != '/' && i >= 0)
				{
					tmp += filename[i];
					i--;
				}
				std::reverse(tmp.begin(), tmp.end());
				filename = tmp;
				user->addPermission(2,filename , 1);
				Serializer::saveUsers(users, "users.txt");
			}
			History::savedFile(user->getName(), file->getPath());
			continue;
		}
		if (id == 3)
		{
			if (user->getPermission(file->getPath()) < 2)
			{
				std::cout << "You do not have permission to save as this file.\n";
				system("pause");
				continue;
			}
			std::cout << "Choose save format:\n0. txt\n1. md\n2. json\n3. xml\n-1. cancel\n";
			int n = getNumber(-1, 3);
			std::string filepath="";
			switch (n)
			{
			case -1:
				break;
			case 0:
				file->saveAs(txt_saver,"txt");
				filepath = file->replaceExtension("txt");
				break;
			case 1:
				file->saveAs(md_saver,"md");
				filepath = file->replaceExtension("md");
				break;
			case 2:
				file->saveAs(json_saver, "json");
				filepath = file->replaceExtension("json");
				break;
			case 3:
				file->saveAs(xml_saver, "xml");
				filepath = file->replaceExtension("xml");
				break;
			}
			if(filepath!="")
				user->addPermission(2, filepath, user->getStorage(file->getPath())), Serializer::saveUsers(users, "users.txt");
			continue;
		}
		if (id == 4) 
		{
			if (user->getPermission(file->getPath()) < 1)
			{
				std::cout << "You do not have permission to cut text in this file.\n";
				system("pause");
				continue;
			}
			cutMenu();
			continue;
		}
		if (id == 5)//find
		{
			std::cout << "Write what you wanna find (find operation work with raw text (\\n = *\\\\n*): ";
			std::string s;
			std::cin >> s;
			find(s);
			continue;
		}
		if (id == 6)
		{
			if (user->getPermission(file->getPath()) != 2)
			{
				std::cout << "You do not have permission to get permission.\n";
				system("pause");
				continue;
			}
			system("cls");
			int numb = 0;
			for (auto& i : users)
			{
				std::cout << numb << ". " << i->getName()<<'\n';
				numb++;
			}
			std::cout << "Choose user: ";
			int ind = getNumber(0, users.size() - 1);
			std::cout << "2. Admin\n";
			std::cout << "1. Editor\n";
			std::cout << "0. Viewer\n";
			std::cout << "-1. Don`t see file (NO PERMISSIONS)\n";
			std::cout << "Choose level permission: ";
			int perm = getNumber(-1, 2);
			users[ind]->addPermission(perm, file->getPath(), 0);
			Serializer::saveUsers(users, "users.txt");
		}
		if (id == 7)
		{
			try {
				act->redo();
				file->update(act->get());
			}
			catch (...)
			{
				std::cout << "Nothing to redo\n";
				system("pause");
			}
		}
		if (id == 8)
		{
			try {
				act->undo();
				file->update(act->get());
			}
			catch (...)
			{
				std::cout << "Nothing to undo\n";
				system("pause");
			}
		}
	}
}

void UI::preview()
{
	std::vector<std::string>text = file->to_string();

	system("cls");
	std::cout << "===Preview===\n";

	int bold = 0;
	int underline = 0;
	int italic = 0;

	std::string tmp;
	styler.setTextStyle();
	for (auto& s : text)
	{
		tmp = "";
		int h = 0;
		//check header
		int ind = 0;
		while (ind < s.size() && s[ind] == '#')
			ind++;
		if (ind == s.size())
		{
			std::cout << '\n';
			continue;
		}
		if (ind >= 1 && ind <= 6 && s[ind] == ' ')
			ind++, h = ind - 1;
		else
			ind = 0;

		while (ind < s.size())
		{
			styler.setTextStyle(bold, underline, italic, h);
			if (tmp.size() == 9)
				std::cout<<tmp.front(),tmp = tmp.substr(1);

			tmp += s[ind];
			ind++;

			if (tmp == "</strong>")
			{
				bold--;
				tmp = "";
				styler.setTextStyle(bold, underline, italic,h);
			}
			if (tmp.find("<strong>")<tmp.size())
			{
				bold++;
				std::cout << tmp.substr(0, tmp.find("<strong>"));
				styler.setTextStyle(bold, underline, italic,h);
				tmp = "";
			}
			if (tmp.find("<u>") < tmp.size())
			{
				underline++;
				std::cout << tmp.substr(0, tmp.find("<u>"));
				styler.setTextStyle(bold, underline, italic,h);
				tmp = "";
			}
			if (tmp.find("</u>")<tmp.size())
			{
				underline--;
				std::cout << tmp.substr(0, tmp.find("</u>"));
				styler.setTextStyle(bold, underline, italic, h);
				tmp = "";
			}
			if (tmp.find("<em>") < tmp.size())
			{
				italic++;
				std::cout << tmp.substr(0, tmp.find("<em>"));
				styler.setTextStyle(bold, underline, italic, h);
				tmp = "";
			}
			if (tmp.find("</em>") < tmp.size())
			{
				italic--;
				std::cout << tmp.substr(0, tmp.find("</em>"));
				styler.setTextStyle(bold, underline, italic,h);
				tmp = "";
			}
		}
		std::cout << tmp;
		std::cout << '\n';
	}
	system("pause");
}

void UI::find(std::string s)
{
	std::string text = file->getRaw();
	if (s.size() > text.size())
		return;
	int l = 0,r = -1;
	
	std::deque<char> target, now;
	
	for (auto& i : s)
		target.push_back(i);	
	
	system("cls");
	while (r < (int)text.size())
	{
		if (now == target)
		{
			styler.setTextStyle("found");
			while (l <= r)
			{
				std::cout << text[l];
				l++;
			}
			styler.setTextStyle();
		}
		if (l > r)
		{
			r = l;
			now.clear();
			for (; r-l < s.size(); r++)
				now.push_back(text[r]);
			r--;
			continue;
		}
		std::cout << text[l];
		l++;
		now.pop_front();
		r++;
		if (r < text.size())
			now.push_back(text[r]);
	}
	while(l<text.size())
	{
		std::cout << text[l];
		l++;
	}

	std::cout << "\n";
	system("pause");
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
	std::string s = file->getRaw();
	
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
	std::cout << "\"delete\" - to delete file\n";
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
	std::string filepath = getFilePath(false);
	std::ofstream cr(filepath);
	cr.close();
	std::cout << "File("<<filepath<<") created successfully\n";
	user->addPermission(2, filepath, 0);
	system("pause");
	std::cin.ignore();
	History::createdFile(user->getName(), filepath);
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