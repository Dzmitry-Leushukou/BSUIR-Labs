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
	//static void Deserialize(Canvas*& canvas);
};
/*
		=== Canvas class ===
		std::vector<std::vector<char>>canva;
		char symb[3] = { ' ', '#', '*' }; // 0 - back, 1 - fill, 2 - draw
		std::vector<Figure*>figures;
	*/

