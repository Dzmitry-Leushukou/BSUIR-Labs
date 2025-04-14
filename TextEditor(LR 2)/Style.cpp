#include "Style.h"

Style& Style::getInstance()
{
	static Style instance;	
	return instance;
}

void Style::clear()
{
	setColor("07");
	setFontSize(16);
}	

void Style::setColor(std::string color)
{
	std::string cmd = "color " + color;
	this->color = stoi(color);
	setTextStyle();
}

void Style::setFontSize(int size)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_FONT_INFOEX cfi;

	cfi.cbSize = sizeof(cfi);
	GetCurrentConsoleFontEx(hConsole, FALSE, &cfi);

	cfi.dwFontSize.Y = size; // Set the font height
	SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
}

void Style::setTextStyle(std::string type)
{
	if (type == "default")
	{
		setTextAttributes(color);
	}
	if (type == "bold")
	{

	}
	if (type == "underline")
	{

	}
	if (type == "italic")
	{

	}
	if (type == "header1")
	{

	}
	if (type == "header2")
	{

	}
	if (type == "header3")
	{

	}
	if (type == "found")
	{
		setTextAttributes(BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_BLUE);
	}
}

void Style::setUserPresets(std::vector<std::string>p)
{
	std::string color = "07";
	int size = 16;
	for (auto& i : p)
	{
		if (i.size() <= 2)
			size = stoi(i);
		else
			color = i.at(i.size() - 2), color+=i.at(i.size() - 1);
	}
	
	setColor(color);
	setFontSize(size);
}

void Style::setTextAttributes(WORD attributes)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, attributes);
}
