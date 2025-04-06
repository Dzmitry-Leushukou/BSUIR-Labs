#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <limits>

#include "User.h"
#include "File.h"
#include "MD.h"
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
	
	void show(char);
	void userMenu();
	void close();
	
	void wrong(std::string s = "");
	void process(std::string);
	void help();
	void simulateConsoleInput(const std::string& text);
//Menu methods
	int getStorageType();
	std::string getFilePath(bool);
	int getNumber(int,int);
	void showUsersList();
//Fields
	File * file = nullptr;
	unsigned char mode = 0;
	std::vector<User*>users;
	User* user = nullptr;
};

