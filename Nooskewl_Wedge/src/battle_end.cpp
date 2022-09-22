#include "Nooskewl_Wedge/battle_end.h"
#include "Nooskewl_Wedge/battle_game.h"
#include "Nooskewl_Wedge/globals.h"

using namespace wedge;

namespace wedge {

Battle_End_Step::Battle_End_Step(Task *task) :
	Step(task),
	count(0)
{
}

Battle_End_Step::~Battle_End_Step()
{
}

bool Battle_End_Step::run()
{
	return true;
}

void Battle_End_Step::done_signal(Step *step)
{
	count++;
	BATTLE->battle_end_signal();
}

}
