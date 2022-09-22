#include "Nooskewl_Wedge/systems.h"

using namespace wedge;

namespace wedge {

System::System(Game *game) :
	game(game),
	paused(false)
{
}

System::~System()
{
	for (Task_List::iterator it = tasks.begin(); it != tasks.end(); it++) {
		Task *task = *it;
		delete task;
	}
}

bool System::run()
{
	if (paused) {
		return true;
	}

	Task_List tmp_tasks = tasks;

	for (Task_List::iterator it = tmp_tasks.begin(); it != tmp_tasks.end();) {
		Task *task = *it;
		if (!task->run()) {
			Task_List::iterator it2 = std::find(tasks.begin(), tasks.end(), task);
			if (it2 != tasks.end()) { // should always find it
				tasks.erase(it2);
				delete task;
			}
		}
		it++;
	}

	return tasks.size() != 0;
}

void System::handle_event(TGUI_Event *event)
{
	if (paused) {
		return;
	}

	Task_List tmp_list = tasks; // save this, we don't want to pump events to steps added in handle_event right away

	for (Task_List::iterator it = tmp_list.begin(); it != tmp_list.end(); it++) {
		(*it)->handle_event(event);
	}
}

void System::draw()
{
	if (paused) {
		return;
	}

	for (Task_List::iterator it = tasks.begin(); it != tasks.end(); it++) {
		(*it)->draw();
	}
}

void System::draw_fore()
{
	if (paused) {
		return;
	}

	for (Task_List::iterator it = tasks.begin(); it != tasks.end(); it++) {
		(*it)->draw_fore();
	}
}

void System::set_paused(bool paused)
{
	this->paused = paused;
}

bool System::is_paused()
{
	return paused;
}

Task_List &System::get_tasks()
{
	return tasks;
}

Game *System::get_game()
{
	return game;
}

void System::remove_task(Task *task)
{
	for (Task_List::iterator it = tasks.begin(); it != tasks.end(); it++) {
		Task *t = *it;
		if (task == t) {
			tasks.erase(it);
			return;
		}
	}
}

void System::lost_device()
{
	for (Task_List::iterator it = tasks.begin(); it != tasks.end(); it++) {
		Task *t = *it;
		t->lost_device();
	}
}

void System::found_device()
{
	for (Task_List::iterator it = tasks.begin(); it != tasks.end(); it++) {
		Task *t = *it;
		t->found_device();
	}
}

}
