#pragma once

#include <string>
#include <vector>
class User
{
public:
	User(std::string, std::vector<std::pair<std::string, char>>[3]);
	User(std::string);
	void addPermission(char type, std::string path,char);
	std::vector<std::vector<std::pair<std::string, char>>> getPermissions();
	std::string getName();
private:
	std::string username;
	std::vector<std::pair<std::string, char>> permissions[3];
};

