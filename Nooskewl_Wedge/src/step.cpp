#include "Nooskewl_Wedge/systems.h"

using namespace wedge;

namespace wedge {

Step::Step(Task *task) :
	task(task),
	step_initialised(false),
	dying(false),
	disowned(false)
{
}

Step::~Step()
{
	remove_monitors();
}

bool Step::run()
{
	return true;
}

void Step::handle_event(TGUI_Event *event)
{
}

void Step::draw()
{
}

void Step::draw_fore()
{
}

void Step::start()
{
}

bool Step::is_initialised()
{
	return step_initialised;
}

void Step::set_initialised(bool initialised)
{
	step_initialised = initialised;
}

Task *Step::get_task()
{
	return task;
}

void Step::add_monitor(Step *step)
{
	monitors.push_back(step);
	step->set_monitoring(this);
}

void Step::remove_monitor(Step *step)
{
	for (Step_List::iterator it = monitors.begin(); it != monitors.end(); it++) {
		Step *s = *it;
		if (s == step) {
			monitors.erase(it);
			step->stop_monitoring(this);
			return;
		}
	}
}

void Step::set_monitoring(Step *step)
{
	monitoring.push_back(step);
}

void Step::stop_monitoring(Step *step)
{
	for (Step_List::iterator it = monitoring.begin(); it != monitoring.end(); it++) {
		Step *s = *it;
		if (s == step) {
			monitoring.erase(it);
			return;
		}
	}
}

void Step::done_signal(Step *step)
{
}

void Step::send_done_signal()
{
	for (Step_List::iterator it = monitors.begin(); it != monitors.end(); it++) {
		(*it)->done_signal(this);
	}
}

void Step::die()
{
	dying = true;
}

bool Step::is_dying()
{
	return dying;
}

void Step::lost_device()
{
}

void Step::found_device()
{
}

void Step::remove_monitors()
{
	Step_List::iterator it;

	while (monitors.size() > 0) {
		it = monitors.begin();
		Step *step = *it;
		remove_monitor(step);
	}

	while (monitoring.size() > 0) {
		it = monitoring.begin();
		Step *step = *it;
		step->remove_monitor(this);
	}
}

bool Step::is_disowned()
{
	return disowned;
}

void Step::disown()
{
	disowned = true;
}

}
