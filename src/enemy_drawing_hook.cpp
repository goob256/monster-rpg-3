#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/omnipresent.h>

#include "enemies.h"
#include "enemy_drawing_hook.h"

Enemy_Drawing_Hook_Step::Enemy_Drawing_Hook_Step(Monster_RPG_3_Battle_Enemy *enemy, bool hook_draw_last) :
	wedge::Step(NULL),
	enemy(enemy),
	hook_draw_last(hook_draw_last)
{
}

Enemy_Drawing_Hook_Step::~Enemy_Drawing_Hook_Step()
{
}

void Enemy_Drawing_Hook_Step::hook()
{
	if (hook_draw_last) {
		OMNIPRESENT->hook_draw_last(this);
	}
	else {
		OMNIPRESENT->hook_draw_fore(this);
	}
}

void Enemy_Drawing_Hook_Step::draw_fore()
{
	enemy->draw_fore();
}
