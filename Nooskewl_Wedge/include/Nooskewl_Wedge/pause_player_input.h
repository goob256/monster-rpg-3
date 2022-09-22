#ifndef NOOSKEWL_WEDGE_PAUSE_PLAYER_INPUT_H
#define NOOSKEWL_WEDGE_PAUSE_PLAYER_INPUT_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Pause_Player_Input_Step : public Step
{
public:
	Pause_Player_Input_Step(bool paused, Task *this_task);
	virtual ~Pause_Player_Input_Step();
	
	void start();
	bool run();

private:
	bool paused;
};

}

#endif // NOOSKEWL_WEDGE_PAUSE_PLAYER_INPUT_H
