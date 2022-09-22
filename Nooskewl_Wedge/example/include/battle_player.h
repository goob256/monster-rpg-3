#ifndef BATTLE_PLAYER_H
#define BATTLE_PLAYER_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/battle_player.h>
#include <Nooskewl_Wedge/stats.h>

class Battle_Player : public wedge::Battle_Player
{
public:
	Battle_Player();
	~Battle_Player();

	void handle_event(TGUI_Event *event);
	void draw();
	void draw_fore();
	void run();
	bool take_turn();

private:
	bool turn_started;
};

#endif // BATTLE_PLAYER_H
