#include "Nooskewl_Wedge/delay.h"
#include "Nooskewl_Wedge/globals.h"

using namespace wedge;

namespace wedge {

Delay_Step::Delay_Step(int millis, Task *task) :
	Step(task),
	millis(millis)
{
}

Delay_Step::~Delay_Step()
{
}

void Delay_Step::start()
{
	start_time = GET_TICKS();
}

bool Delay_Step::run()
{
	int now = GET_TICKS();
	int diff = now - start_time;
	if (diff >= millis) {
		send_done_signal();
		return false;
	}
	return true;
}

}
