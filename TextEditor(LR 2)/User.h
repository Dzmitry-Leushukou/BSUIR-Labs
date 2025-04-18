#pragma once

#include <string>
#include <vector>
#include <Windows.h>

class User
{
public:
	User(std::string, std::vector<std::pair<std::string, char>>[3], std::vector<std::string>);
	User(std::string);
	void addPermission(char type, std::string path,char storage);
	std::vector<std::vector<std::pair<std::string, char>>> getPermissions();
	std::string getName();
	char getPermission(std::string);
	std::vector<std::string>getStyles()const;
	void addStyle(std::string s);
	char getStorage(std::string s);
	std::pair<std::string, char>getFile(int);
	void deleteFile(std::pair<std::string, char>);
private:
	std::string username;
	std::vector<std::pair<std::string, char>> permissions[3];
	std::vector<std::string>styles;
};

