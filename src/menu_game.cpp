#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/globals.h>

#include "general.h"
#include "menu.h"
#include "menu_game.h"

Menu_Game::Menu_Game()
{
	if (wedge::is_onscreen_controller_enabled() == true && can_show_settings() == false) {
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

Menu_Game::~Menu_Game()
{
	for (std::vector<gui::GUI *>::iterator it = shim::guis.begin(); it != shim::guis.end(); it++) {
		gui::GUI *gui = *it;
		if (dynamic_cast<Menu_GUI *>(gui)) {
			gui->exit();
		}
	}
}

bool Menu_Game::start()
{
	if (Game::start() == false) {
		return false;
	}

	Stats_GUI *gui = new Stats_GUI(0);
	shim::guis.push_back(gui);

	return true;
}

bool Menu_Game::run()
{
	Game::run(); // don't return if false, there are no systems usually

	if (shim::guis.size() == 0 && AREA != NULL) {
		AREA->end_menu();
		return false;
	}

	return true;
}

void Menu_Game::draw()
{
	Game::draw();
}

void Menu_Game::draw_fore()
{
	Game::draw_fore();
}
