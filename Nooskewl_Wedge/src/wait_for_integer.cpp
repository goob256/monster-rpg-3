#include "Nooskewl_Wedge/wait_for_integer.h"

using namespace wedge;

namespace wedge {

Wait_For_Integer_Step::Wait_For_Integer_Step(int *i, int desired_value, Task *task) :
	Step(task),
	i(i),
	desired_value(desired_value)
{
}

Wait_For_Integer_Step::~Wait_For_Integer_Step()
{
}

bool Wait_For_Integer_Step::run()
{
	bool done = *i == desired_value;
	if (done) {
		send_done_signal();
	}
	return !done;
}

}
