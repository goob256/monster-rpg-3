#include <Nooskewl_Wedge/globals.h>

#include "battle_game.h"
#include "battle_transition_out2.h"
#include "transition.h"

Battle_Transition_Out2_Step::Battle_Transition_Out2_Step(wedge::Battle_Game *battle_game, wedge::Task *task) :
	Transition_Step(false, task),
	battle_game(battle_game)
{
}

Battle_Transition_Out2_Step::~Battle_Transition_Out2_Step()
{
}

void Battle_Transition_Out2_Step::start()
{
	delete battle_game;

	Transition_Step::start();
}

bool Battle_Transition_Out2_Step::run()
{
	return Transition_Step::run();
}
