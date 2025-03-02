#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "Canvas.h"
#include "Figure.h"

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
	void move(std::string);
	void draw(std::string);
	void fill(std::string);
	void erase(std::string);


	//Fields
	Canvas* canvas;
	bool move_mode = false;
	bool fill_mode = false;
};

