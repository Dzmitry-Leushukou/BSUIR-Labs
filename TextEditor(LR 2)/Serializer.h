#pragma once

#include "User.h"
#include "fstream"

static class Serializer
{
public:
	static void saveUsers(std::vector<User*>,std::string file);
	static std::vector<User*> loadUsers(std::string);
};

