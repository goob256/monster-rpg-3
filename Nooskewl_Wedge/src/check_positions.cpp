#include "Nooskewl_Wedge/check_positions.h"
#include "Nooskewl_Wedge/map_entity.h"

using namespace wedge;

namespace wedge {

Check_Positions_Step::Check_Positions_Step(std::vector<Map_Entity *> entities, std::vector< util::Point<int> > positions, bool check_for_zero_offset, Task *task) :
	Step(task),
	entities(entities),
	positions(positions),
	check_for_zero_offset(check_for_zero_offset)
{
}

Check_Positions_Step::~Check_Positions_Step()
{
}

bool Check_Positions_Step::run()
{
	bool found = false;
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->get_position() != positions[i]) {
			found = true;
			break;
		}
		else if (check_for_zero_offset && entities[i]->get_offset() != util::Point<float>(0.0f, 0.0f)) {
			found = true;
			break;
		}
	}
	if (found == false) {
		send_done_signal();
	}
	return found == true;
}

}
