#ifndef BATTLE_TRANSITION_OUT2_H
#define BATTLE_TRANSITION_OUT2_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

#include "battle_game.h"
#include "transition.h"

class Battle_Transition_Out2_Step : public Transition_Step
{
public:
	Battle_Transition_Out2_Step(wedge::Battle_Game *battle_game, wedge::Task *task);
	virtual ~Battle_Transition_Out2_Step();
	
	void start();
	bool run();

private:
	wedge::Battle_Game *battle_game;
};

#endif // BATTLE_TRANSITION_OUT2_H
