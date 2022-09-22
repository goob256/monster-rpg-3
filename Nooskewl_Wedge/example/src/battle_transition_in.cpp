#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>

#include "battle_game.h"
#include "battle_transition_in.h"
#include "battle_transition_in2.h"
#include "transition.h"

Battle_Transition_In_Step::Battle_Transition_In_Step(wedge::Battle_Game *battle_game, wedge::Task *task) :
	Transition_Step(true, task),
	battle_game(battle_game)
{
}

Battle_Transition_In_Step::~Battle_Transition_In_Step()
{
}

bool Battle_Transition_In_Step::run()
{
	bool ret = Transition_Step::run();
	if (ret == false) {
		BATTLE = battle_game;
		BATTLE->start();
		BATTLE->show_enemy_stats(false);
		BATTLE->show_player_stats(false);
		NEW_SYSTEM_AND_TASK(BATTLE)
		Battle_Transition_In2_Step *step = new Battle_Transition_In2_Step(new_task);
		ADD_STEP(step)
		ADD_TASK(new_task)
		FINISH_SYSTEM(BATTLE)
	}
	return ret;
}
