#ifndef NOOSKEWL_WEDGE_BATTLE_PLAYER_H
#define NOOSKEWL_WEDGE_BATTLE_PLAYER_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/battle_entity.h"
#include "Nooskewl_Wedge/stats.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Battle_Player : public Battle_Entity
{
public:
	Battle_Player(std::string name, int index);
	virtual ~Battle_Player();

	virtual void handle_event(TGUI_Event *event) = 0;
	virtual void draw() = 0;
	virtual void run() = 0;
	virtual bool take_turn() = 0;
	virtual bool start();
	virtual void draw_fore();
	virtual void take_damage(int hp, int type, int y_offset = 0);
	virtual util::Point<float> get_draw_pos();

	bool is_dead();
	Player_Stats *get_player_stats();
	int get_index(); // what their index into Globals::Instance::stats is

protected:
	wedge::Player_Stats *player_stats;
	int index;
};

}

#endif // NOOSKEWL_WEDGE_BATTLE_PLAYER_H
