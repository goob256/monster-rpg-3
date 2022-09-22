#include "Nooskewl_Wedge/wait.h"

using namespace wedge;

namespace wedge {

Wait_Step::Wait_Step(Task *task) :
	Step(task),
	done(false)
{
}

Wait_Step::~Wait_Step()
{
}

bool Wait_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return !done;
}

void Wait_Step::done_signal(Step *step)
{
	done = true;
}

}
