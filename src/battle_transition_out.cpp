#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>

#include "battle_transition_out.h"
#include "battle_transition_out2.h"
#include "globals.h"
#include "transition.h"

Battle_Transition_Out_Step::Battle_Transition_Out_Step(wedge::Task *task) :
	Transition_Step(true, task)
{
}

Battle_Transition_Out_Step::~Battle_Transition_Out_Step()
{
}

void Battle_Transition_Out_Step::handle_event(TGUI_Event *event)
{
	// This makes movement after battle during the transition out work without being annoying and ignoring keys/buttons
	// NOPE: this can cause sliding player when dialogue pops up after battle
	//AREA->handle_event(event);
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
	
		send_done_signal();
	}
	return ret;
}

void Battle_Transition_Out_Step::start()
{
	Transition_Step::start();

	if (AREA->get_current_area()->get_name().substr(0, 6) == "desert") {
		M3_GLOBALS->wind1->stop(); // Was playing quietly from Battle_Transition_In2_Step
		M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
		if (util::rand(0, 1) == 0) {
			M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
		}
		else {
			M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
		}
	}
}
