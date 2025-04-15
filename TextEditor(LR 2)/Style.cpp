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

void Style::setTextStyle(int b, int u, int i,int h)
{
	
	if (b == 0 && u == 0 && i == 0 && h == 0)
	{
		setTextStyle();
		return;
	}
	
	int result = 0;
	if (h > 0)
		result |= headerColor(h);
	else
		result |= color / 10;
	if (b > 0&&h==0)
		result |= 1;
	if (u > 0)
		result |= 2;
	if (i > 0)
		result |= 4;
	setTextAttributes(result);
}

int Style::headerColor(int h)
{
	switch (h)
	{
	case 1:
		return BACKGROUND_RED;
	case 2:
		return (BACKGROUND_BLUE);
	case 3:
		return (BACKGROUND_GREEN);
	case 4:
		return (BACKGROUND_RED | BACKGROUND_BLUE);
	case 5:
		return (BACKGROUND_BLUE | BACKGROUND_GREEN);
	case 6:
		return (BACKGROUND_RED | BACKGROUND_GREEN);
	}
}

void Style::setTextStyle(std::string type)
{
	if (type == "default")
	{
		setTextAttributes(color);
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
