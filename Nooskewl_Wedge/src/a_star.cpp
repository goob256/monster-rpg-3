#include "Nooskewl_Wedge/a_star.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/map_entity.h"

using namespace wedge;

namespace wedge {

A_Star_Step::A_Star_Step(Map_Entity *entity, util::Point<int> goal, Task *task) :
	Step(task),
	entity(entity),
	goal(goal),
	done(false),
	check_solids(true),
	allow_out_of_bounds(false)
{
}

A_Star_Step::~A_Star_Step()
{
}

bool A_Star_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return !done;
}

void A_Star_Step::start()
{
	Map_Entity_Input_Step *input_step = entity->get_input_step();
	if (input_step != NULL && input_step->set_path(goal, false, check_solids, allow_out_of_bounds) == true) {
		input_step->add_monitor(this);
	}
	else {
		done = true;
	}
}

void A_Star_Step::done_signal(Step *step)
{
	done = true;
}

void A_Star_Step::set_allow_out_of_bounds(bool allow_out_of_bounds)
{
	this->allow_out_of_bounds = allow_out_of_bounds;
}

void A_Star_Step::set_check_solids(bool check_solids)
{
	this->check_solids = check_solids;
}

}
