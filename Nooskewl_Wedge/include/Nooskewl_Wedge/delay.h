#ifndef NOOSKEWL_WEDGE_DELAY_H
#define NOOSKEWL_WEDGE_DELAY_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Delay_Step : public Step
{
public:
	Delay_Step(int millis, Task *task);
	virtual ~Delay_Step();
	
	void start();
	bool run();

private:
	int millis;
	int start_time;
};

}

#endif // NOOSKEWL_WEDGE_DELAY_H
