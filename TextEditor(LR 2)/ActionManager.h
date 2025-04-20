#pragma once

#include<string>
#include<vector>
#include<stdexcept>

class ActionManager
{
public:
	void redo();
	void undo();
	void clear(std::string);
	std::string get() const;
	void add(std::string);
private:
	std::vector<std::string>action;
	size_t ind=0;
};

