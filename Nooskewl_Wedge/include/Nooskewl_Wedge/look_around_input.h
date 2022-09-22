#ifndef NOOSKEWL_WEDGE_LOOK_AROUND_INPUT_H
#define NOOSKEWL_WEDGE_LOOK_AROUND_INPUT_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Look_Around_Input_Step : public Map_Entity_Input_Step
{
public:
	static const int DELAY = 5000;

	Look_Around_Input_Step(Map_Entity *entity, std::vector<Direction> dont_look, Task *task);
	virtual ~Look_Around_Input_Step();

	bool run(); // return false to pop this Step and advance to the next

protected:
	int ticks;
	std::vector<Direction> dont_look;
};

}

#endif // NOOSKEWL_WEDGE_LOOK_AROUND_INPUT_H
