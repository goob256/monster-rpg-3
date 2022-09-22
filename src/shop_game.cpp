#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/globals.h>

#include "shop.h"
#include "shop_game.h"
					
Shop_Game::Shop_Game(wedge::Object_Type type, std::vector<wedge::Object> items) :
	type(type),
	items(items)
{
}

Shop_Game::~Shop_Game()
{
}

bool Shop_Game::start()
{
	if (Game::start() == false) {
		return false;
	}

	Shop_GUI *gui = new Shop_GUI(0, 0, 0, true, type, items);
	shim::guis.push_back(gui);

	return true;
}

bool Shop_Game::run()
{
	Game::run(); // don't return if false, there are no systems usually

	if (shim::guis.size() == 0) {
		AREA->end_shop();
		return false;
	}

	return true;
}

void Shop_Game::draw()
{
	Game::draw();
}

void Shop_Game::draw_fore()
{
	Game::draw_fore();
}
