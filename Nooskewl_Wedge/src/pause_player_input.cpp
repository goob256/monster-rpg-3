#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/pause_player_input.h"

using namespace wedge;

namespace wedge {

Pause_Player_Input_Step::Pause_Player_Input_Step(bool paused, Task *this_task) :
	Step(this_task),
	paused(paused)
{
}

Pause_Player_Input_Step::~Pause_Player_Input_Step()
{
}

void Pause_Player_Input_Step::start()
{
	pause_player_input(paused);
}

bool Pause_Player_Input_Step::run()
{
	send_done_signal();
	return false;
}

}
