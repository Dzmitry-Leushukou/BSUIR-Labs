#include "File.h"

File::File(std::string PATH)
{
	path = PATH;

	std::ifstream fin;
	fin.open(path);
	std::string s;
	text.clear();
	while (std::getline(fin, s))
	{
		text.push_back(s);
	}

	fin.close();
}

std::vector<std::string> File::to_string() const
{
	return text;
}

void File::update(std::string s)
{
	std::string tmp;
	std::string back;
	text.clear();
	for (auto& i : s)
	{
		tmp += i;
		if (tmp.size() >= 4)
		{
			back = "";
			for (int j = tmp.size() - 4; j < tmp.size(); j++)
			{
				back += tmp[j];
			}

			if (back == "*\\n*") 
			{
				tmp.erase(tmp.begin() + tmp.size() - 4, tmp.end());
				text.push_back(tmp);
				tmp = "";
			}
		}
	}
	if (tmp.size() != 0)
		text.push_back(tmp);
}

void File::save()
{
	std::ofstream fout(path);
	for (auto& i : text)
		fout << i << '\n';
}

void File::saveAs(FileSaver * saver, std::string ext)
{
	saver->save(text, replaceExtension(ext));
}

std::string File::replaceExtension(std::string new_ext)
{
	std::size_t dotPos = path.find_last_of('.');

	if (dotPos != std::string::npos) {
		return path.substr(0, dotPos) + "." + new_ext;
	}

	return path + "." + new_ext;
}

std::string File::getPath() const
{
	return path;
}