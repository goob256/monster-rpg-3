#include <Nooskewl_Wedge/battle_enemy.h>
#include <Nooskewl_Wedge/stats.h>

#include "enemies.h"

Enemy_Slime::Enemy_Slime() :
	wedge::Battle_Enemy(TRANSLATE("Slime")END)
{
}

Enemy_Slime::~Enemy_Slime()
{
}

bool Enemy_Slime::start()
{
	experience = 8;
	gold = 11;
	sprite = new gfx::Sprite("slime");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 50;
	stats->fixed.attack = 30;
	stats->fixed.defense = 10;

	return true;
}

bool Enemy_Slime::take_turn()
{
	return wedge::Battle_Enemy::turn_attack();
}
