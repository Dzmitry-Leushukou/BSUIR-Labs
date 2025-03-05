#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <iostream>
#include "Canvas.h"

static class Serializer
{
public:
	static void Serialize(char symb[3],std::vector<const Figure*>figures, std::string filename);
	static std::vector<std::string> SerializeToString(char symb[3], std::vector<const Figure*>figures);
	static Canvas* Deserialize(std::string);
	static Canvas DeserializeFromString(std::vector<std::string>);
private:
	static long long readNumber(std::string s, int& i, int line);
	static int readInt(std::string s, int& i, int line);
	static bool checkCanvasFigures(Canvas * canva);
};


