#include "Nooskewl_Wedge/generic_immediate_callback.h"

using namespace wedge;

namespace wedge {

Generic_Immediate_Callback_Step::Generic_Immediate_Callback_Step(util::Callback callback, void *callback_data, Task *task) :
	Step(task),
	callback(callback),
	callback_data(callback_data)
{
}

Generic_Immediate_Callback_Step::~Generic_Immediate_Callback_Step()
{
}

void Generic_Immediate_Callback_Step::start()
{
	callback(callback_data);
}

bool Generic_Immediate_Callback_Step::run()
{
	send_done_signal();
	return false;
}

}
