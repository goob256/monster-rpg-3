#ifndef ENEMY_DRAWING_HOOK_H
#define ENEMY_DRAWING_HOOK_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Monster_RPG_3_Battle_Enemy;

class Enemy_Drawing_Hook_Step : public wedge::Step
{
public:
	Enemy_Drawing_Hook_Step(Monster_RPG_3_Battle_Enemy *enemy, bool hook_draw_last);
	virtual ~Enemy_Drawing_Hook_Step();

	void hook();
	void draw_fore();

private:
	Monster_RPG_3_Battle_Enemy *enemy;
	bool hook_draw_last;
};

#endif // ENEMY_DRAWING_HOOK_H
