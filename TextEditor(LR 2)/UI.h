#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <limits>

#include "User.h"
#include "File.h"
#include "Serializer.h"

class UI
{
public:
	void inputHandler();
	UI();
private:
//Input processing methods
	void create();
	void open();
	void style();
	void wrong(std::string s = "");
	void process(std::string);
	void help();
	void simulateConsoleInput(const std::string& text);
//Menu methods
	int getStorageType();
	std::string getFilePath(bool);
	int getNumber(int,int);
	void showUsersList();
	void userMenu();
	void fileMenu();
	void editMenu();
	void show(char);
	void changeColor();
	void changeFont();
//Fields
	File * file = nullptr;
	unsigned char mode = 0;
	std::vector<User*>users;
	User* user = nullptr;
	std::vector<std::string> files;
};

