#ifndef NOOSKEWL_WEDGE_SET_SOLID_H
#define NOOSKEWL_WEDGE_SET_SOLID_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Set_Solid_Step : public Step
{
public:
	Set_Solid_Step(Map_Entity *entity, bool solid, Task *task);
	virtual ~Set_Solid_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	bool solid;
};

}

#endif // NOOSKEWL_WEDGE_SET_SOLID_H
