#pragma once
#include <iostream>
#include <vector>
#include "InputProcesser.h"
#include "Application.h"

class PresentationLayer
{
public:
	void inputHandler();
private:
	void output(std::pair<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>>data);
	void operations(const std::vector<std::string> op);
	void showStudents();
	
};

