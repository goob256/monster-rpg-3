#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/set_animation.h"

using namespace wedge;

namespace wedge {

Set_Animation_Step::Set_Animation_Step(Map_Entity *entity, std::string animation, Task *task) :
	Step(task),
	entity(entity),
	animation(animation)
{
}

Set_Animation_Step::~Set_Animation_Step()
{
}

void Set_Animation_Step::start()
{
	gfx::Sprite *sprite = entity->get_sprite();
	if (sprite) {
		sprite->set_animation(animation);
	}
}

bool Set_Animation_Step::run()
{
	send_done_signal();
	return false;
}

}
