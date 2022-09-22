#ifndef NOOSKEWL_WEDGE_WAIT_FOR_INTEGER_H
#define NOOSKEWL_WEDGE_WAIT_FOR_INTEGER_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Wait_For_Integer_Step : public Step
{
public:
	Wait_For_Integer_Step(int *i, int desired_value, Task *task);
	virtual ~Wait_For_Integer_Step();
	
	bool run();

private:
	int *i;
	int desired_value;
};

}

#endif // NOOSKEWL_WEDGE_WAIT_FOR_INTEGER_H
