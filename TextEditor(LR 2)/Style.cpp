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
	system(cmd.c_str());
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
void Style::targetText()
{

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