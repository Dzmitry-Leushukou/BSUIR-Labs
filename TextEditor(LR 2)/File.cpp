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

void File::cut(int ll, int lc, int rl, int rc)
{
	std::vector<std::string>updText;
	std::string cutted, newLine;
	for (int i = 0; i < ll; i++)
		updText.push_back(text[i]);
	for (int i = 0; i < lc; i++)
		newLine += text[ll][i];
	if(rl==ll)
		for (int i = lc; i <= rc; i++)
		{
			cutted += text[ll][i];
		}
	else 
	{
		for (int i = lc; i < text[ll].size(); i++)
		{
			cutted += text[ll][i];
		}
		ll++;
		while (ll < rl)
		{
			cutted += "*\\n*";
			for (int i = 0; i < text[ll].size(); i++)
				cutted += text[ll][i];
			ll++;
		}
		cutted += "*\\n*";
		for (int i = 0; i <= rc; i++)
			cutted += text[ll][i];

	}

	for (int j = rc + 1; j < text[rl].size(); j++)
		newLine += text[rl][j];
	updText.push_back(newLine);

	for (int i = rl+1; i < text.size(); i++)
		updText.push_back(text[i]);
	text = updText;

	OpenClipboard(NULL);
	EmptyClipboard();
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, cutted.size() + 1);
	if (hGlobal) 
	{
		memcpy(GlobalLock(hGlobal), cutted.c_str(), cutted.size() + 1);
		GlobalUnlock(hGlobal);
		SetClipboardData(CF_TEXT, hGlobal);
	}
	CloseClipboard();
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