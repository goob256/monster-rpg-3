#include "Nooskewl_Wedge/generic_gui.h"

using namespace wedge;

namespace wedge {

Generic_GUI_Step::Generic_GUI_Step(gui::GUI *gui, bool resize, Task *task) :
	Step(task),
	done(false),
	gui(gui),
	resize(resize)
{
}

Generic_GUI_Step::~Generic_GUI_Step()
{
}

void Generic_GUI_Step::start()
{
	if (resize) {
		gui->resize(shim::screen_size);
	}
	shim::guis.push_back(gui);
}

bool Generic_GUI_Step::run()
{
	if (done == true) {
		send_done_signal();
	}
	return done == false;
}

void Generic_GUI_Step::set_done(bool done)
{
	this->done = done;
}

}
