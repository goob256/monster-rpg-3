#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/set_direction.h"

using namespace wedge;

namespace wedge {

Set_Direction_Step::Set_Direction_Step(Map_Entity *entity, Direction direction, bool set_animation, bool moving, Task *task) :
	Step(task),
	entity(entity),
	direction(direction),
	set_animation(set_animation),
	moving(moving)
{
}

Set_Direction_Step::~Set_Direction_Step()
{
}

void Set_Direction_Step::start()
{
	entity->set_direction(direction, set_animation, moving);
}

bool Set_Direction_Step::run()
{
	send_done_signal();
	return false;
}

}
