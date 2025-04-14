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
	void targetText();
	void setUserPresets(std::vector<std::string>);
	static Style& getInstance();
private:
	
};

