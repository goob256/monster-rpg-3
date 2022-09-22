#ifndef BATTLE_TRANSITION_OUT_H
#define BATTLE_TRANSITION_OUT_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

#include "transition.h"

class Battle_Transition_Out_Step : public Transition_Step
{
public:
	Battle_Transition_Out_Step(wedge::Task *task);
	virtual ~Battle_Transition_Out_Step();
	
	void handle_event(TGUI_Event *event);
	bool run();
	void start();
};

#endif // BATTLE_TRANSITION_OUT_H
