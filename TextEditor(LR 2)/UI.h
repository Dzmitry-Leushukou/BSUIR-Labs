#pragma once

#include <iostream>
#include <string>
#include <filesystem>

#include "File.h"
#include "MD.h"
#include "TXT.h"

class UI
{
public:
	void inputHandler();
private:
//Input processing methods
	void create();
	void open(std::string);
	void del();
	void edit();
	void save();
	void load();
	void wrong(std::string s = "");
	void process(std::string);
	void help();
//Fields
	File * file = nullptr;
};

