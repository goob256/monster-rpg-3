#ifndef NOOSKEWL_WEDGE_SET_INTEGER_H
#define NOOSKEWL_WEDGE_SET_INTEGER_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Set_Integer_Step : public Step
{
public:
	Set_Integer_Step(int *i, int value, Task *task);
	virtual ~Set_Integer_Step();

	bool run();

private:
	int *i;
	int value;
};

}

#endif // NOOSKEWL_WEDGE_SET_INTEGER_H
