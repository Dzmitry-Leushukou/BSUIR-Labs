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
				fout << k.first << " " << (int)k.second << '\n';
			}
			fout << "*\n";
		}
		for (auto& j : i->getStyles())
		{
			fout << j << '\n';
		}
		fout << "*\n";
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
			while (fin >> s)
			{
				if (s == "*")
					break;
				p[i].push_back({ s,0 });
				fin >> s;
				p[i].back().second = std::stoi(s);
			}
		}
		std::vector<std::string>st;
		std::getline(fin, s);
		while (std::getline(fin, s))
		{
			if (s == "*")
				break;
			st.push_back(s);
		}
		User* u = new User(user, p,st);
		users.push_back(u);
	}
	return users;
}