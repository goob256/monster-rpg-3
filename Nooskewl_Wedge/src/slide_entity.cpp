#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/slide_entity.h"

using namespace wedge;

namespace wedge {

// speed is pixels per tick (60 ticks per second normally)
Slide_Entity_Step::Slide_Entity_Step(Map_Entity *entity, util::Point<int> destination_tile, float speed, Task *task) :
	Step(task),
	entity(entity),
	destination_tile(destination_tile),
	speed(speed)
{
}

Slide_Entity_Step::~Slide_Entity_Step()
{
}

void Slide_Entity_Step::start()
{
	start_pos = entity->get_position();
	start_offset = entity->get_offset();
}

bool Slide_Entity_Step::run()
{
	util::Point<float> pos;
	util::Point<int> tile = entity->get_position();
	util::Point<float> offset = entity->get_offset();
	tile_to_abs(tile, offset, pos);

	util::Point<float> dest;
	tile_to_abs(destination_tile, util::Point<float>(0.0f, 0.0f), dest);
	util::Point<float> start;
	tile_to_abs(start_pos, start_offset, start);
	util::Point<float> diff = dest - start;
	float angle = diff.angle();
	util::Point<float> inc(cos(angle) * speed, sin(angle) * speed);
	float full_dist = (dest-pos).length();
	bool done;
	float len = inc.length();
	if (full_dist <= speed || len >= full_dist) {
		done = true;

	}
	else {
		pos += inc;
		done = false;
	}

	if (done) {
		entity->set_position(destination_tile);
		entity->set_offset(util::Point<float>(0.0f, 0.0f));
	}
	else  {
		abs_to_tile(pos, tile, offset);

		entity->set_position(tile);
		entity->set_offset(offset);
	}
	
	if (done) {
		send_done_signal();
	}

	return done == false;
}

}
