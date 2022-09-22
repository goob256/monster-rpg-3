#include "Nooskewl_Wedge/pause_task.h"

using namespace wedge;

namespace wedge {

Pause_Task_Step::Pause_Task_Step(Task *task_to_pause, bool paused, Task *this_task) :
	Step(this_task),
	task_to_pause(task_to_pause),
	paused(paused)
{
}

Pause_Task_Step::~Pause_Task_Step()
{
}

void Pause_Task_Step::start()
{
	task_to_pause->set_paused(paused);
}

bool Pause_Task_Step::run()
{
	send_done_signal();
	return false;
}

}
