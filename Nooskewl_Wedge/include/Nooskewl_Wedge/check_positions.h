#ifndef NOOSKEWL_WEDGE_CHECK_POSITIONS_H
#define NOOSKEWL_WEDGE_CHECK_POSITIONS_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Check_Positions_Step : public Step
{
public:
	Check_Positions_Step(std::vector<Map_Entity *> entities, std::vector< util::Point<int> > positions, bool check_for_zero_offset, Task *task);
	virtual ~Check_Positions_Step();

	bool run();

private:
	std::vector<Map_Entity *> entities;
	std::vector< util::Point<int> > positions;
	bool check_for_zero_offset;
};

}

#endif // NOOSKEWL_WEDGE_CHECK_POSITIONS_H
