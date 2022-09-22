#ifndef NOOSKEWL_WEDGE_GENERIC_CALLBACK_H
#define NOOSKEWL_WEDGE_GENERIC_CALLBACK_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Generic_Callback_Step : public Step
{
public:
	Generic_Callback_Step(util::Callback callback, void *callback_data, Task *task);
	virtual ~Generic_Callback_Step();
	
	bool run();
	void done_signal(Step *step);

private:
	util::Callback callback;
	void *callback_data;
	bool done;
};

}

#endif // NOOSKEWL_WEDGE_GENERIC_CALLBACK_H
