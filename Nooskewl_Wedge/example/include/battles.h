#ifndef BATTLES_H
#define BATTLES_H

#include <Nooskewl_Wedge/inventory.h>

#include "battle_game.h"

class Battle_2Slimes : public Battle_Game
{
public:
	Battle_2Slimes();
	virtual ~Battle_2Slimes();

	bool start();

private:
	wedge::Object get_found_object();
};

#endif // BATTLES_H
