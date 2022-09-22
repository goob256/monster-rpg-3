#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/set_visible.h"

using namespace wedge;

namespace wedge {

Set_Visible_Step::Set_Visible_Step(Map_Entity *entity, bool visible, Task *task) :
	Step(task),
	entity(entity),
	visible(visible)
{
}

Set_Visible_Step::~Set_Visible_Step()
{
}

void Set_Visible_Step::start()
{
	entity->set_visible(visible);
}

bool Set_Visible_Step::run()
{
	send_done_signal();
	return false;
}

}
