#include "Nooskewl_Wedge/battle_game.h"
#include "Nooskewl_Wedge/battle_player.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/omnipresent.h"

namespace wedge {

Battle_Player::Battle_Player(std::string name, int index) :
	Battle_Entity(Battle_Entity::PLAYER, name),
	index(index)
{
	stats = &INSTANCE->stats[index].base;
	player_stats = &INSTANCE->stats[index];
	sprite = INSTANCE->stats[index].sprite;
	
	if (stats->hp > 0) {
		sprite->set_animation("stand_w");
	}
	else {
		sprite->set_animation("dead");
	}
}

Battle_Player::~Battle_Player()
{
}

bool Battle_Player::start()
{
	return Battle_Entity::start();
}

bool Battle_Player::is_dead()
{
	return false;
}

wedge::Player_Stats *Battle_Player::get_player_stats()
{
	return player_stats;
}

int Battle_Player::get_index()
{
	return index;
}

util::Point<float> Battle_Player::get_draw_pos()
{
	util::Point<int> pos;
	pos.x = ((shim::screen_size.w / shim::tile_size) - 2) * shim::tile_size;
	pos.y = shim::tile_size*1.5 + (shim::tile_size+shim::tile_size/3)*index;
	return pos;
}

void Battle_Player::take_damage(int hp, int type, int y_offset)
{
	wedge::Battle_Entity::take_damage(hp, type, y_offset);
	if (stats->hp <= 0) {
		sprite->set_animation("dead");
		stats->status = wedge::STATUS_OK;
		bool all_dead = true;
		for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
			if (INSTANCE->stats[i].base.hp > 0) {
				all_dead = false;
			}
		}
		if (all_dead) {
			audio::stop_music();
			GLOBALS->gameover->play(shim::music_volume, true);
			gfx::add_notification(GLOBALS->game_t->translate(1355)/* Originally: You died in battle! */);
			BATTLE->set_gameover(true);
			OMNIPRESENT->start_fade(GLOBALS->gameover_fade_colour, GLOBALS->gameover_timeout-GLOBALS->gameover_fade_time, GLOBALS->gameover_fade_time);
		}
		rumble(1.0f, 1000);
	}
	else {
		rumble(1.0f, 500);
	}
}

void Battle_Player::draw_fore()
{
	Battle_Entity::draw_fore();
}

}
