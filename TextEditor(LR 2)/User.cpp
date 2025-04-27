#include "User.h"

void User::addPermission(char type, std::string path, char st)
{
	deleteFile({ path,st });
	if (type == -1)
	{
		return;
	}
	permissions[type].push_back({ path,st });
}

void User::deleteFile(std::pair<std::string, char>f)
{
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < permissions[j].size(); i++)
		{
			if (permissions[j][i] == f)
			{
				permissions[j].erase(permissions[j].begin() + i);
				i--;
			}
		}
	}
}

std::pair<std::string, char>User::getFile(int id)
{
	for(int j=0;j<3;j++)
		for (int i = 0; i < permissions[j].size(); i++)
		{
			id--;
			if (id == 0)
				return permissions[j][i];
		}
	
}

char User::getStorage(std::string filepath)
{
	for (int j = 0; j < 3; j++)
	{
		for (auto& i : permissions[j])
		{
			if (i.first == filepath)
			{
				return i.second;
			}
		}
	}
}

User::User(std::string s, std::vector<std::pair<std::string, char>> p[3], std::vector<std::string>st)
{
	username = s;
	for (int j = 0; j < 3; j++)
		permissions[j] = p[j];
	styles = st;
}


void User::addStyle(std::string s)
{
	styles.push_back(s);
}

User::User(std::string s)
{
	username = s;
}

char User::getPermission(std::string filepath)
{
	for (int j = 0; j < 3; j++)
	{
		for (auto& i : permissions[j])
		{
			if (i.first == filepath)
			{
				return j;
			}
		}
	}
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
std::vector<std::string>User::getStyles()const
{
	return styles;
}