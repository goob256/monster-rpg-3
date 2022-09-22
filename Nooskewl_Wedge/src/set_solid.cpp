#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/set_solid.h"

using namespace wedge;

namespace wedge {

Set_Solid_Step::Set_Solid_Step(Map_Entity *entity, bool solid, Task *task) :
	Step(task),
	entity(entity),
	solid(solid)
{
}

Set_Solid_Step::~Set_Solid_Step()
{
}

void Set_Solid_Step::start()
{
	entity->set_solid(solid);
}

bool Set_Solid_Step::run()
{
	send_done_signal();
	return false;
}

}
