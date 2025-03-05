#pragma once

#include <vector>
#include "Canvas.h"
#include "Serializer.h"

class Actions
{
public:
	Canvas undo();
	Canvas redo();
	void addAction(Canvas * newact);
private:
	std::vector<std::vector<std::string>>actions;
	int ind;
};

