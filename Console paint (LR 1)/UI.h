#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Canvas.h"

class UI
{
public:
	UI();
	void inputHandler();
private:
	void process(std::string);
	void show();
	
	//Commands
	void help();
	void wrong();
	void set(char, char);

	//Fields
	Canvas* canvas;
};

