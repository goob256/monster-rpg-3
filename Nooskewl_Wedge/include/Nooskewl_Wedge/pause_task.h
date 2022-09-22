#ifndef NOOSKEWL_WEDGE_PAUSE_TASK_H
#define NOOSKEWL_WEDGE_PAUSE_TASK_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Pause_Task_Step : public Step
{
public:
	Pause_Task_Step(Task *task_to_pause, bool paused, Task *this_task);
	virtual ~Pause_Task_Step();
	
	void start();
	bool run();

private:
	Task *task_to_pause;
	bool paused;
};

}

#endif // NOOSKEWL_WEDGE_PAUSE_TASK_H
