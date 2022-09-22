#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/stats.h>

#include "battle_player.h"
#include "globals.h"

Battle_Player::Battle_Player() :
	wedge::Battle_Player("Hero", 0),
	turn_started(false)
{
}

Battle_Player::~Battle_Player()
{
}

void Battle_Player::handle_event(TGUI_Event *event)
{
}

void Battle_Player::draw()
{
	gfx::Image *current_image = sprite->get_current_image();
	util::Size<int> offset = (util::Size<int>(shim::tile_size, shim::tile_size) - current_image->size) / 2;
	util::Point<float> pos = get_draw_pos() + util::Point<int>(offset.w, offset.h);
	current_image->draw(pos);
}

void Battle_Player::draw_fore()
{
}

bool Battle_Player::take_turn()
{
	// this example just does a simple auto-attack

	if (turn_started == false) {
		static_cast<Globals *>(wedge::globals)->melee->play(false);
		sprite->set_animation("punch");
		sprite->reset();
		sprite->start();
		turn_started = true;
	}
	else {
		if (sprite->is_finished()) {
			std::vector<wedge::Battle_Entity *> enemies = BATTLE->get_enemies();
			BATTLE->hit(this, enemies[util::rand(0, enemies.size()-1)]);
			sprite->set_animation("stand_w");
			turn_started = false;
			return true;
		}
	}

	return false;
}

void Battle_Player::run()
{
}
