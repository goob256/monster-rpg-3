#ifndef NOOSKEWL_WEDGE_WANDER_INPUT_H
#define NOOSKEWL_WEDGE_WANDER_INPUT_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Wander_Input_Step : public Map_Entity_Input_Step
{
public:
	static const int DELAY = 2500;

	Wander_Input_Step(Map_Entity *entity, bool can_wander_l, bool can_wander_r, bool can_wander_u, bool can_wander_d, util::Point<int> home_position, int max_dist_from_home, Task *task);
	virtual ~Wander_Input_Step();

	bool run(); // return false to pop this Step and advance to the next

protected:
	bool can_wander[4]; // l, r, u, d
	int num_directions; // number of directions entity can move in eg 2 for l and r
	int last_move_direction; // 0-3, lrud
	int ticks;
	util::Point<int> home_position;
	int max_dist_from_home;
	bool waiting_for_movement_end;
};

}

#endif // NOOSKEWL_WEDGE_WANDER_INPUT_H
