#ifndef NOOSKEWL_WEDGE_BATTLE_END_H
#define NOOSKEWL_WEDGE_BATTLE_END_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Battle_End_Step : public Step
{
public:
	Battle_End_Step(Task *task);
	virtual ~Battle_End_Step();
	
	bool run();
	void done_signal(Step *step);

private:
	int count;
};

}

#endif // NOOSKEWL_WEDGE_BATTLE_END_H
