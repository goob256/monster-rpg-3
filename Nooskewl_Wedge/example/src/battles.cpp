#include <Nooskewl_Wedge/globals.h>

#include "battles.h"
#include "enemies.h"
#include "inventory.h"

Battle_2Slimes::Battle_2Slimes() :
	Battle_Game("example", 0)
{
	boss_battle = false;
}

Battle_2Slimes::~Battle_2Slimes()
{
}

bool Battle_2Slimes::start()
{
	if (Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *slime1 = new Enemy_Slime();
	slime1->start();
	slime1->set_position(util::Point<int>(shim::tile_size*3, shim::tile_size*1.5));
	entities.push_back(slime1);

	gold += slime1->get_gold();
	experience += slime1->get_experience();

	wedge::Battle_Enemy *slime2 = new Enemy_Slime();
	slime2->start();
	slime2->set_position(util::Point<int>(shim::tile_size, shim::tile_size));
	entities.push_back(slime2);

	gold += slime2->get_gold();
	experience += slime2->get_experience();

	return true;
}

wedge::Object Battle_2Slimes::get_found_object()
{
	return OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 1);
}
