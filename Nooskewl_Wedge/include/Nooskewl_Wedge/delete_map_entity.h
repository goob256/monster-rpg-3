#ifndef NOOSKEWL_WEDGE_DELETE_MAP_ENTITY_H
#define NOOSKEWL_WEDGE_DELETE_MAP_ENTITY_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Delete_Map_Entity_Step : public Step
{
public:
	Delete_Map_Entity_Step(Map_Entity *entity, Task *task);
	virtual ~Delete_Map_Entity_Step();
	
	bool run();
	void start();

private:
	Map_Entity *entity;
};

}

#endif // NOOSKEWL_WEDGE_DELETE_MAP_ENTITY_H
