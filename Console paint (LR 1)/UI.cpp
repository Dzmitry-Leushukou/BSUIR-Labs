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
	std::cout << "\"/move\" - go to move mode (help inside mode)\n";
	std::cout << "\"/draw id x1 y1 x2 y2 x3 y3\" - draw selected figure\n";
	std::cout << "NOTE: id = [0 - rectangle (x1 y1 x2 y2), 1 - triangle(x1 y1 x2 y2 x3 y3), 2 - cirlce(x1 y1 x2(radius))]\n";
	std::cout << "\"/fill\" - go to fill mode (help inside mode)\n"; 
	std::cout << "\"/erase\" - go to erase mode (help inside mode)\n";
	std::cout << "\"/clear\" - clear palette\n";
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

	if (fill_mode)
	{
		if (com == "q")
		{
			fill_mode = false;
			show();
		}
		else
			fill(com);
		return;
	}

	if (erase_mode)
	{
		if (com == "q")
		{
			erase_mode = false;
			show();
		}
		else
			erase(com);
		return;
	}

	if (com == "/clear")
	{
		clear();
		return;
	}

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

	if (com.size() > 5 && com.substr(0, 5) == "/draw")
	{
		draw(com.substr(5));
		return;
	}

	if (com=="/fill")
	{
		fill_mode = true;
		show();
		return;
	}

	if (com == "/erase")
	{
		erase_mode = true;
		show();
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

		i = 0;
		std::string ID;
		while (s.at(i) != ' ')
		{
			ID += s.at(i);
			i++;
		}

		id = stoi(ID);
		i++;
		std::string ch;
		while (s.at(i) != ' ')
		{
			ch += s.at(i);
			i++;
		}

		move_x = stoi(ch);
		i++;
		ch = "";
		while (i < s.size()&&(std::isdigit(s.at(i))||s.at(i)=='-'))
		{
			ch += s.at(i);
			i++;
		}

		if (i < s.size())
			throw std::out_of_range("");

		move_y = stoi(ch);

		canvas->move(id, move_x, move_y);
		show();
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
	for (int i = 0; i < a.at(0).size()+2; i++)
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
	for (int i = 0; i < a.at(0).size()+2; i++)
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

	if (fill_mode)
	{
		std::cout << "[FILL MODE]\n";
		std::cout << "For fill figure write \"id\"\nFor quit from mode write \"q\"\nFigures:\n";
		int id = 0;
		std::vector<const Figure*>figures = canvas->getFigures();
		for (auto& i : figures)
		{
			std::cout << "[" << id++ << "] " << i->toString() << "\n";
		}
	}

	if (erase_mode)
	{
		std::cout << "[ERASE MODE]\n";
		std::cout << "For erase figure write \"id\"\nFor quit from mode write \"q\"\nFigures:\n";
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

void UI::draw(std::string s) // get all after "/draw"
{
	int type = 0, i = 0;
	try
	{
		if (s.at(i++) != ' ')
			throw std::invalid_argument("Wrong params format");

		if(s.at(i+1)!=' ')
			throw std::invalid_argument("Wrong params format");

		type = s.at(i) - '0';
		i += 2;
		//get x1 y1
		std::vector<std::pair<int, int>>v;
		bool f = false;
		while (i < s.size())
		{
			std::string s1="";
			while (i < s.size() && (std::isdigit(s.at(i)) || s.at(i) == '-'))
			{
				s1 += s.at(i);
				i++;
			}
			if (i < s.size() && s.at(i) == ' ') // space
				i++;
			else
				if (i < s.size()) //!space but string not end
					throw std::invalid_argument("Wrong params format");

			if (f)
			{
				v.back().second = stoi(s1), f = false;
			}
			else
				v.push_back({ stoi(s1),0 }), f = true;
		}

		if((f&&type!=2)||(!f&&type==2))
			throw std::invalid_argument("Wrong params format");

		canvas->draw(type, v);
		
	}
	catch (const std::out_of_range& e)
	{
		wrong();
		return;
	}
	catch (const std::invalid_argument& e)
	{
		std::cout << e.what() << "\n";
		wrong();
		return;
	}
	show();
}

void UI::fill(std::string s)
{
	try 
	{
		for (auto& i : s)
		{
			if (!isdigit(i))
				throw std::invalid_argument("");
		}
		int id = stoi(s);
		canvas->fill(id);
		show();
	}
	catch (const std::out_of_range& e)
	{
		wrong();
		return;
	}
	catch (const std::invalid_argument& e)
	{
		std::cout << e.what() << '\n';
		wrong();
		return;
	}
	
}

void UI::erase(std::string s)
{
	try
	{
		for (auto& i : s)
		{
			if (!isdigit(i))
				throw std::invalid_argument("");
		}
		int id = stoi(s);
		canvas->erase(id);
		show();
	}
	catch (const std::out_of_range& e)
	{
		wrong();
		return;
	}
	catch (const std::invalid_argument& e)
	{
		std::cout << e.what() << '\n';
		wrong();
		return;
	}
}

void UI::clear()
{
	while (canvas->getFigures().size())
	{
		canvas->erase(0);
	}
	show();
}