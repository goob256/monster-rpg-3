#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>

#include "battle_transition_out.h"
#include "battle_transition_out2.h"
#include "transition.h"

Battle_Transition_Out_Step::Battle_Transition_Out_Step(wedge::Task *task) :
	Transition_Step(true, task)
{
}

Battle_Transition_Out_Step::~Battle_Transition_Out_Step()
{
}

bool Battle_Transition_Out_Step::run()
{
	bool ret = Transition_Step::run();
	if (ret == false) {
		NEW_SYSTEM_AND_TASK(AREA)
		Battle_Transition_Out2_Step *step = new Battle_Transition_Out2_Step(BATTLE, new_task);
		ADD_STEP(step)
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)

		BATTLE = NULL;
	}
	return ret;
}
