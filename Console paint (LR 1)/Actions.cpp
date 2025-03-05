#include "Actions.h"
Canvas Actions::undo()
{
	ind--;
	if (ind < 0)
	{
		ind = 0;
		throw std::invalid_argument("Nothing to undo");
	}
	return Serializer::DeserializeFromString(actions.at(ind));
}
Canvas Actions::redo()
{
	ind++;
	if (ind >= actions.size())
	{
		ind = actions.size() - 1;
		throw std::invalid_argument("Nothing to redo");
	}
	return Serializer::DeserializeFromString(actions.at(ind));
}
void Actions::addAction(Canvas* newact)
{
	if (ind != actions.size() - 1 && actions.size() != 0) // if some undos and has actions
	{
		while (ind != actions.size() - 1)
			actions.pop_back();
	}

	if (actions.size() != 0)
	{
		ind++;
	}
	else
		ind = 0;
	actions.push_back(Serializer::SerializeToString(newact->getSymb(),newact->getFigures()));
}