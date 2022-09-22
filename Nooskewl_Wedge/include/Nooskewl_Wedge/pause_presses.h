#ifndef NOOSKEWL_WEDGE_PAUSE_PRESSES_H
#define NOOSKEWL_WEDGE_PAUSE_PRESSES_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Pause_Presses_Step : public Step
{
public:
	Pause_Presses_Step(bool paused, bool repeat_pressed, Task *this_task);
	virtual ~Pause_Presses_Step();
	
	void start();
	bool run();

private:
	bool paused;
	bool repeat_pressed;
};

}

#endif // NOOSKEWL_WEDGE_PAUSE_PRESSES_H
