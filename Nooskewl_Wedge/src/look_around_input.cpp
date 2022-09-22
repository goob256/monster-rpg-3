#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/look_around_input.h"
#include "Nooskewl_Wedge/map_entity.h"

using namespace wedge;

namespace wedge {

Look_Around_Input_Step::Look_Around_Input_Step(Map_Entity *entity, std::vector<Direction> dont_look, Task *task) :
	Map_Entity_Input_Step(entity, task),
	ticks(0),
	dont_look(dont_look)
{
	ticks = util::rand(0, DELAY/(1000.0f/shim::logic_rate));
}

Look_Around_Input_Step::~Look_Around_Input_Step()
{
}

bool Look_Around_Input_Step::run()
{
	ticks++;
	int elapsed = ticks * (1000.0f/shim::logic_rate);
	if (elapsed >= DELAY) {
		std::vector<Direction> can_look;
		if (std::find(dont_look.begin(), dont_look.end(), DIR_E) == dont_look.end()) {
			can_look.push_back(DIR_E);
		}
		if (std::find(dont_look.begin(), dont_look.end(), DIR_W) == dont_look.end()) {
			can_look.push_back(DIR_W);
		}
		if (std::find(dont_look.begin(), dont_look.end(), DIR_N) == dont_look.end()) {
			can_look.push_back(DIR_N);
		}
		if (std::find(dont_look.begin(), dont_look.end(), DIR_S) == dont_look.end()) {
			can_look.push_back(DIR_S);
		}
		if (can_look.size() != 0) {
			int r = util::rand(0, (int)can_look.size()-1);
			entity->set_direction(can_look[r], true, false);
		}
		ticks = 0;
	}

	return true;
}

}
