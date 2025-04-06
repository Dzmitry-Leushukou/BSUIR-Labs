#include "User.h"

void User::addPermission(char type, std::string path, char st)
{
	permissions[type].push_back({ path,st });
}

User::User(std::string s, std::vector<std::pair<std::string, char>> p[3])
{
	username = s;
	for (int j = 0; j < 3; j++)
		permissions[j] = p[j];
}

User::User(std::string s)
{
	username = s;
}

std::vector<std::vector<std::pair<std::string, char>>> User::getPermissions()
{
	std::vector<std::vector<std::pair<std::string, char>>> ret;
	for (int i = 0; i < 3; i++)
		ret.push_back(permissions[i]);
	return ret;
}

std::string User::getName()
{
	return username;
}