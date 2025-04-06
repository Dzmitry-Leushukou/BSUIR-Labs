#include "Serializer.h"

void Serializer::saveUsers(std::vector<User*>u,std::string file)
{
	std::ofstream fout(file);
	for (auto& i : u)
	{
		fout << i->getName()<<'\n';
		std::vector<std::vector<std::pair<std::string, char>>> p = i->getPermissions();
		for (int j = 0; j < p.size(); j++)
		{
			for (auto& k : p[j])
			{
				fout << k.first << " " << k.second << '\n';
			}
			fout << "*\n";
		}
	}

	fout.close();
}

std::vector<User*> Serializer::loadUsers(std::string file)
{
	std::vector<User*> users;
	std::ifstream fin(file);
	std::string user, s;

	while (fin >> user)
	{
		std::vector<std::pair<std::string, char>>p[3];
		for (int i = 0; i < 3; i++)
		{
			while (fin >> s && s != "*")
			{
				p[i].push_back({ s,0 });
				fin >> s;
				p[i].back().second = std::stoi(s);
			}
		}

		User* u = new User(user, p);
		users.push_back(u);
	}
	return users;
}