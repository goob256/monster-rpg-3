#ifndef NOOSKEWL_WEDGE_GENERIC_IMMEDIATE_CALLBACK_H
#define NOOSKEWL_WEDGE_GENERIC_IMMEDIATE_CALLBACK_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Generic_Immediate_Callback_Step : public Step
{
public:
	Generic_Immediate_Callback_Step(util::Callback callback, void *callback_data, Task *task);
	virtual ~Generic_Immediate_Callback_Step();

	void start();
	bool run();

private:
	util::Callback callback;
	void *callback_data;
};

}

#endif // NOOSKEWL_WEDGE_GENERIC_IMMEDIATE_CALLBACK_H
