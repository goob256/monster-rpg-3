#ifndef NOOSKEWL_WEDGE_WAIT_H
#define NOOSKEWL_WEDGE_WAIT_H

// Wait for signal, call add_monitor on the Step to wait for

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Wait_Step : public Step
{
public:
	Wait_Step(Task *task);
	virtual ~Wait_Step();
	
	bool run();
	
	void done_signal(Step *step);

private:
	bool done;
};

}

#endif // NOOSKEWL_WEDGE_WAIT_H
