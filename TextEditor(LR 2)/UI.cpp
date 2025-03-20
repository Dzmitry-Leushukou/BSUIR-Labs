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
			open(s.substr(6));
			return;
		}

		wrong();
	}
	catch (const std::exception& e)
	{
		wrong(e.what());
	}
}

void UI::open(std::string filepath)
{
	if (std::filesystem::exists(filepath)) //check exist of file
	{
		
		std::filesystem::path filePath(filepath);

		if (filePath.has_extension()) 
		{
			if (filePath.extension().string()=="md")
			{
				delete file;
				file = new MD();
				return;
			}
			if (filePath.extension().string() == "txt")
			{
				delete file;
				file = new TXT();
				return;
			}
			throw std::invalid_argument("Invalid file extension");
		}
		throw std::invalid_argument("File hasn`t extension");
		
	}
	throw std::invalid_argument("File doesn`t exist");
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
	std::cout << "\"openl path_to_file\" - open local file\n";
	system("pause");
	system("cls");
}