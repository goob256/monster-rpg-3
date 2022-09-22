#ifndef NOOSKEWL_WEDGE_GENERIC_GUI_H
#define NOOSKEWL_WEDGE_GENERIC_GUI_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Generic_GUI_Step : public Step
{
public:
	Generic_GUI_Step(gui::GUI *gui, bool resize, Task *task);
	virtual ~Generic_GUI_Step();

	void start();
	bool run();

	void set_done(bool done);

private:
	bool done;
	gui::GUI *gui;
	bool resize;
};

}

#endif // NOOSKEWL_WEDGE_GENERIC_GUI_H
