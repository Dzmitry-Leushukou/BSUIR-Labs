#include "pch.h"
#include "../ActionManager.h"


TEST(ActionManager, UndoEmpty) 
{
	ActionManager* act = new ActionManager();
	EXPECT_THROW(act->undo(),std::runtime_error);
}
TEST(ActionManager, RedoEmpty)
{
	ActionManager* act = new ActionManager();
	EXPECT_THROW(act->redo(), std::runtime_error);
}
