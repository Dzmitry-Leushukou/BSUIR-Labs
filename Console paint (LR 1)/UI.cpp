#include "UI.h"

UI::UI()
{
	canvas = new Canvas(20,60);
}

void UI::inputHandler()
{
	show();
	std::string com;
	while (true)
	{
		std::getline(std::cin,com);
		process(com);
		std::getline(std::cin, com);
		process(com);
		std::getline(std::cin, com);
		process(com);
		show();
	}
}

void UI::help()
{
	show();
	std::cout << "==Commands==\n";
	std::cout << "\"/help\" - show all commands\n";
	std::cout << "\"/set id (id = [0 - background, 1 - fill, 2 - draw]) char\" - set symbol for fill, draw, background\n";
	//std::cout << "/\n";
	//std::cout << "/set id (0 - background, 1 - fill, 2 - draw) symbol\n";
	system("pause");
}

void UI::wrong()
{
	std::cout << "Wrong command. For command help, type /help\n";
	system("pause");
	show();
}

void UI::process(std::string com)
{
	if (com == "/help")
	{
		help();
		return;
	}

	if (com.size() == 8 && com.substr(0, 4) == "/set")
	{
		set(com[5],com.back());
		return;
	}


	wrong();
}

void UI::show()
{
	system("cls");
	
	std::vector<std::vector<char>> a = canvas->getCanva();
	for (int i = 0; i < a[0].size()+2; i++)
	{
		std::cout << "_";
	}
	std::cout << '\n';
	for (auto& i : a)
	{
		std::cout << "|";
		for (auto& j : i)
			std::cout << j;
		std::cout << "|";
		std::cout << '\n';
	}
	for (int i = 0; i < a[0].size()+2; i++)
	{
		std::cout << "-";
	}
	std::cout << '\n';
}

void UI::set(char c,char s)
{
	if (c >= '0' && c <= '2')
	{
		canvas->set(c - '0', s);
		show();
	}
	else
		wrong();
}