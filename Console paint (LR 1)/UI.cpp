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
	std::cout << "\"/move\" - go to move mode (help about move figure showes when you go to it)\n";
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

	if (com == "/move")
	{
		move_mode = true;
		show();
		return;
	}

	if (move_mode)
	{
		if (com == "q")
		{
			move_mode = false;
			show();
		}
		else
			move(com);
		return;
	}
	wrong();
}

void UI::move(std::string s)
{
	int id, i, move_x,move_y;
	//check command
	try
	{
		std::string type;
		type = s[0] + s[1];

		if (type != "id"||s[2]!= ' ')
			throw std::out_of_range("");

		i = 3;
		std::string ID;
		while (s[i] != ' ')
		{
			ID += s[i];
			i++;
		}

		id = stoi(ID);

		std::string ch;
		while (s[i] != ' ')
		{
			ch += s[i];
			i++;
		}

		move_x = stoi(ch);

		ch = "";
		while (s[i] != ' ')
		{
			ch += s[i];
			i++;
		}

		move_y = stoi(ch);

		canvas->move(id, move_x, move_y);
	}
	catch (const std::out_of_range& e)
	{
		wrong();
		return;
	}
	catch (const std::invalid_argument& e)
	{
		std::cout << e.what()<<'\n';
		wrong();
		return;
	}
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

	if (move_mode)
	{
		std::cout << "[MOVE MODE]\n";
		std::cout << "For move figure write \"id x y\"\nFor quit from mode write \"q\"\nFigures:\n";
		int id = 0;
		std::vector<const Figure*>figures = canvas->getFigures();
		for (auto& i : figures)
		{
			std::cout << "[" << id++ << "] " << i->toString() << "\n";
		}
	}
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