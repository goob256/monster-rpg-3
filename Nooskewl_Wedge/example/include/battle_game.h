#ifndef BATTLE_GAME_H
#define BATTLE_GAME_H

#include <Nooskewl_Wedge/battle_game.h>

class Battle_Game : public wedge::Battle_Game
{
public:
	Battle_Game(std::string bg, int bg_delay);
	virtual ~Battle_Game();

	void start_transition_in();
	void start_transition_out();
	wedge::Battle_Player *create_player(int index);
	void draw();
	void start_hit_effect(wedge::Battle_Entity *attacked);
	void show_retry_boss_gui();
};

#endif // BATTLE_GAME_H
