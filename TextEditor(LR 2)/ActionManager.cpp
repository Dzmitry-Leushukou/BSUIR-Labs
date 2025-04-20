#include "ActionManager.h"

void ActionManager::redo()
{
	if (ind == action.size() - 1)
		throw std::runtime_error("Nothing to redo");
	ind++;

}
void ActionManager::undo()
{
	if (ind == 0)
		throw std::runtime_error("Nothing to undo");
	ind--;
}
void ActionManager::clear(std::string s)
{
	ind = 0;
	action.clear();
	action.push_back(s);
}
std::string ActionManager::get() const
{
	return action.at(ind);
}
void ActionManager::add(std::string s)
{
	while (ind != action.size() - 1)
		action.pop_back();
	action.push_back(s);
	ind++;
}