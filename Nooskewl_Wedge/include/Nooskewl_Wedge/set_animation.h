#ifndef NOOSKEWL_WEDGE_SET_ANIMATION_H
#define NOOSKEWL_WEDGE_SET_ANIMATION_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Set_Animation_Step : public Step
{
public:
	Set_Animation_Step(Map_Entity *entity, std::string animation, Task *task);
	virtual ~Set_Animation_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	std::string animation;
};

}

#endif // NOOSKEWL_WEDGE_SET_ANIMATION_H
