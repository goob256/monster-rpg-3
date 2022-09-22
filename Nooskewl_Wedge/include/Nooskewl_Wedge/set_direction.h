#ifndef NOOSKEWL_WEDGE_SET_DIRECTION_H
#define NOOSKEWL_WEDGE_SET_DIRECTION_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Set_Direction_Step : public Step
{
public:
	Set_Direction_Step(Map_Entity *entity, Direction direction, bool set_animation, bool moving, Task *task);
	virtual ~Set_Direction_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	Direction direction;
	bool set_animation;
	bool moving;
};

}

#endif // NOOSKEWL_WEDGE_SET_DIRECTION_H
