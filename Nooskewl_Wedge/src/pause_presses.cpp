#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/pause_presses.h"

using namespace wedge;

namespace wedge {

Pause_Presses_Step::Pause_Presses_Step(bool paused, bool repeat_pressed, Task *this_task) :
	Step(this_task),
	paused(paused),
	repeat_pressed(repeat_pressed)
{
}

Pause_Presses_Step::~Pause_Presses_Step()
{
}

void Pause_Presses_Step::start()
{
	pause_presses(paused, repeat_pressed);
}

bool Pause_Presses_Step::run()
{
	send_done_signal();
	return false;
}

}
