#pragma once

#include <string>
#include <vector>
#include <Windows.h>

class Style
{
public:
	void clear();
	void setColor(std::string);
	void setFontSize(int);
	void setTextStyle(std::string = "default");
	void setTextStyle(int,int,int,int);
	void setUserPresets(std::vector<std::string>);
	static Style& getInstance();
private:
	void setTextAttributes(WORD attributes);
	int headerColor(int);
	int color;
};

