#include <Nooskewl_Wedge/battle_enemy.h>
#include <Nooskewl_Wedge/battle_entity.h>
#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/battle_player.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/generic_immediate_callback.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/spells.h>
#include <Nooskewl_Wedge/stats.h>

#include "bolt.h"
#include "cure.h"
#include "fire.h"
#include "heal.h"
#include "ice.h"
#include "inventory.h"
#include "spells.h"
#include "stats.h"

#include "vampires.h"
#include "vbolt.h"
#include "vfire.h"
#include "vice.h"

const float MULTI_INCREASE = 0.1f;

const int FIRE_DAMAGE = 80;
const int BOLT_DAMAGE = 150;
const int ICE_DAMAGE = 400;

#define AMOUNT(v) (((v + (v * MULTI_INCREASE*(stats.size()-1))) / stats.size()) * scale)

static bool y_sort_battle_entity(wedge::Battle_Entity *a, wedge::Battle_Entity *b)
{
	int a_y = 0;
	int b_y = 0;

	wedge::Battle_Player *a_player = dynamic_cast<wedge::Battle_Player *>(a);
	if (a_player) {
		util::Point<float> pos = a_player->get_draw_pos();
		a_y = pos.y + shim::tile_size;
	}
	else {
		wedge::Battle_Enemy *a_enemy = dynamic_cast<wedge::Battle_Enemy *>(a);
		if (a_enemy) {
			util::Point<float> pos = a_enemy->get_position();
			gfx::Sprite *sprite = a_enemy->get_sprite();
			gfx::Image *img = sprite->get_current_image();
			util::Point<int> topleft, bottomright;
			img->get_bounds(topleft, bottomright);
			a_y = pos.y + bottomright.y;
		}
	}

	wedge::Battle_Player *b_player = dynamic_cast<wedge::Battle_Player *>(a);
	if (b_player) {
		util::Point<float> pos = b_player->get_draw_pos();
		b_y = pos.y + shim::tile_size;
	}
	else {
		wedge::Battle_Enemy *b_enemy = dynamic_cast<wedge::Battle_Enemy *>(a);
		if (b_enemy) {
			util::Point<float> pos = b_enemy->get_position();
			gfx::Sprite *sprite = b_enemy->get_sprite();
			gfx::Image *img = sprite->get_current_image();
			util::Point<int> topleft, bottomright;
			img->get_bounds(topleft, bottomright);
			b_y = pos.y + bottomright.y;
		}
	}

	return a_y < b_y;
}

Monster_RPG_3_Spell_Interface::~Monster_RPG_3_Spell_Interface()
{
}

bool Monster_RPG_3_Spell_Interface::is_white_magic(std::string name)
{
	if (name == "Heal" || name == "Cure" || name == "Heal Plus" || name == "Heal Omega") {
		return true;
	}
	else {
		return false;
	}
}

bool Monster_RPG_3_Spell_Interface::can_cast_in_menus(std::string name)
{
	if (name == "Heal" || name == "Cure" || name == "Heal Plus" || name == "Heal Omega") {
		return true;
	}
	else {
		return false;
	}
}

bool Monster_RPG_3_Spell_Interface::can_multi(std::string name)
{
	if (name == "FIXME") { // None that can't at the moment
		return false;
	}
	else {
		return true;
	}
}

int Monster_RPG_3_Spell_Interface::get_cost(std::string name)
{
	// white magic
	if (name == "Heal") {
		return 5;
	}
	else if (name == "Cure") {
		return 10;
	}
	else if (name == "Heal Plus") {
		return 15;
	}
	else if (name == "Heal Omega") {
		return 40;
	}
	// black magic
	else if (name == "Fire") {
		return 5;
	}
	else if (name == "Bolt") {
		return 10;
	}
	else if (name == "Ice") {
		return 20;
	}

	return 0;
}

std::vector<int> Monster_RPG_3_Spell_Interface::use(std::string name, std::vector<wedge::Base_Stats *> stats, float scale)
{
	std::vector<int> ret;

	USE_WEAK_STRONG

	for (size_t i = 0; i < stats.size(); i++) {
		wedge::Base_Stats *target_stats = stats[i];

		if (target_stats == NULL) {
			ret.push_back(0);
			continue;
		}

		// white magic
		if (name == "Heal") {
			int amount = AMOUNT(POTION_HP);
			ret.push_back(amount);
			target_stats->hp = MIN(target_stats->fixed.max_hp, target_stats->hp + amount);
		}
		else if (name == "Cure") {
			ret.push_back(0);
			target_stats->status = wedge::STATUS_OK;
		}
		else if (name == "Heal Plus") {
			int amount = AMOUNT(POTION_PLUS_HP);
			ret.push_back(amount);
			target_stats->hp = MIN(target_stats->fixed.max_hp, target_stats->hp + amount);
		}
		else if (name == "Heal Omega") {
			int amount = AMOUNT(POTION_OMEGA_HP);
			ret.push_back(amount);
			target_stats->hp = MIN(target_stats->fixed.max_hp, target_stats->hp + amount);
		}
		// black magic
		else if (name == "Fire") {
			int amount = AMOUNT(FIRE_DAMAGE);
			if (target_stats->fixed.weakness == WEAK_STRONG_FIRE) {
				amount *= 1.5f;
				WEAK_STRONG(WS_WEAK)
			}
			if (target_stats->fixed.strength == WEAK_STRONG_FIRE) {
				amount = -amount/2;
				WEAK_STRONG(WS_STRONG)
			}
			ret.push_back(amount);
			target_stats->hp = MAX(0, target_stats->hp - amount);
		}
		else if (name == "Bolt") {
			int amount = AMOUNT(BOLT_DAMAGE);
			if (target_stats->fixed.weakness == WEAK_STRONG_BOLT) {
				amount *= 1.5f;
				WEAK_STRONG(WS_WEAK)
			}
			if (target_stats->fixed.strength == WEAK_STRONG_BOLT) {
				amount = -amount/2;
				WEAK_STRONG(WS_STRONG)
			}
			ret.push_back(amount);
			target_stats->hp = MAX(0, target_stats->hp - amount);
		}
		else if (name == "Ice") {
			int amount = AMOUNT(ICE_DAMAGE);
			if (target_stats->fixed.weakness == WEAK_STRONG_ICE) {
				amount *= 1.5f;
				WEAK_STRONG(WS_WEAK)
			}
			if (target_stats->fixed.strength == WEAK_STRONG_ICE) {
				amount = -amount/2;
				WEAK_STRONG(WS_STRONG)
			}
			ret.push_back(amount);
			target_stats->hp = MAX(0, target_stats->hp - amount);
		}
		// vampires
		else if (name == "vBolt") {
			int amount = AMOUNT(BOLT_DAMAGE*2);
			if (target_stats->fixed.weakness == WEAK_STRONG_BOLT) {
				amount *= 1.5f;
				WEAK_STRONG(WS_WEAK)
			}
			if (target_stats->fixed.strength == WEAK_STRONG_BOLT) {
				amount = -amount/2;
				WEAK_STRONG(WS_STRONG)
			}
			ret.push_back(amount);
			target_stats->hp = MAX(0, target_stats->hp - amount);
		}
		else if (name == "vFire") {
			int amount = 9999;
			ret.push_back(amount);
			target_stats->hp = 0;
		}
		else if (name == "vIce") {
			int amount = AMOUNT(ICE_DAMAGE*2);
			if (target_stats->fixed.weakness == WEAK_STRONG_ICE) {
				amount *= 1.5f;
				WEAK_STRONG(WS_WEAK)
			}
			if (target_stats->fixed.strength == WEAK_STRONG_ICE) {
				amount = -amount/2;
				WEAK_STRONG(WS_STRONG)
			}
			ret.push_back(amount);
			target_stats->hp = MAX(0, target_stats->hp - amount);
		}
	}

	WEAK_STRONG_NOTIFICATION

	return ret;
}

void Monster_RPG_3_Spell_Interface::play_sound(std::string name)
{
	std::map<std::string, audio::Sound *>::iterator it = wedge::globals->spell_sfx.find(name);

	if (it != wedge::globals->spell_sfx.end()) {
		std::pair<std::string, audio::Sound *> p = *it;
		p.second->play(false);
	}
}

std::vector<wedge::Step *> Monster_RPG_3_Spell_Interface::start_effect(std::string spell, std::vector<wedge::Battle_Entity *> targets, util::Callback callback, void *callback_data)
{
	std::vector<wedge::Step *> ret;

	std::sort(targets.begin(), targets.end(), y_sort_battle_entity);

	for (size_t i = 0; i < targets.size(); i++) {
		wedge::Battle_Entity *entity = targets[i];
		wedge::Battle_Enemy *enemy = dynamic_cast<wedge::Battle_Enemy *>(entity);
		wedge::Battle_Player *player = dynamic_cast<wedge::Battle_Player *>(entity);
		util::Point<int> entity_pos;
		if (enemy) {
			entity_pos = enemy->get_position();
		}
		else {
			entity_pos = player->get_draw_pos();
		}
		gfx::Sprite *sprite = entity->get_sprite();
		util::Point<int> topleft;
		util::Point<int> bottomright;
		util::Size<int> opaque_size;
		sprite->get_bounds(topleft, bottomright);
		topleft += targets[i]->get_spell_effect_offset();
		util::Point<int> tmp = bottomright - topleft;
		opaque_size.w = tmp.x + 1;
		opaque_size.h = tmp.y + 1;
		util::Point<int> top_middle = entity->get_decoration_offset(0, util::Point<int>(-opaque_size.w/2, 0), NULL);
		bool target_is_player = dynamic_cast<wedge::Battle_Player *>(targets[i]) != NULL;

		NEW_SYSTEM_AND_TASK(BATTLE)

		if (spell == "Fire") {
			Fire_Step *f = new Fire_Step(top_middle+util::Point<int>(0, opaque_size.h), new_task);
			ADD_STEP(f)
			ret.push_back(f);
		}
		else if (spell == "Bolt") {
			util::Point<float> end = top_middle + util::Point<int>(0, opaque_size.h);
			util::Point<float> start(end.x+60, end.y-60);
			SDL_Colour c1 = shim::palette[17];
			SDL_Colour c2 = shim::palette[18];
			SDL_Colour c3 = shim::palette[19];
			SDL_Colour c4 = shim::white;
			Bolt_Step *b = new Bolt_Step(start, end, 16, target_is_player, c1, c2, c3, c4, new_task);
			ADD_STEP(b)
			ret.push_back(b);
		}
		else if (spell == "vBolt") {
			util::Point<float> end = top_middle + util::Point<int>(0, opaque_size.h);
			util::Point<float> start(end.x+60, end.y-60);
			SDL_Colour c1 = shim::palette[27];
			SDL_Colour c2 = shim::palette[28];
			SDL_Colour c3 = shim::palette[29];
			SDL_Colour c4 = shim::white;
			vBolt_Step *v = new vBolt_Step(start, end, 16, target_is_player, c1, c2, c3, c4, new_task);
			ADD_STEP(v)
			ret.push_back(v);
		}
		else if (spell == "vFire") {
			vFire_Step *v = new vFire_Step(new_task);
			ADD_STEP(v)
			ret.push_back(v);
		}
		if (spell == "Ice") {
			Ice_Step *i = new Ice_Step(new_task);
			ADD_STEP(i)
			ret.push_back(i);
		}
		else if (spell == "vIce") {
			vIce_Step *v = new vIce_Step(top_middle+util::Point<int>(0, opaque_size.h-3), new_task);
			ADD_STEP(v)
			ret.push_back(v);
		}
		else if (spell == "Heal" || spell == "Heal Plus" || spell == "Heal Omega") {
			int w = MAX(shim::tile_size, opaque_size.w);
			int pulse = (spell == "Heal") ? 250 : (spell == "Heal Plus" ? 100 : 50);
			Heal_Step *h = new Heal_Step(top_middle-util::Point<int>(w/2, 0), util::Size<int>(w, opaque_size.h), pulse, new_task);
			ADD_STEP(h)
			ret.push_back(h);
		}
		else if (spell == "Cure") {
			int w = MAX(shim::tile_size, opaque_size.w);
			Cure_Step *c = new Cure_Step(top_middle-util::Point<int>(w/2, 0), util::Size<int>(w, opaque_size.h), new_task);
			ADD_STEP(c)
			ret.push_back(c);
		}

		ADD_STEP(new wedge::Generic_Immediate_Callback_Step(callback, callback_data, new_task))

		ADD_TASK(new_task)
		FINISH_SYSTEM(BATTLE)
	}

	return ret;
}
