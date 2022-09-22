#ifndef BATTLE_TRANSITION_IN2_H
#define BATTLE_TRANSITION_IN2_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

#include "transition.h"

class Battle_Transition_In2_Step : public Transition_Step
{
public:
	Battle_Transition_In2_Step(wedge::Task *task);
	virtual ~Battle_Transition_In2_Step();
	
	bool run();
	void start();
};

#endif // BATTLE_TRANSITION_IN2_H
