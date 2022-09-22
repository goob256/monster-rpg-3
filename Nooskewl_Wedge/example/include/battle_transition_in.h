#ifndef BATTLE_TRANSITION_IN_H
#define BATTLE_TRANSITION_IN_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

#include "battle_game.h"
#include "transition.h"

class Battle_Transition_In_Step : public Transition_Step
{
public:
	Battle_Transition_In_Step(wedge::Battle_Game *battle_game, wedge::Task *task);
	virtual ~Battle_Transition_In_Step();
	
	bool run();

private:
	wedge::Battle_Game *battle_game;
};

#endif // BATTLE_TRANSITION_IN_H
