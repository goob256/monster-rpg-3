#ifndef NOOSKEWL_WEDGE_PLAYER_INPUT_H
#define NOOSKEWL_WEDGE_PLAYER_INPUT_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Player_Input_Step : public Map_Entity_Input_Step
{
public:
	Player_Input_Step(Map_Entity *entity, Task *task);
	virtual ~Player_Input_Step();

	void handle_event(TGUI_Event *event);

	void set_input_paused(bool paused);

private:
	bool input_paused;
};

}

#endif // NOOSKEWL_WEDGE_PLAYER_INPUT_H
