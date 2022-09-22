#ifndef PLAY_ANIMATION_AND_DELETE_H
#define PLAY_ANIMATION_AND_DELETE_H

#include <Nooskewl_Wedge/systems.h>

class Map_Entity;

class Play_Animation_And_Delete_Step : public wedge::Step
{
public:
	Play_Animation_And_Delete_Step(wedge::Map_Entity *entity, std::string anim_name, wedge::Task *task);
	virtual ~Play_Animation_And_Delete_Step();
	
	void start();
	bool run();

	void set_done(bool done);

private:
	wedge::Map_Entity *entity;
	std::string anim_name;
	bool done;
};

#endif // PLAY_ANIMATION_AND_DELETE_H
