#ifndef NOOSKEWL_WEDGE_A_STAR_H
#define NOOSKEWL_WEDGE_A_STAR_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT A_Star_Step : public Step
{
public:
	A_Star_Step(Map_Entity *entity, util::Point<int> goal, Task *task);
	virtual ~A_Star_Step();
	
	bool run();
	void start();
	
	void done_signal(Step *step);

	void set_check_solids(bool check_solids);
	void set_allow_out_of_bounds(bool allow_out_of_bounds);

private:
	Map_Entity *entity;
	util::Point<int> goal;
	bool done;
	bool check_solids;
	bool allow_out_of_bounds;
};

}

#endif // NOOSKEWL_WEDGE_A_STAR_H
