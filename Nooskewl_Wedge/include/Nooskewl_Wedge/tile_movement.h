#ifndef NOOSKEWL_WEDGE_TILE_MOVEMENT_H
#define NOOSKEWL_WEDGE_TILE_MOVEMENT_H

#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Tile_Movement_Step : public Step
{
public:
	Tile_Movement_Step(Map_Entity *entity, Direction direction, Task *task);
	virtual ~Tile_Movement_Step();
	
	bool run();

	// call this from done_signal() to keep going/change to 'direction'
	// or call it if not moving to start moving in 'direction'
	void set_next_direction(Direction direction);

	bool is_moving();

	bool hit_wall();

	void delay_movement(int frames);

protected:
	void go(Direction direction);

	Map_Entity *entity;
	util::Point<int> start_tile;
	util::Point<float> increment;
	util::Point<float> initial_offset;
	bool moving;
	Direction next_direction;
	bool waiting_for_next_direction;
	bool _hit_wall;
	int movement_delay;
};

}

#endif // NOOSKEWL_WEDGE_TILE_MOVEMENT_H
