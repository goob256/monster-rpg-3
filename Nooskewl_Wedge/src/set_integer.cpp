#include "Nooskewl_Wedge/set_integer.h"

using namespace wedge;

namespace wedge {

Set_Integer_Step::Set_Integer_Step(int *i, int value, Task *task) :
	Step(task),
	i(i),
	value(value)
{
}

Set_Integer_Step::~Set_Integer_Step()
{
}

bool Set_Integer_Step::run()
{
	*i = value;

	send_done_signal();

	return false;
}

}
