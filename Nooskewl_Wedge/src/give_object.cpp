#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/chest.h"
#include "Nooskewl_Wedge/give_object.h"
#include "Nooskewl_Wedge/globals.h"

using namespace wedge;

namespace wedge {

Give_Object_Step::Give_Object_Step(Object object, Dialogue_Position dialogue_position, Task *task) :
	Step(task),
	object(object),
	done(false),
	dialogue_position(dialogue_position)
{
}

Give_Object_Step::~Give_Object_Step()
{
}

void Give_Object_Step::start()
{
	GLOBALS->add_next_dialogue_monitor(this);
	Chest *chest = new Chest("tmp", "", object, -1, dialogue_position); // simplest way to get the same message format/etc
	chest->activate(AREA->get_player(0));
	delete chest;
}

bool Give_Object_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return done == false;
}

void Give_Object_Step::done_signal(Step *step)
{
	done = true;
}

} // End namespace wedge
