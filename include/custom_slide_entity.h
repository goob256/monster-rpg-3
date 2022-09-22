#ifndef CUSTOM_SLIDE_ENTITY_H
#define CUSTOM_SLIDE_ENTITY_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

namespace wedge {
	class Map_Entity;
}

class Custom_Slide_Entity_Step : public wedge::Step
{
public:
	Custom_Slide_Entity_Step(wedge::Map_Entity *entity, util::Point<int> destination_tile, float speed, int *check, wedge::Task *task);
	virtual ~Custom_Slide_Entity_Step();

	void start();
	bool run();

private:
	wedge::Map_Entity *entity;
	util::Point<int> destination_tile;
	float speed;
	int *check;
	util::Point<int> start_pos;
	util::Point<float> start_offset;
};

#endif // CUSTOM_SLIDE_ENTITY_H
