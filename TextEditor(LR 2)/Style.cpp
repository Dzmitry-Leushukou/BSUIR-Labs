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
	if (type == "h1")
	{
		setTextAttributes(BACKGROUND_RED);
	}
	if (type == "h2")
	{
		setTextAttributes(BACKGROUND_BLUE);
	}
	if (type == "h3")
	{
		setTextAttributes(BACKGROUND_GREEN);
	}
	if (type == "h4")
	{
		setTextAttributes(BACKGROUND_RED| BACKGROUND_BLUE);
	}
	if (type == "h5")
	{
		setTextAttributes(BACKGROUND_BLUE | BACKGROUND_GREEN);
	}
	if (type == "h6")
	{
		setTextAttributes(BACKGROUND_RED | BACKGROUND_GREEN);
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
