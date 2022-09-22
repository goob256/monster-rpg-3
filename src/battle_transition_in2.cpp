#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/globals.h>

#include "battle_game.h"
#include "battle_transition_in2.h"
#include "globals.h"
#include "transition.h"

Battle_Transition_In2_Step::Battle_Transition_In2_Step(wedge::Task *task) :
	Transition_Step(false, task)
{
}

Battle_Transition_In2_Step::~Battle_Transition_In2_Step()
{
}

bool Battle_Transition_In2_Step::run()
{
	bool ret = Transition_Step::run();
	if (ret == false) {
		BATTLE->show_enemy_stats(true);
		BATTLE->show_player_stats(true);
		send_done_signal();
	}
	return ret;
}

void Battle_Transition_In2_Step::start()
{
	Transition_Step::start();

	if (AREA->get_current_area()->get_name().substr(0, 6) == "desert") {
		M3_GLOBALS->wind1->stop();
		M3_GLOBALS->wind2->stop();
		M3_GLOBALS->wind3->stop();
		M3_GLOBALS->wind1->play(0.5f, true, audio::SAMPLE_TYPE_USER+0);
	}
}
