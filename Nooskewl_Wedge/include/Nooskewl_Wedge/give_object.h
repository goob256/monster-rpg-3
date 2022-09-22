#ifndef NOOSKEWL_WEDGE_GIVE_OBJECT_H
#define NOOSKEWL_WEDGE_GIVE_OBJECT_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Give_Object_Step : public Step
{
public:
	Give_Object_Step(Object object, Dialogue_Position dialogue_position, Task *task);
	virtual ~Give_Object_Step();

	void start();
	bool run();
	void done_signal(Step *step);

private:
	Object object;
	bool done;
	Dialogue_Position dialogue_position;
};

}

#endif // NOOSKEWL_WEDGE_GIVE_OBJECT_H
