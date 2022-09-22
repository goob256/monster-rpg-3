#ifndef JUMP_H
#define JUMP_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/systems.h>

class Jump_Step : public wedge::Step
{
public:
	Jump_Step(wedge::Map_Entity *entity, util::Point<float> run_offset, float jump_height, util::Point<int> dest_tile, wedge::Task *task);
	virtual ~Jump_Step();

	void start();
	bool run();

private:
	static const int JUMP_TIME_PER_PIXEL = 24; // ms

	wedge::Map_Entity *entity;
	util::Point<float> run_offset;
	float jump_height;
	util::Point<int> dest_tile;
	util::Point<int> start_tile;
	Uint32 jump_start;
	int jump_time; // JUMP_TIME_PER_PIXEL * pixels
	bool jumping;
	int jump_pixels;
	bool was_solid;
};

#endif // JUMP_H
