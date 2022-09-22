#ifndef START_BATTLE_H
#define START_BATTLE_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

#include "battle_game.h"
#include "transition.h"

class Start_Battle_Step : public wedge::Step
{
public:
	Start_Battle_Step(wedge::Battle_Game *battle_game, wedge::Task *task);
	virtual ~Start_Battle_Step();
	
	bool run();

private:
	wedge::Battle_Game *battle_game;
};

#endif // START_BATTLE_H
