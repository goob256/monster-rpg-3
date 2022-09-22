#ifndef ENEMIES_H
#define ENEMIES_H

#include <Nooskewl_Wedge/battle_enemy.h>
#include <Nooskewl_Wedge/battle_player.h>

class Enemy_Slime : public wedge::Battle_Enemy
{
public:
	Enemy_Slime();
	virtual ~Enemy_Slime();

	bool start();
	bool take_turn();

protected:
};

#endif // ENEMIES_H
