#include "Nooskewl_Wedge/battle_enemy.h"
#include "Nooskewl_Wedge/battle_entity.h"
#include "Nooskewl_Wedge/battle_game.h"
#include "Nooskewl_Wedge/battle_player.h"
#include "Nooskewl_Wedge/generic_callback.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/special_number.h"
#include "Nooskewl_Wedge/stats.h"

using namespace wedge;

namespace wedge {

Battle_Entity::Battle_Entity(Type type, std::string name) :
	type(type),
	name(name),
	stats(0),
	sprite(0),
	defending(false),
	spell_effect_offset(0.0f, 0.0f)
{
}

Battle_Entity::~Battle_Entity()
{
}

bool Battle_Entity::start()
{
	return true;
}

void Battle_Entity::handle_event(TGUI_Event *event)
{
}

void Battle_Entity::draw_back()
{
}

void Battle_Entity::draw()
{
}

void Battle_Entity::draw_fore()
{
}

bool Battle_Entity::take_turn()
{
	return true;
}

bool Battle_Entity::is_dead()
{
	return stats->hp <= 0;
}

void Battle_Entity::take_damage(int hp, int type, int y_offset)
{
	stats->hp = MAX(0, stats->hp - hp);

	std::string text = util::itos(std::abs(hp));

	util::Point<int> topleft, bottomright;
	sprite->get_bounds(topleft, bottomright);
		
	util::Point<int> number_pos = get_decoration_offset(shim::font->get_text_width(text), util::Point<int>(shim::tile_size*3/4, 0), NULL);

	number_pos.y += y_offset;

	SDL_Colour colour;
	SDL_Colour shadow_colour;

	if (type == STATUS_POISONED) {
		colour = shim::palette[29];
		shadow_colour = shim::palette[27];
	}
	else if (hp < 0) {
		colour = shim::palette[13];
		shadow_colour = shim::palette[27];
	}
	else {
		colour = shim::palette[20];
		shadow_colour = shim::palette[27];
	}

	NEW_SYSTEM_AND_TASK(BATTLE)
	Special_Number_Step *step = new Special_Number_Step(colour, shadow_colour, text, number_pos, Special_Number_Step::SHAKE, new_task);
	ADD_STEP(step)
	Generic_Callback_Step *g = new Generic_Callback_Step(battle_hit_callback, BATTLE, new_task);
	step->add_monitor(g);
	ADD_STEP(g)
	ADD_TASK(new_task)
	FINISH_SYSTEM(BATTLE)
	
	BATTLE->inc_waiting_for_hit(1);
}

Battle_Entity::Type Battle_Entity::get_type()
{
	return type;
}

Base_Stats *Battle_Entity::get_stats()
{
	// Kind of a hack to put this here but it's convenient
	stats->set_name(name);
	return stats;
}

gfx::Sprite *Battle_Entity::get_sprite()
{
	return sprite;
}

std::string Battle_Entity::get_name()
{
	return name;
}

bool Battle_Entity::is_defending()
{
	return defending;
}

util::Point<int> Battle_Entity::get_decoration_offset(int decoration_width, util::Point<int> offset, int *flags)
{
	util::Point<int> pos;

	util::Point<int> topleft;
	util::Point<int> bottomright;
	sprite->get_bounds(topleft, bottomright);

	if (type == Battle_Entity::PLAYER) {
		Battle_Player *player = static_cast<Battle_Player *>(this);
		pos = player->get_draw_pos() + util::Point<int>(-(decoration_width + offset.x), offset.y) + topleft;
		if (flags != NULL) {
			*flags = gfx::Image::FLIP_H;
		}
	}
	else {
		Battle_Enemy *enemy = static_cast<Battle_Enemy *>(this);
		pos = enemy->get_position() + offset + util::Point<int>(bottomright.x, topleft.y);
		if (flags != NULL) {
			*flags = 0;
		}
	}

	return pos;
}

float Battle_Entity::get_poison_odds()
{
	return 0.0f;
}

void Battle_Entity::run()
{
}

void Battle_Entity::lost_device()
{
}

void Battle_Entity::found_device()
{
}

void Battle_Entity::resize(util::Size<int> new_size)
{
}

util::Point<float> Battle_Entity::get_spell_effect_offset()
{
	return spell_effect_offset;
}

}
