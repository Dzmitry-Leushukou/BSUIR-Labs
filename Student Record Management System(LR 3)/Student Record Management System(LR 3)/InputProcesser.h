#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <Windows.h>
#include "StudentDTO.h"
#include "Application.h"

class InputProcesser
{
public:
	static std::pair<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>> process(std::string);
	static Application* app;
private:
	static std::string getUsername();
	static std::string editUserMenu(int,std::vector<std::string>);
	static void simulateConsoleInput(const std::string& text);
	
};

