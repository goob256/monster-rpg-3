#include "Nooskewl_Wedge/systems.h"

using namespace wedge;

namespace wedge {

Game::Game()
{
}

Game::~Game()
{
	for (System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		delete *it;
	}
}

bool Game::start()
{
	return true;
}

void Game::handle_event(TGUI_Event *event)
{
	System_List tmp_list = systems; // save this, we don't want to pump events to steps added in handle_event right away

	for (System_List::iterator it = tmp_list.begin(); it != tmp_list.end(); it++) {
		(*it)->handle_event(event);
	}
}

bool Game::run()
{
	System_List::iterator it;

	for (it = systems.begin(); it != systems.end();) {
		System *system = *it;
		if (!system->run()) {
			it = systems.erase(it);
			delete system;
		}
		else {
			it++;
		}
	}

	return systems.size() != 0;
}

void Game::draw()
{
	for (System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		System *system = *it;
		system->draw();
	}
}

void Game::draw_fore()
{
	for (System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		System *system = *it;
		system->draw_fore();
	}
}

void Game::resize(util::Size<int> new_size)
{
}

System_List &Game::get_systems()
{
	return systems;
}

void Game::remove_system(System *system)
{
	for (System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		System *sys = *it;
		if (system == sys) {
			systems.erase(it);
			return;
		}
	}
}

void Game::lost_device()
{
	for (System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		System *sys = *it;
		sys->lost_device();
	}
}

void Game::found_device()
{
	for (System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		System *sys = *it;
		sys->found_device();
	}
}

}
