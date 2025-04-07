#include "MDSaver.h"

void MDSaver::save(std::vector<std::string> text, std::string path)
{
	std::ofstream fout(path);
	for (auto& i : text)
		fout << i << '\n';
}
