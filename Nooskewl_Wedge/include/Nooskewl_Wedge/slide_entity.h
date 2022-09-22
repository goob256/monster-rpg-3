#ifndef NOOSKEWL_WEDGE_SLIDE_ENTITY_H
#define NOOSKEWL_WEDGE_SLIDE_ENTITY_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Slide_Entity_Step : public Step
{
public:
	Slide_Entity_Step(Map_Entity *entity, util::Point<int> destination_tile, float speed, Task *task);
	virtual ~Slide_Entity_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	util::Point<int> destination_tile;
	float speed;
	util::Point<int> start_pos;
	util::Point<float> start_offset;
};

}

#endif // NOOSKEWL_WEDGE_SLIDE_ENTITY_H
