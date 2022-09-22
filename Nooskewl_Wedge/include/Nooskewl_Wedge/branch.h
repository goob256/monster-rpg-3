#ifndef NOOSKEWL_WEDGE_BRANCH_H
#define NOOSKEWL_WEDGE_BRANCH_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Branch_Step : public Step
{
public:
	Branch_Step(int *i, Game *game, std::vector< std::vector< std::vector< Step *> > > steps, Task *task);
	virtual ~Branch_Step();

	bool run();

private:
	int *i;
	Game *game;
	std::vector< std::vector< std::vector< Step *> > > steps;
};

}

#endif // NOOSKEWL_WEDGE_BRANCH_H
