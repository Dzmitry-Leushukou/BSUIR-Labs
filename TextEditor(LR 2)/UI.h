#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <Windows.h>

#include "File.h"
#include "MD.h"

class UI
{
public:
	void inputHandler();
private:
//Input processing methods
	void create();
	void openl(std::string);
	void del();
	void edit();
	void save();
	void load();
	void show();
	void close();
	void showcreateMenu();
	void showFiles();
	void wrong(std::string s = "");
	void process(std::string);
	void help();
	void simulateConsoleInput(const std::string& text);
//Fields
	File * file = nullptr;
	unsigned char mode = 0;
};

