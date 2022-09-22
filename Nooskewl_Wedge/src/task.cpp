#include "Nooskewl_Wedge/systems.h"

using namespace wedge;

namespace wedge {

Task::Task(System *system) :
	system(system),
	paused(false)
{
}

Task::~Task()
{
	for (Step_List::iterator it = steps.begin(); it != steps.end(); it++) {
		Step *step = *it;
		delete step;
	}
}

bool Task::run()
{
	if (paused) {
		return true;
	}

	if (steps.size() > 0) {
		Step_List::iterator it = steps.begin();
		Step *step = *it;
		if (step->is_initialised() == false) {
			step->start();
			step->set_initialised(true);
		}
		if (step->is_dying() || step->run() == false) {
			steps.erase(it);
			delete step;
		}
		else if (step->is_disowned()) {
			Game *game = system->get_game();
			NEW_SYSTEM_AND_TASK(game)
			ADD_STEP(step)
			ADD_TASK(new_task)
			System_List &systems = game->get_systems();
			for (System_List::iterator it2 = systems.begin(); it2 != systems.end(); it2++) {
				if (*it2 == system) {
					systems.insert(it2, new_system);
					break;
				}
			}
			steps.erase(it);
		}
	}

	return steps.size() != 0;
}

void Task::handle_event(TGUI_Event *event)
{
	if (paused) {
		return;
	}

	if (steps.size() > 0) {
		Step_List::iterator it = steps.begin();
		Step *step = *it;
		if (step->is_initialised() == false) {
			step->start();
			step->set_initialised(true);
		}
		if (step->is_dying() == false) {
			step->handle_event(event);
		}
	}
}

void Task::draw()
{
	if (paused) {
		return;
	}

	if (steps.size() > 0) {
		Step_List::iterator it = steps.begin();
		Step *step = *it;
		if (step->is_initialised() == false) {
			step->start();
			step->set_initialised(true);
		}
		if (step->is_dying() == false) {
			step->draw();
		}
	}
}

void Task::draw_fore()
{
	if (paused) {
		return;
	}

	if (steps.size() > 0) {
		Step_List::iterator it = steps.begin();
		Step *step = *it;
		if (step->is_initialised() == false) {
			step->start();
			step->set_initialised(true);
		}
		if (step->is_dying() == false) {
			step->draw_fore();
		}
	}
}

void Task::set_paused(bool paused)
{
	this->paused = paused;
}

bool Task::is_paused()
{
	return paused;
}

Step_List &Task::get_steps()
{
	return steps;
}

System *Task::get_system()
{
	return system;
}

void Task::remove_step(Step *step)
{
	for (Step_List::iterator it = steps.begin(); it != steps.end(); it++) {
		Step *s = *it;
		if (step == s) {
			steps.erase(it);
			return;
		}
	}
}

void Task::lost_device()
{
	for (Step_List::iterator it = steps.begin(); it != steps.end(); it++) {
		Step *s = *it;
		s->lost_device();
	}
}

void Task::found_device()
{
	for (Step_List::iterator it = steps.begin(); it != steps.end(); it++) {
		Step *s = *it;
		s->found_device();
	}
}

}
