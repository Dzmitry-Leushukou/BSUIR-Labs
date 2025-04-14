#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <limits>
#include <deque>

#include "User.h"
#include "File.h"
#include "Serializer.h"
#include "TXTSaver.h"
#include "JSONSaver.h"
#include "XMLSaver.h"
#include "FileSaver.h"
#include "MDSaver.h"
#include "Style.h"

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
	void cutMenu();
	void find(std::string s);
//Fields
	File * file = nullptr;
	unsigned char mode = 0;
	std::vector<User*>users;
	User* user = nullptr;
	std::vector<std::string> files;
	Style& styler = Style::getInstance();
//Savers
	FileSaver* txt_saver = new TXTSaver();
	FileSaver* json_saver = new JSONSaver();
	FileSaver* xml_saver = new XMLSaver();
	FileSaver* md_saver = new MDSaver();
};

