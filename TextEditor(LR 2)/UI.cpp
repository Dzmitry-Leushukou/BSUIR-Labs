#include "UI.h"

void UI::inputHandler()
{
	std::string s;
	help();
	while (std::getline(std::cin, s))
	{
		process(s);
		system("cls");
	}
}

void UI::process(std::string s)
{
	try 
	{
		if (s.size() > 6 && s.substr(0, 6) == "openl ")
		{
			openl(s.substr(6));
			show();
			return;
		}

		if (s == "help")
		{
			help();
			return;
		}

		wrong();
	}
	catch (const std::exception& e)
	{
		wrong(e.what());
	}
}

void UI::openl(std::string filepath)
{
	if (std::filesystem::exists(filepath)) //check exist of file
	{		
		std::filesystem::path filePath(filepath);

		delete file;
		if (filePath.has_extension()&&filePath.extension().string()=="md")
		{
			file = new MD(filepath);
		}
		else
			file = new File(filepath);
		
		std::cout << "Loading file info...\n";
		return;		
	}
	throw std::invalid_argument("File doesn`t exist");
}

void UI::close()
{

}

void UI::show()
{
	system("cls");
	switch (mode)
	{
	case 0: // default
	{
		if (file)
		{
			
		}
		else
			showFiles();
		break;
	}
	case 1: // edit
	{
		break;
	}
	case 2: // preview
	{
		break;
	}
	case 3: // create
	{
		showcreateMenu();
		break;
	}
	}
}

void UI::showFiles()
{

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
	std::cout << "\"create\" - open create menu\n";
	std::cout << "\"openl path_to_file\" - open local file\n";
	std::cout << "\"help\" - to show this menu again\n";
	system("pause");
	system("cls");
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

void UI::showcreateMenu()
{
	std::cout<<"Choose file format:\n0. "
}