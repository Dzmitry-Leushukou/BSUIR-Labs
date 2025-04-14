#pragma once

#include <string>
#include <vector>
#include <Windows.h>

class User
{
public:
	User(std::string, std::vector<std::pair<std::string, char>>[3], std::vector<std::string>);
	User(std::string);
	void addPermission(char type, std::string path,char);
	std::vector<std::vector<std::pair<std::string, char>>> getPermissions();
	std::string getName();
	char getPermission(std::string);
	std::vector<std::string>getStyles()const;
	void addStyle(std::string s);
private:
	std::string username;
	std::vector<std::pair<std::string, char>> permissions[3];
	std::vector<std::string>styles;
};

