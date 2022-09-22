#include "start_battle.h"

Start_Battle_Step::Start_Battle_Step(wedge::Battle_Game *battle_game, wedge::Task *task) :
	Step(task),
	battle_game(battle_game)
{
}

Start_Battle_Step::~Start_Battle_Step()
{
}

bool Start_Battle_Step::run()
{
	battle_game->start_transition_in();
	send_done_signal();
	return false;
}
