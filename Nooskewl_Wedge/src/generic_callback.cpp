#include "Nooskewl_Wedge/generic_callback.h"

using namespace wedge;

namespace wedge {

Generic_Callback_Step::Generic_Callback_Step(util::Callback callback, void *callback_data, Task *task) :
	Step(task),
	callback(callback),
	callback_data(callback_data),
	done(false)
{
}

Generic_Callback_Step::~Generic_Callback_Step()
{
}

bool Generic_Callback_Step::run()
{
	if (done == true) {
		send_done_signal();
	}
	return done == false;
}

void Generic_Callback_Step::done_signal(Step *step)
{
	if (callback) {
		callback(callback_data);
	}
	done = true;
}

}
