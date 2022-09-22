#ifndef NOOSKEWL_WEDGE_SET_VISIBLE_H
#define NOOSKEWL_WEDGE_SET_VISIBLE_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Set_Visible_Step : public Step
{
public:
	Set_Visible_Step(Map_Entity *entity, bool visible, Task *task);
	virtual ~Set_Visible_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	bool visible;
};

}

#endif // NOOSKEWL_WEDGE_SET_VISIBLE_H
