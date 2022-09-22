#include "Nooskewl_Wedge/battle_enemy.h"
#include "Nooskewl_Wedge/battle_game.h"
#include "Nooskewl_Wedge/battle_player.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/stats.h"

using namespace wedge;

static void sprite_callback(void *data)
{
	Battle_Enemy *enemy = static_cast<Battle_Enemy *>(data);
	gfx::Sprite *sprite = enemy->get_sprite();
	sprite->set_animation("idle");
	enemy->sprite_done();
}

namespace wedge {

Battle_Enemy::Battle_Enemy(std::string name) :
	Battle_Entity(ENEMY, name),
	die_time(1000),
	attack_sound(NULL),
	die_sound(NULL),
	started_attack(false),
	done_sprite(false),
	attack_hits_all(false),
	boss(false),
	use_death_shader(true),
	_use_death_sound(true),
	_remove_when_dead(true),
	play_attack_sound(true)
{
}

Battle_Enemy::~Battle_Enemy()
{
	delete stats;
	delete sprite;
	delete attack_sound;
	delete die_sound;
}

bool Battle_Enemy::start()
{
	return true;
}

void Battle_Enemy::draw_enemy(SDL_Colour tint, gfx::Image *image, util::Point<float> draw_pos, float scale)
{
	bool dead = stats->hp <= 0;
	if (dead && use_death_shader) {
		util::Point<int> topleft, bottomright;
		image->get_bounds(topleft, bottomright);
		shim::current_shader = globals->enemy_die_shader;
		shim::current_shader->use();
		gfx::update_projection();
		shim::current_shader->set_float("p", MIN(1.0f, (GET_TICKS()-dead_start)/(float)die_time));
		shim::current_shader->set_float("tint_p", 0.5f);
		shim::current_shader->set_colour("tint", shim::palette[28]);
		shim::current_shader->set_int("sprite_w", (bottomright.x-topleft.x)/scale);
		shim::current_shader->set_int("sprite_h", (bottomright.y-topleft.y)/scale);
		shim::current_shader->set_int("draw_x", (draw_pos.x+topleft.x)/scale);
		shim::current_shader->set_int("draw_y", (draw_pos.y+topleft.y)/scale);
		shim::current_shader->set_int("screen_offset_x", shim::screen_offset.x);
		shim::current_shader->set_int("screen_offset_y", shim::screen_offset.y);
		shim::current_shader->set_int("screen_w", shim::real_screen_size.w);
		shim::current_shader->set_int("screen_h", shim::real_screen_size.h);
		shim::current_shader->set_float("scale", shim::scale);
	}
	image->draw_tinted(tint, draw_pos);
	if (dead && use_death_shader) {
		shim::current_shader = shim::default_shader;
		shim::current_shader->use();
		gfx::update_projection();
	}
}

void Battle_Enemy::draw_boss(SDL_Colour tint, gfx::Image *image, util::Point<float> draw_pos, float scale, int curr_frame, int num_frames)
{
	bool dead = stats->hp <= 0;
	if (dead && use_death_shader) {
		shim::current_shader = globals->boss_die_shader;
		shim::current_shader->use();
		gfx::update_projection();
		shim::current_shader->set_colour("tint", shim::palette[28]);
		shim::current_shader->set_int("sprite_w", image->size.w/scale);
		shim::current_shader->set_int("sprite_h", image->size.h/scale);
		shim::current_shader->set_int("draw_x", draw_pos.x/scale);
		shim::current_shader->set_int("draw_y", draw_pos.y/scale);
		shim::current_shader->set_int("screen_offset_x", shim::screen_offset.x);
		shim::current_shader->set_int("screen_offset_y", shim::screen_offset.y);
		shim::current_shader->set_int("screen_w", shim::real_screen_size.w);
		shim::current_shader->set_int("screen_h", shim::real_screen_size.h);
		shim::current_shader->set_int("shim_screen_w", shim::screen_size.w);
		shim::current_shader->set_int("shim_screen_h", shim::screen_size.h);
		shim::current_shader->set_float("scale", shim::scale);
		shim::current_shader->set_int("num_frames", num_frames);
		shim::current_shader->set_texture("noise", GLOBALS->noise, 1);
		float p = MIN(1.0f, (GET_TICKS()-dead_start)/(float)die_time);
		float p2 = MAX(0.0, (p - 0.5) / 0.5);
		float p3 = MAX(0.0, (p - 0.75) / 0.25);
		float p4 = 1.0 - p2;
		float per_frame = 1.0 / float(num_frames);
		float frame_offset = float(curr_frame) * per_frame;
		shim::current_shader->set_float("p", p);
		shim::current_shader->set_float("p2", p2);
		shim::current_shader->set_float("p3", p3);
		shim::current_shader->set_float("p4", p4);
		shim::current_shader->set_float("frame_offset", frame_offset);
		image->stretch_region(util::Point<float>(0.0f, 0.0f), image->size, util::Point<float>(0.0f, 0.0f), shim::screen_size);
		shim::current_shader = shim::default_shader;
		shim::current_shader->use();
		gfx::update_projection();
	}
	else {
		image->draw_tinted(tint, draw_pos);
	}
}

void Battle_Enemy::draw()
{
	if (sprite) {
		gfx::Image *image = sprite->get_current_image();
#ifdef ANDROID
		if (boss && GLOBALS->have_highp_fragment_shaders) {
#else
		if (boss) {
#endif
			draw_boss(shim::white, image, position, 1.0f, sprite->get_current_frame(), sprite->get_num_frames());
		}
		else {
			draw_enemy(shim::white, image, position, 1.0f);
		}
	}
}

bool Battle_Enemy::take_turn()
{
	return true;
}

bool Battle_Enemy::is_dead()
{
	return stats->hp <= 0 && int(GET_TICKS()-dead_start) >= die_time;
}

void Battle_Enemy::take_damage(int hp, int type, int y_offset)
{
	Battle_Entity::take_damage(hp, type, y_offset);
	if (stats->hp <= 0) {
		set_dead();
	}
}

int Battle_Enemy::get_experience()
{
	return experience;
}

int Battle_Enemy::get_gold()
{
	return gold;
}

bool Battle_Enemy::turn_attack(Battle_Player *attacked, int y_offset)
{
	if (started_attack == false) {
		if (play_attack_sound) {
			if (attack_sound) {
				attack_sound->play(false);
			}
			else {
				globals->enemy_attack->play(false);
			}
		}
		if (sprite) {
			sprite->set_animation("attack", sprite_callback, this);
		}
		else {
			done_sprite = true;
		}
		started_attack = true;
	}
	if (done_sprite) {
		started_attack = false;
		done_sprite = false;
		if (attack_hits_all) {
			std::vector<Battle_Entity *> players = BATTLE->get_players();
			for (size_t i = 0; i < players.size(); i++) {
				BATTLE->hit(this, players[i], 1.0f, y_offset);
			}
		}
		else {
			BATTLE->hit(this, attacked, 1.0f, y_offset);
		}
		return true;
	}

	return false;
}

bool Battle_Enemy::turn_attack()
{
	if (started_attack == false && attack_hits_all == false) {
		attacked = rand_living_player();
	}

	return turn_attack(attacked);
}

void Battle_Enemy::sprite_done()
{
	done_sprite = true;
}

void Battle_Enemy::set_position(util::Point<int> position)
{
	this->position = position;
}

util::Point<int> Battle_Enemy::get_position()
{
	return position;
}

void Battle_Enemy::play_die_sound()
{
	if (die_sound != NULL) {
		die_sound->play(false);
	}
	else if (_use_death_sound) {
		if (boss) {
			globals->boss_die->play(false);
		}
		else {
			globals->enemy_die->play(false);
		}
	}
}

Battle_Player *Battle_Enemy::rand_living_player()
{
	std::vector<Battle_Entity *> players = BATTLE->get_players();
	std::vector<Battle_Entity *> living;
	for (size_t i = 0; i < players.size(); i++) {
		if (players[i]->get_stats()->hp > 0) {
			living.push_back(players[i]);
		}
	}
	if (living.size() == 0) {
		return NULL;
	}
	return static_cast<Battle_Player *>(living[util::rand()%living.size()]);
}

void Battle_Enemy::set_dead()
{
	dead_start = GET_TICKS();
}

bool Battle_Enemy::remove_when_dead()
{
	return _remove_when_dead;
}

bool Battle_Enemy::use_death_sound()
{
	return _use_death_sound;
}

}
