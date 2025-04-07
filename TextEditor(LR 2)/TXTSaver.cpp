#include "TXTSaver.h"

void TXTSaver::save(std::vector<std::string> text, std::string path)
{
	std::ofstream fout(path);
	for (auto& i : text)
		fout << i << '\n';
}