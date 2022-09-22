#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/battle_player.h>
#include <Nooskewl_Wedge/generic_callback.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/special_number.h>
#include <Nooskewl_Wedge/stats.h>

#include "battle_game.h"
#include "enemies.h"
#include "enemy_drawing_hook.h"
#include "fire.h"
#include "globals.h"
#include "inventory.h"
#include "milestones.h"
#include "monster_rpg_3.h"
#include "spells.h"
#include "stats.h"
	
Monster_RPG_3_Battle_Enemy::Monster_RPG_3_Battle_Enemy(std::string name) :
	wedge::Battle_Enemy(name),
	off_y(0)
{
}

Monster_RPG_3_Battle_Enemy::~Monster_RPG_3_Battle_Enemy()
{
}

void Monster_RPG_3_Battle_Enemy::set_position(util::Point<int> position)
{
//printf("%p set_pos this->pos=%f,%f\n", this, this->position.x, this->position.y);
	off_y = MAX(0, (shim::screen_size.h - SCR_H) / 2);
//printf("%p off_y=%d\n", this, off_y);
	wedge::Battle_Enemy::set_position(position + util::Point<int>(0, off_y));
}

void Monster_RPG_3_Battle_Enemy::resize(util::Size<int> new_size)
{
	// Redo off_y if screen size changes
//printf("%p resize this->pos=%f,%f off_y=%d\n", this, this->position.x, this->position.y, off_y);
	util::Point<int> pos = position + util::Point<int>(0, -off_y);
	off_y = MAX(0, (new_size.h - SCR_H) / 2);
//printf("%p off_y=%d\n", this, off_y);
	position = pos + util::Point<int>(0, off_y);
}

//--

Enemy_Fiddler::Enemy_Fiddler() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1394)/* Originally: Fiddler */),
	turn_count(0)
{
}

Enemy_Fiddler::~Enemy_Fiddler()
{
	delete bottom;
}

bool Enemy_Fiddler::start()
{
	boss = true;
	die_time = 1500;

	experience = 25;
	gold = 25;
	attack_sound = new audio::MML("sfx/growl.mml");
	sprite = new gfx::Sprite("fiddler");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 150;
	stats->fixed.attack = 35;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 10);

	bottom = new gfx::Sprite("fiddler_bottom");

	return true;
}

bool Enemy_Fiddler::take_turn()
{
	if (turn_count <= 1) {
		std::vector<Battle_Entity *> players = BATTLE->get_players();
		bool ret = turn_attack(static_cast<wedge::Battle_Player *>(players[turn_count]));
		if (ret) {
			turn_count++;
		}
		return ret;
	}
	else {
		return turn_attack();
	}
}

void Enemy_Fiddler::draw()
{
	float alpha;
	bool dead = stats->hp <= 0;
	if (dead) {
		alpha = 1.0f - MIN(1.0f, (GET_TICKS()-dead_start)/(float)die_time);
	}
	else {
		alpha = 1.0f;
	}
	bottom->sync_with(sprite, true);
	gfx::Image *image = bottom->get_current_image();
	SDL_Colour tint = shim::white;
	tint.r *= alpha;
	tint.g *= alpha;
	tint.b *= alpha;
	tint.a *= alpha;
	image->draw_tinted(tint, position);

	Battle_Enemy::draw();
}

float Enemy_Fiddler::get_poison_odds()
{
	if (turn_count <= 1) {
		return 1.0f;
	}
	return 0.0f;
}

//--

Enemy_Goo::Enemy_Goo() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1395)/* Originally: Goo */)
{
}

Enemy_Goo::~Enemy_Goo()
{
}

bool Enemy_Goo::start()
{
	use_death_shader = false; // they have a kind of quick special death when splitting to too minis

	experience = 8;
	gold = 11;
	die_sound = new audio::MML("sfx/goo_die.mml");
	sprite = new gfx::Sprite("goo");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 85;
	stats->fixed.attack = 30;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 10);

	return true;
}

bool Enemy_Goo::take_turn()
{
	return turn_attack();
}

void Enemy_Goo::set_dead()
{
	Monster_RPG_3_Battle_Enemy::set_dead();

	// spawn two mini goos

	Enemy_MiniGoo *mini1 = new Enemy_MiniGoo();
	mini1->start();
	mini1->set_position(position);
	BATTLE->add_entity(mini1);
	BATTLE->add_gold(mini1->get_gold());
	BATTLE->add_experience(mini1->get_experience());
	
	Enemy_MiniGoo *mini2 = new Enemy_MiniGoo();
	mini2->start();
	mini2->set_position(position + util::Point<int>(shim::tile_size, shim::tile_size));
	BATTLE->add_entity(mini2);
	BATTLE->add_gold(mini2->get_gold());
	BATTLE->add_experience(mini2->get_experience());
}

//--

Enemy_Mushroom::Enemy_Mushroom() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1396)/* Originally: Mushroom */)
{
}

Enemy_Mushroom::~Enemy_Mushroom()
{
}

bool Enemy_Mushroom::start()
{
	experience = 30;
	gold = 35;
	sprite = new gfx::Sprite("mushroom");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 100;
	stats->fixed.attack = 40;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 10);

	return true;
}

bool Enemy_Mushroom::take_turn()
{
	return turn_attack();
}

float Enemy_Mushroom::get_poison_odds()
{
	return 1.0f/4.0f;
}

//--

Enemy_MiniGoo::Enemy_MiniGoo() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1397)/* Originally: Mini Goo */)
{
}

Enemy_MiniGoo::~Enemy_MiniGoo()
{
}

bool Enemy_MiniGoo::start()
{
	experience = 2;
	gold = 2;
	attack_sound = new audio::MML("sfx/mini_enemy_attack.mml");
	sprite = new gfx::Sprite("minigoo");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 20;
	stats->fixed.attack = 20;
	stats->fixed.defense = 5;
	stats->fixed.set_extra(LUCK, 10);

	return true;
}

bool Enemy_MiniGoo::take_turn()
{
	return turn_attack();
}

//--

int Enemy_Bloated::count;
gfx::Image *Enemy_Bloated::spit;
audio::MML *Enemy_Bloated::sound;

void Enemy_Bloated::static_start()
{
	count = 0;
}

Enemy_Bloated::Enemy_Bloated() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1398)/* Originally: Bloated */),
	turn_started(false)
{
}

Enemy_Bloated::~Enemy_Bloated()
{
	count--;
	if (count == 0) {
		delete spit;
		delete sound;
	}
}

bool Enemy_Bloated::start()
{
	if (count == 0) {
		spit = new gfx::Image("battle/spit.tga");
		sound = new audio::MML("sfx/spit.mml");
	}
	count++;

	experience = 8;
	gold = 8;
	sprite = new gfx::Sprite("bloated");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 50;
	stats->fixed.attack = 30;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 10);

	return true;
}

bool Enemy_Bloated::take_turn()
{
	if (turn_started == false) {
		turn_started = true;
		spit_start = GET_TICKS();
		spit_target = rand_living_player();
		BATTLE->set_indicator(GLOBALS->game_t->translate(1399)/* Originally: Spit */, true);
		sound->play(false);
	}

	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - spit_start;

	spit_p = elapsed / (float)SPIT_TIME;

	if (elapsed >= SPIT_TIME) {
		BATTLE->hit(this, spit_target, 1.0f);
		turn_started = false;
		return true;
	}

	return false;
}

void Enemy_Bloated::draw_fore()
{
	if (turn_started && spit_p <= 1.0f) {
		float p;
		if (spit_p <= 0.5f) {
			p = spit_p / 0.5f;
		}
		else {
			p = (0.5f - (spit_p - 0.5f)) / 0.5f;
		}
		p = sin(p*M_PI/2.0f);
		float yinc = p * 24;

		util::Point<float> start = get_position() + util::Point<int>(10, 6)/* mouth pos */;
		util::Point<float> end = spit_target->get_draw_pos() + spit_target->get_sprite()->get_current_image()->size / 2;
		util::Point<float> diff = end - start;
		util::Point<float> pos = start + (diff * spit_p);
		pos.y -= yinc;

		spit->draw(pos, spit_p > 0.5f ? gfx::Image::FLIP_V : 0);
	}
}

//--

Enemy_Treant::Enemy_Treant() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1400)/* Originally: Treant */),
	action(NONE),
	casting_acorn(false)
{
}

Enemy_Treant::~Enemy_Treant()
{
	delete acorn;
	delete acorn_sound;
}

bool Enemy_Treant::start()
{
	experience = 20;
	gold = 20;
	sprite = new gfx::Sprite("treant");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 120;
	stats->fixed.attack = 45;
	stats->fixed.defense = 5;
	stats->fixed.set_extra(LUCK, 10);
	stats->fixed.weakness = WEAK_STRONG_FIRE;

	acorn = new gfx::Image("battle/acorn.tga");
	acorn_sound = new audio::MML("sfx/acorn.mml");

	return true;
}

bool Enemy_Treant::take_turn()
{
	if (action == NONE) {
		if (util::rand(0, 1) == 0) {
			action = ATTACK;
		}
		else {
			action = ACORN;
			BATTLE->set_indicator(GLOBALS->game_t->translate(1401)/* Originally: Acorn */, true);
			sprite->set_animation("cast");
		}
	}

	bool ret;

	if (action == ATTACK) {
		ret = turn_attack();
	}
	else {
		ret = turn_acorn();
	}

	if (ret == true) {
		action = NONE;
	}

	return ret;
}

bool Enemy_Treant::turn_acorn()
{
	if (casting_acorn == false) {
		casting_acorn = true;
		acorn_target = rand_living_player();
		acorn_start_time = GET_TICKS();
		acorn_stage = 0;
		acorn_y = INITIAL_ACORN_HEIGHT;
		acorn_sound->play(false);
		return false;
	}
	else {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - acorn_start_time;
		if (acorn_stage == 0) {
			if (elapsed >= ACORN_FALL_TIME) {
				acorn_stage++;
			}
		}
		else if (acorn_stage == 1) {
			if (elapsed >= ACORN_FALL_TIME+ACORN_BOUNCE_TIME) {
				BATTLE->hit(this, acorn_target, 1.25f);
				acorn_stage++;
			}
		}
		else {
			if (elapsed >= ACORN_TOTAL_TIME) {
				casting_acorn = false;
				sprite->set_animation("idle");
				return true;
			}
		}
		util::Point<float> player_pos = acorn_target->get_draw_pos();
		if (acorn_stage == 0) {
			float p = MIN(1.0f, (float)elapsed/ACORN_FALL_TIME);
			p = p * p;
			acorn_y = INITIAL_ACORN_HEIGHT + p * (player_pos.y-INITIAL_ACORN_HEIGHT);
		}
		else if (acorn_stage == 1) {
			float p = MIN(1.0f, (float)(elapsed-ACORN_FALL_TIME)/ACORN_BOUNCE_TIME);
			p = p * p;
			acorn_y = player_pos.y - p * shim::tile_size;
		}
		else {
			float p = MIN(1.0f, (float)(elapsed-ACORN_FALL_TIME-ACORN_BOUNCE_TIME)/ACORN_FINAL_FALL_TIME);
			p = p * p;
			acorn_y = (player_pos.y - shim::tile_size) + p * shim::tile_size * 2;
		}
		return false;
	}
}

void Enemy_Treant::draw_fore()
{
	if (casting_acorn) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - acorn_start_time;
		float angle;
		if (elapsed >= ACORN_FALL_TIME) {
			angle = (GET_TICKS() % 250)/250.0f * M_PI * 2.0f;
		}
		else {
			angle = (GET_TICKS() % 350)/350.0f * M_PI * 2.0f;
		}
		util::Point<float> player_pos = acorn_target->get_draw_pos();
		Uint8 alpha;
		if (elapsed >= ACORN_FALL_TIME+ACORN_BOUNCE_TIME) {
			float p = MIN(1.0f, (float)(elapsed-ACORN_FALL_TIME-ACORN_BOUNCE_TIME)/ACORN_FINAL_FALL_TIME);
			alpha = 255 * (1.0f - p);
		}
		else {
			alpha = 255;
		}
		SDL_Colour colour = { alpha, alpha, alpha, alpha };
		gfx::Image *player_image = acorn_target->get_sprite()->get_current_image();
		acorn->draw_tinted_rotated(colour, util::Point<float>(acorn->size.w/2.0f, acorn->size.h/2.0f), util::Point<float>(player_pos.x+player_image->size.w/2.0f, acorn_y), angle, 0);
	}
}

//--

int Enemy_Sludge::count;
gfx::Sprite *Enemy_Sludge::gunk;
audio::MML *Enemy_Sludge::sound;

void Enemy_Sludge::static_start()
{
	count = 0;
}

Enemy_Sludge::Enemy_Sludge() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1402)/* Originally: Sludge */),
	turn_started(false)
{
}

Enemy_Sludge::~Enemy_Sludge()
{
	count--;
	if (count == 0) {
		delete gunk;
		delete sound;
	}
}

bool Enemy_Sludge::start()
{
	if (count == 0) {
		gunk = new gfx::Sprite("gunk");
		sound = new audio::MML("sfx/poison.mml");
	}
	count++;

	experience = 25;
	gold = 20;
	sprite = new gfx::Sprite("sludge");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 100;
	stats->fixed.attack = 40;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 15);

	return true;
}

bool Enemy_Sludge::take_turn()
{
	if (turn_started == false) {
		turn_started = true;
		sprite->set_animation("attack");
		BATTLE->set_indicator(GLOBALS->game_t->translate(1403)/* Originally: Gunk */, true);
	}
	else {
		if (sprite->get_animation() == "attack") {
			if (sprite->is_finished()) {
				sprite->set_animation("idle");
				sludge_start = GET_TICKS();
				sludge_target = rand_living_player();
				sound->play(false);
			}
		}
		else {
			Uint32 now = GET_TICKS();
			Uint32 elapsed = now - sludge_start;

			sludge_p = elapsed / (float)SLUDGE_TIME;

			if (elapsed >= SLUDGE_TIME) {
				BATTLE->hit(this, sludge_target, 1.0f);
				turn_started = false;
				return true;
			}
		}
	}

	return false;
}

void Enemy_Sludge::draw_fore()
{
	if (turn_started && sprite->get_animation() == "idle" && sludge_p <= 1.0f) {
		float p;
		if (sludge_p <= 0.5f) {
			p = sludge_p / 0.5f;
		}
		else {
			p = (0.5f - (sludge_p - 0.5f)) / 0.5f;
		}
		p = sin(p*M_PI/2.0f);
		float yinc = p * 24;

		util::Point<float> start = get_position() + util::Point<int>(11, 8)/* gunk start pos */;
		util::Point<float> end = sludge_target->get_draw_pos() + sludge_target->get_sprite()->get_current_image()->size / 2;
		util::Point<float> diff = end - start;
		util::Point<float> pos = start + (diff * sludge_p);
		pos -= gunk->get_current_image()->size/2;
		pos.y -= yinc;

		gunk->get_current_image()->draw(pos, 0);
	}
}

float Enemy_Sludge::get_poison_odds()
{
	return 1.0f/30.0f;
}

//--

int Enemy_Does_Creepy::count;
gfx::Image *Enemy_Does_Creepy::creep_images[2];
audio::MML *Enemy_Does_Creepy::sound;

Enemy_Does_Creepy::Enemy_Does_Creepy(std::string name) :
	Monster_RPG_3_Battle_Enemy(name)
{
	if (count == 0) {
		creep_images[0] = new gfx::Image("battle/creep1.tga");
		creep_images[1] = new gfx::Image("battle/creep2.tga");
		sound = new audio::MML("sfx/creepy.mml");
	}
	count++;
}

Enemy_Does_Creepy::~Enemy_Does_Creepy()
{
	count--;
	if (count == 0) {
		delete creep_images[0];
		delete creep_images[1];
		delete sound;
	}
}

bool Enemy_Does_Creepy::turn_creepy()
{
	if (creeps.size() == 0) {
		std::vector<Battle_Entity *> players = BATTLE->get_players();
		living.clear();
		for (size_t i = 0; i < players.size(); i++) {
			if (players[i]->get_stats()->hp > 0) {
				living.push_back(players[i]);
			}
		}
		for (size_t i = 0; i < living.size(); i++) {
			for (int j = 0; j < NUM_CREEPS; j++) {
				Creep c;
				c.target = living[i];
				c.prev_drift = util::Point<float>(0, 0);
				c.drift = new_drift();
				c.image_index = util::rand(0, 1);
				creeps.push_back(c);
			}
		}
		creepy_start_time = GET_TICKS();
		last_drift = creepy_start_time;
		sound->play(false);
		return false;
	}
	else {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - creepy_start_time;
		int drift_time = CREEP_TIME / DRIFTS;
		Uint32 elapsed2 = now - last_drift;
		if ((int)elapsed2 > drift_time) {
			for (size_t i = 0; i < creeps.size(); i++) {
				if (creep_visible((int)i)) {
					creeps[i].prev_drift = creeps[i].drift;
					creeps[i].drift = new_drift();
				}
			}
			last_drift = now;
		}
		if (sprite->get_animation() == "creepy" && sprite->is_finished()) {
			sprite->set_animation("idle");
		}
		if (elapsed >= CREEP_FULL_TIME) {
			for (size_t i = 0; i < living.size(); i++) {
				BATTLE->hit(this, living[i], creepy_mult * (3 - living.size()));
			}
			creeps.clear();
			return true;
		}
		return false;
	}
}

util::Point<float> Enemy_Does_Creepy::new_drift()
{
	util::Point<float> p;
	p.x = (int)util::rand(0, MAX_DRIFT*2) - MAX_DRIFT;
	p.y = (int)util::rand(0, MAX_DRIFT*2) - MAX_DRIFT;
	return p;
}

float Enemy_Does_Creepy::creep_p(int index)
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - creepy_start_time;
	if (elapsed < LAUGH_TIME) {
		return -1.0f;
	}
	int order = index >= NUM_CREEPS ? index - NUM_CREEPS : index; // top half would be a new set if there are two living players
	elapsed -= LAUGH_TIME;
	int add_time = order * ADD_TIME;
	if ((int)elapsed < add_time) {
		return -1.0f;
	}
	elapsed -= add_time;
	if (elapsed > CREEP_TIME) {
		return 2.0f;
	}
	return elapsed / (float)CREEP_TIME;
}

bool Enemy_Does_Creepy::creep_visible(int index)
{
	float p = creep_p(index);
	return p >= 0.0f && p <= 1.0f;
}

util::Point<float> Enemy_Does_Creepy::creep_pos(int index)
{
	util::Point<int> topleft, bottomright;
	gfx::Image *img = sprite->get_current_image();
	img->get_bounds(topleft, bottomright);
	int sprite_w = (bottomright.x - topleft.x) * 1.1f;
	util::Size<int> sprite_size(sprite_w, img->size.h);
	util::Point<int> start_pos = get_decoration_offset(0, util::Point<float>(0.0f, 0.0f) + sprite_size/2, NULL);
	util::Point<int> turn_pos = creeps[index].target->get_decoration_offset(0, util::Point<int>(-5, 0) + creeps[index].target->get_sprite()->get_current_image()->size/2, NULL);
	std::vector<Battle_Entity *> players = BATTLE->get_players();
	bool top = players[0] == creeps[index].target;
	util::Point<int> end_pos;
	if (top) {
		end_pos = turn_pos + util::Point<int>(40, -20);
	}
	else {
		end_pos = turn_pos + util::Point<int>(40, 20);
	}
	util::Point<int> d1 = turn_pos - start_pos;
	util::Point<int> d2 = end_pos - turn_pos;
	float l1 = d1.length();
	float l2 = d2.length();
	float p1 = l1 / (l1 + l2);
	float p = creep_p(index);
	if (p <= p1) {
		p = p / p1;
		return start_pos + util::Point<int>(d1.x * p, d1.y * p);
	}
	else {
		p = (p - p1) / (1.0f - p1);
		return turn_pos + util::Point<int>(d2.x * p, d2.y * p);
	}
}

util::Point<float> Enemy_Does_Creepy::get_drift(int index)
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - last_drift;
	int drift_time = CREEP_TIME / DRIFTS;
	float p = elapsed / (float)drift_time;
	if (p > 1.0f) {
		p = 1.0f;
	}
	util::Point<float> d = creeps[index].drift - creeps[index].prev_drift;
	return creeps[index].prev_drift + util::Point<float>(d.x * p, d.y * p);
}

SDL_Colour Enemy_Does_Creepy::get_tint(int index)
{
	float p = creep_p(index);
	if (p <= 0.1f) {
		p = p / 0.1f;
	}
	else if (p >= 0.6f) {
		p = 1.0f - ((p - 0.6f) / 0.4f);
	}
	else {
		return shim::white;
	}

	SDL_Colour colour;
	colour.r = p * 255;
	colour.g = p * 255;
	colour.b = p * 255;
	colour.a = p * 255;
	return colour;
}


void Enemy_Does_Creepy::static_start()
{
	count = 0;
}

void Enemy_Does_Creepy::start_creepy()
{
	BATTLE->set_indicator(GLOBALS->game_t->translate(1405)/* Originally: Creepy */, true);
	sprite->set_animation("creepy");
}

void Enemy_Does_Creepy::draw_creepy()
{
	for (size_t i = 0; i < creeps.size(); i++) {
		if (creep_visible((int)i)) {
			util::Point<float> pos = creep_pos((int)i);
			pos += get_drift((int)i);
			gfx::Image *img = creep_images[creeps[i].image_index];
			img->draw_tinted(get_tint((int)i), pos - img->size/2);
		}
	}
}

//--

Enemy_Ghastly::Enemy_Ghastly() :
	Enemy_Does_Creepy(GLOBALS->game_t->translate(1404)/* Originally: Ghastly */),
	action(NONE)
{
	creepy_mult = 0.9f;
}

Enemy_Ghastly::~Enemy_Ghastly()
{
}

bool Enemy_Ghastly::start()
{
	experience = 30;
	gold = 25;
	sprite = new gfx::Sprite("ghastly");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 130;
	stats->fixed.attack = 45;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 15);

	return true;
}

bool Enemy_Ghastly::take_turn()
{
	if (action == NONE) {
		if (util::rand(0, 1) == 0) {
			action = ATTACK;
		}
		else {
			action = CREEPY;
			start_creepy();
		}
	}

	bool ret;

	if (action == ATTACK) {
		ret = turn_attack();
	}
	else {
		ret = turn_creepy();
	}

	if (ret == true) {
		action = NONE;
	}

	return ret;
}

void Enemy_Ghastly::draw_fore()
{
	if (action == CREEPY) {
		draw_creepy();
	}
}

//--

int Enemy_Werewolf::count;
audio::Sound *Enemy_Werewolf::bite_sound;

void Enemy_Werewolf::static_start()
{
	count = 0;
}

Enemy_Werewolf::Enemy_Werewolf() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1406)/* Originally: Werewolf */),
	turn_started(false)
{
}

Enemy_Werewolf::~Enemy_Werewolf()
{
	count--;
	if (count == 0) {
		delete bite_sound;
	}
}

bool Enemy_Werewolf::start()
{
	if (count == 0) {
		bite_sound = new audio::MML("sfx/growl.mml");
	}
	count++;

	experience = 40;
	gold = 30;
	sprite = new gfx::Sprite("werewolf");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 180;
	stats->fixed.attack = 55;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 15);

	return true;
}

bool Enemy_Werewolf::take_turn()
{
	if (turn_started == false) {
		turn_started = true;
		if (util::rand(0, 2) == 0) {
			punching = false;
			sprite->set_animation("bite");
			BATTLE->set_indicator(GLOBALS->game_t->translate(1407)/* Originally: Bite */, true);
			bite_sound->play(false);
		}
		else {
			punching = true;
		}
	}

	bool ret;

	if (punching) {
		ret = turn_attack();
	}
	else {
		if (sprite->is_finished()) {
			sprite->set_animation("idle");
			wedge::Battle_Entity *target = rand_living_player();
			BATTLE->hit(this, target, 1.25f);
			ret = true;
		}
		else {
			ret = false;
		}
	}

	if (ret == true) {
		turn_started = false;
	}

	return ret;
}

//--

int Enemy_Knightly::count;
gfx::Image *Enemy_Knightly::head_image;
gfx::Sprite *Enemy_Knightly::head_spin_sprite;
audio::Sound *Enemy_Knightly::head_spin_sound;
audio::Sound *Enemy_Knightly::throw_sound;

void Enemy_Knightly::static_start()
{
	count = 0;
}

Enemy_Knightly::Enemy_Knightly() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1408)/* Originally: Knightly */),
	has_head(true),
	attacking(false),
	head_returning(false),
	throw_started(false)
{
}

Enemy_Knightly::~Enemy_Knightly()
{
	count--;
	if (count == 0) {
		delete head_image;
		delete head_spin_sprite;
		delete head_spin_sound;
		delete throw_sound;
	}
}

bool Enemy_Knightly::start()
{
	if (count == 0) {
		head_image = new gfx::Image("battle/knightly_head.tga");
		head_spin_sprite = new gfx::Sprite("knightly_head_spin");
		head_spin_sound = new audio::MML("sfx/knightly_head_spin.mml");
		throw_sound = new audio::MML("sfx/throw.mml");
	}
	count++;

	experience = 45;
	gold = 40;
	sprite = new gfx::Sprite("knightly");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 175;
	stats->fixed.attack = 55;
	stats->fixed.defense = INT_MAX; // gets changed after he throws his head
	stats->fixed.set_extra(LUCK, 25);

	return true;
}

bool Enemy_Knightly::take_turn()
{
	if (head_returning) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - head_return_start;
		if ((int)elapsed >= HEAD_RETURN_TIME) {
			head_returning = false;
			start_attack();
		}
	}

	if (attacking) {
		if (sprite->get_animation() == "throw" && sprite->get_current_frame() == 2 && throw_started == false) {
			start_throw();
		}
		else if (sprite->get_animation() == "throw" && sprite->is_finished()) {
			if (throw_started == false) {
				start_throw();
			}
			throw_started = false;
			sprite->set_animation("sit");
		}
		else if (sprite->get_animation() == "sit") {
			Uint32 now = GET_TICKS();
			Uint32 elapsed = now - throw_start;
			if ((int)elapsed >= THROW_TIME) {
				BATTLE->hit(this, target, 1.0f);
				attacking = false;
				return true;
			}
		}
	}
	else {
		if (has_head) {
			start_attack();
		}
		else if (head_returning == false) {
			head_returning = true;
			head_return_start = GET_TICKS();
			head_spin_sound->play(false);
			head_spin_sprite->reset();
		}
	}

	return false;
}

void Enemy_Knightly::draw()
{
	Monster_RPG_3_Battle_Enemy::draw();

	if (attacking) {
		bool draw_head;
		if (sprite->get_animation() == "throw") {
			int frame = sprite->get_current_frame();
			if (frame <= 1) {
				draw_head = false;
				if (frame == 1) {
					head_image->draw_rotated(util::Point<int>(head_image->size.w/2, head_image->size.h/2), position + util::Point<int>(1, 7), float(-M_PI/4.0f), 0);
				}
			}
			else {
				draw_head = true;
			}
		}
		else {
			draw_head = true;
		}

		if (draw_head) {
			Uint32 now = GET_TICKS();
			Uint32 elapsed = now - throw_start;
			if ((int)elapsed > THROW_TIME) {
				elapsed = THROW_TIME;
			}
			float p = elapsed / (float)THROW_TIME;
			util::Point<float> diff = throw_end_pos - throw_start_pos;
			util::Point<float> pos = throw_start_pos + util::Point<float>(diff.x * p, diff.y * p);
			float angle = fmodf(p, 0.2f) / 0.2f * M_PI * 2.0f;
			head_image->draw_rotated(util::Point<int>(head_image->size.w/2, head_image->size.h/2), pos, -angle, 0);
		}
	}
	else if (head_returning) {
		const int start_y = -50;
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - head_return_start;
		if ((int)elapsed > HEAD_RETURN_TIME) {
			elapsed = HEAD_RETURN_TIME;
		}
		float p = (float)elapsed / HEAD_RETURN_TIME;
		float y = start_y * (1.0f - p);
		head_spin_sprite->get_current_image()->draw(position + util::Point<int>(8, 4+y));
	}
}

void Enemy_Knightly::start_attack()
{
	attacking = true;
	sprite->set_animation("throw");
	has_head = false;
	stats->fixed.defense = DEFENSE;
}

void Enemy_Knightly::start_throw()
{
	throw_start = GET_TICKS();
	throw_start_pos = position + util::Point<int>(23, 10);
	target = rand_living_player();
	util::Point<int> target_pos = target->get_decoration_offset(0, util::Point<int>(0, 0), NULL);
	util::Point<int> diff = target_pos - throw_start_pos;
	float angle = diff.angle();
	const int dist = 150;
	throw_end_pos = util::Point<int>(throw_start_pos.x + cos(angle) * dist, throw_start_pos.y + sin(angle) * dist);
	throw_started = true;
	throw_sound->play(false);
}

//--

Enemy_Palla::Enemy_Palla() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(91)/* Originally: Palla */),
	turn_type(NONE),
	turn(0)
{
}

Enemy_Palla::~Enemy_Palla()
{
	delete throw_sprite;
	delete throw_sound;
	delete vampire_sound;
	delete vampire_small_sound;
	delete bats_sound;
	delete vamp_sprite;
	delete vamp_small_sprite;
}

bool Enemy_Palla::start()
{
	boss = true;
	die_time = (2.0f/0.05f)*(1000.0f/shim::logic_rate); // 0.05f is basic walk speed, player walks 1 tile (here it's 2 to give extra time)
	use_death_shader = false;
	_use_death_sound = false;
	_remove_when_dead = false;

	throw_sprite = new gfx::Sprite("palla_sword");
	throw_sound = new audio::MML("sfx/throw.mml");
	vampire_sound = new audio::MML("sfx/vampire.mml");
	vampire_small_sound = new audio::MML("sfx/vampire_small.mml");
	bats_sound = new audio::MML("sfx/bats.mml");
	vamp_sprite = new gfx::Sprite("vamp");
	vamp_small_sprite = new gfx::Sprite("vamp_small");

	experience = 1000;
	gold = 1000;
	sprite = new gfx::Sprite("palla_battle");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 1250;
	stats->fixed.attack = 90;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 20);
	
	spell_effect_offset = util::Point<float>(12, 0); // His sword otherwise makes spells hit between him and his sword

	return true;
}

bool Enemy_Palla::take_turn()
{
	if (turn_type == NONE) {
		int t = turn % 3;
		if (t == 0 || t == 2) {
			turn_type = THROW;
			target = rand_living_player();
			sprite->set_animation("throw");
			throw_sprite->reset();
			throw_start = 0;
			hit = false;
			BATTLE->set_indicator(GLOBALS->game_t->translate(1410)/* Originally: Throw */, true);
		}
		else {
			int r = util::rand(0, 1);
			if (r == 0) {
				turn_type = VAMPIRE;
				target = rand_living_player();
				sprite->set_animation("point");
				vamp_sprite->reset();
				BATTLE->set_indicator(GLOBALS->game_t->translate(1350)/* Originally: Vampire */, true);
				vampire_sound->play(false);
			}
			else {
				turn_type = BATS;
				const int nbats = 4;
				std::vector<Battle_Entity *> players = BATTLE->get_players();
				std::vector<bool> living;
				int alive_count = 0;
				for (size_t i = 0; i < players.size(); i++) {
					bool alive = players[i]->get_stats()->hp > 0;
					living.push_back(alive);
					if (alive) {
						alive_count++;
					}
				}
				int p1 = 0;
				int p2 = 0;
				for (int i = 0; i < nbats; i++) {
					Bat b;
					if (alive_count == 2) {
						b.player1 = util::rand(0, 1) == 0;
					}
					else if (living[0]) {
						b.player1 = true;
					}
					else {
						b.player1 = false;
					}
					b.delay = (b.player1 ? p1 : p2) * BITE_TIME + i * 100;
					b.bit = false;
					b.sprite = new gfx::Sprite("bat");
					b.start_y = (int)util::rand(0, 20) - 10;
					b.stage = -1;
					bats.push_back(b);
					if (b.player1) {
						p1++;
					}
					else {
						p2++;
					}
				}
				sprite->set_animation("point");
				BATTLE->set_indicator(GLOBALS->game_t->translate(1412)/* Originally: Bats */, true);
				bats_sound->play(true);
			}
		}
		turn++;
		turn_start = GET_TICKS();
	}

	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - turn_start;
	
	if (turn_type == THROW) {
		if (hit == false && throw_start != 0 && (int)elapsed >= int(THROW_TIME-(throw_start-turn_start))) {
			hit = true;
			BATTLE->hit(this, target, 1.0f);
		}
		if (elapsed >= THROW_TIME) {
			turn_type = NONE;
			set_idle();
			return true;
		}
	}
	else if (turn_type == VAMPIRE) {
		if (elapsed >= VAMPIRE_TIME) {
			turn_type = NONE;
			int amount = BATTLE->hit(this, target, 0.75f);
			SDL_Colour colour, shadow_colour;
			colour = shim::palette[13];
			shadow_colour = shim::palette[27];
			std::string text = util::itos(amount);
			util::Point<int> number_pos = get_decoration_offset(shim::font->get_text_width(text), util::Point<int>(shim::tile_size*3/4, 0), NULL);
			NEW_SYSTEM_AND_TASK(BATTLE)
			wedge::Special_Number_Step *step = new wedge::Special_Number_Step(colour, shadow_colour, text, number_pos, wedge::Special_Number_Step::RISE, new_task);
			ADD_STEP(step)
			wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, BATTLE, new_task);
			step->add_monitor(g);
			ADD_STEP(g)
			ADD_TASK(new_task)
			FINISH_SYSTEM(BATTLE)
			BATTLE->inc_waiting_for_hit(1);
			stats->hp += amount;
			stats->hp = MIN(stats->fixed.max_hp, stats->hp);
			set_idle();
			return true;
		}
	}
	else {
		int biggest = 0;
		for (size_t i = 0; i < bats.size(); i++) {
			if ((int)bats[i].delay > biggest) {
				biggest = bats[i].delay;
			}
		}
		if ((int)elapsed >= BAT_TIME+biggest) {
			turn_type = NONE;
			int p1 = 0;
			int p2 = 0;
			for (size_t i = 0; i < bats.size(); i++) {
				if (bats[i].player1) {
					p1++;
				}
				else {
					p2++;
				}
			}
			for (size_t i = 0; i < bats.size(); i++) {
				delete bats[i].sprite;
			}
			bats.clear();
			std::vector<Battle_Entity *> players = BATTLE->get_players();
			int amount = 0;
			if (p1 > 0) {
				amount += BATTLE->hit(this, players[0], 0.333f*p1);
			}
			if (p2 > 0) {
				amount += BATTLE->hit(this, players[1], 0.333f*p2);
			}
			SDL_Colour colour, shadow_colour;
			colour = shim::palette[13];
			shadow_colour = shim::palette[27];
			std::string text = util::itos(amount);
			util::Point<int> number_pos = get_decoration_offset(shim::font->get_text_width(text), util::Point<int>(shim::tile_size*3/4, 0), NULL);
			NEW_SYSTEM_AND_TASK(BATTLE)
			wedge::Special_Number_Step *step = new wedge::Special_Number_Step(colour, shadow_colour, text, number_pos, wedge::Special_Number_Step::RISE, new_task);
			ADD_STEP(step)
			wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, BATTLE, new_task);
			step->add_monitor(g);
			ADD_STEP(g)
			ADD_TASK(new_task)
			FINISH_SYSTEM(BATTLE)
			BATTLE->inc_waiting_for_hit(1);
			stats->hp += amount;
			stats->hp = MIN(stats->fixed.max_hp, stats->hp);
			bats_sound->stop();
			set_idle();
			return true;
		}
	}

	return false;
}

void Enemy_Palla::draw_fore()
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - turn_start;

	if (turn_type == THROW) {
		if (throw_start == 0) {
			int frame = sprite->get_current_frame();
			if (frame == sprite->get_num_frames()-1) {
				throw_start = now;
				throw_sound->play(false);
			}
		}
		if (throw_start == 0) {
			return;
		}
		elapsed -= (throw_start-turn_start);
		util::Point<int> my_pos = position + util::Point<int>(41, 33); // hands pos
		util::Point<int> player_pos = static_cast<wedge::Battle_Player *>(target)->get_draw_pos() + target->get_sprite()->get_current_image()->size/2;
		float p = elapsed/(float)(THROW_TIME-(throw_start-turn_start));
		if (p > 1.0f) {
			p = 1.0f;
		}
		float save_p = p;
		if (p <= 0.5f) {
			p /= 0.5f;
		}
		else {
			p = 1.0f - ((p - 0.5f) / 0.5f);
		}
		float h = fabsf((float)sin(((0.5f - fabsf(p-0.5f))) * M_PI)) * 15;
		util::Point<float> diff = player_pos - my_pos;
		float len = diff.length();
		float angle = diff.angle();
		util::Point<float> pos(my_pos.x+cos(angle)*p*len, my_pos.y+sin(angle)*p*len);
		if (save_p <= 0.5f) {
			angle += float(M_PI/2);
		}
		else {
			angle -= float(M_PI/2);
		}
		pos.x += cos(angle)*h;
		pos.y += sin(angle)*h;
		gfx::Image *img = throw_sprite->get_current_image();
		img->draw(pos-img->size/2);
	}
	else if (turn_type == VAMPIRE) {
		gfx::Image *img = vamp_sprite->get_current_image();
		util::Point<int> pos(shim::screen_size.w/2-img->size.w/2, shim::screen_size.h/4-img->size.h/2);
		img->draw(pos);
	}
	else if (turn_type == BATS) {
		for (size_t i = 0; i < bats.size(); i++) {
			if (elapsed < bats[i].delay) {
				continue;
			}
			Uint32 e = elapsed - bats[i].delay;
			util::Point<int> start_pos(-shim::tile_size, position.y+sprite->get_current_image()->size.h/2+bats[i].start_y);
			std::vector<Battle_Entity *> players = BATTLE->get_players();
			Battle_Entity *target = bats[i].player1 ? players[0] : players[1];
			util::Point<int> topleft, bottomright;
			target->get_sprite()->get_bounds(topleft, bottomright);
			util::Point<int> dest_pos = target->get_decoration_offset(0, util::Point<float>(0.0f, 0.0f) + util::Point<int>(topleft.x, -bats[i].sprite->get_current_image()->size.h+1+bats[i].start_y/5), NULL);
			float p;
			if (e <= (BAT_TIME-BITE_TIME)/2) {
				bats[i].stage = 0;
				p = e/(float)((BAT_TIME-BITE_TIME)/2);
			}
			else if (e <= (BAT_TIME-BITE_TIME)/2+BITE_TIME) {
				if (bats[i].stage == 0) {
					if (bats[i].sprite->get_animation() != "bite") {
						bats[i].sprite->set_animation("bite");
					}
					vampire_small_sound->play(false);
				}
				bats[i].stage = 1;
				p = (e-((BAT_TIME-BITE_TIME)/2))/(float)BITE_TIME;
			}
			else {
				if (bats[i].stage == 1) {
					if (bats[i].sprite->get_animation() != "fly") {
						bats[i].sprite->set_animation("fly");
					}
				}
				bats[i].stage = 2;
				p = 1.0f - (e-((BAT_TIME-BITE_TIME)/2+BITE_TIME))/(float)((BAT_TIME-BITE_TIME)/2);
			}
			gfx::Image *img = bats[i].sprite->get_current_image();
			if (bats[i].stage == 0 || bats[i].stage == 2) {
				int flags = bats[i].stage == 2 ? gfx::Image::FLIP_H : 0;
				util::Point<float> diff = dest_pos - start_pos;
				util::Point<float> pos = start_pos + util::Point<float>(diff.x * p, diff.y * p + sin(p*M_PI*2*4) * 5);
				img->draw(pos, flags);
			}
			else {
				img->draw(dest_pos);
				gfx::Image *vamp = vamp_small_sprite->get_current_image();
				vamp->draw(dest_pos+util::Point<int>(img->size.w/2-vamp->size.w/2, img->size.h+1));
			}
		}
	}
}

void Enemy_Palla::set_idle()
{
	sprite->set_animation("idle");
}

//--

Enemy_Tentacle::Enemy_Tentacle() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1413)/* Originally: Tentacle */)
{
}

Enemy_Tentacle::~Enemy_Tentacle()
{
}

bool Enemy_Tentacle::start()
{
	experience = 60;
	gold = 60;
	attack_sound = new audio::MML("sfx/growl.mml");
	sprite = new gfx::Sprite("tentacle");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 400;
	stats->fixed.attack = 100;
	stats->fixed.defense = 25;
	stats->fixed.set_extra(LUCK, 30);
	stats->fixed.weakness = WEAK_STRONG_BOLT;

	return true;
}

bool Enemy_Tentacle::take_turn()
{
	return turn_attack();
}

//--

Enemy_Wave::Enemy_Wave() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1414)/* Originally: Wave */),
	played_splash(false)
{
}

Enemy_Wave::~Enemy_Wave()
{
	delete deck_water;

	delete splash;
}

bool Enemy_Wave::start()
{
	attack_hits_all = true;

	deck_water = new gfx::Sprite("deck_water");

	splash = new audio::MML("sfx/big_splash.mml");

	experience = 200;
	gold = 175;
	attack_sound = new audio::MML("sfx/wave.mml");
	sprite = new gfx::Sprite("wave");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 1000;
	stats->fixed.attack = 115;
	stats->fixed.defense = 25;
	stats->fixed.set_extra(LUCK, 30);
	stats->fixed.weakness = WEAK_STRONG_BOLT;

	return true;
}

bool Enemy_Wave::take_turn()
{
	if (started_attack == false) {
		played_splash = false;
		BATTLE->set_indicator(GLOBALS->game_t->translate(1415)/* Originally: Tidal */, true);
	}
	return turn_attack();
}

void Enemy_Wave::draw_back()
{
	if (started_attack) {
		if (sprite->get_animation() == "attack") {
			int frame = sprite->get_current_frame();
			const int start_frame = 6;
			const int num_frames = 5;
			if (frame >= start_frame && frame <= (start_frame+num_frames-1)) {
				if (played_splash == false) {
					played_splash = true;
					splash->play(false);
				}
				int water_frame = frame - start_frame;
				gfx::Image *img = deck_water->get_image(water_frame);
				img->draw(BATTLE->get_offset());
			}
		}
	}
}

//--

int Enemy_Shocker::count;
gfx::Image **Enemy_Shocker::prerendered;

void Enemy_Shocker::static_start()
{
	count = 0;
	prerendered = 0;
}

void Enemy_Shocker::spell_effect_callback(void *data)
{
	Enemy_Shocker *e = static_cast<Enemy_Shocker *>(data);
	e->set_spell_effect_done(true);
}

Enemy_Shocker::Enemy_Shocker() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1416)/* Originally: Shocker */),
	turn_started(false),
	spell_effect_done(false)
{
}

Enemy_Shocker::~Enemy_Shocker()
{
	lost_device();
	delete shocker_ring;
}

void Enemy_Shocker::lost_device()
{
	if (count > 0) {
		count--;
		if (count == 0) {
			delete_prerendered();
		}
	}
}

void Enemy_Shocker::found_device()
{
	if (count == 0) {
		prerender();
	}
	count++;
}

bool Enemy_Shocker::start()
{
	experience = 75;
	gold = 85;
	sprite = new gfx::Sprite("shocker");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 230;
	stats->fixed.attack = 90;
	stats->fixed.defense = 30;
	stats->fixed.set_extra(LUCK, 30);
	stats->fixed.strength = WEAK_STRONG_BOLT;

	shocker_ring = new gfx::Sprite("shocker_ring");

	found_device();

	return true;
}

bool Enemy_Shocker::take_turn()
{
	if (turn_started == false) {
		turn_started = true;
		attacking = util::rand(0, 1) == 0;
		if (attacking == false) {
			target = rand_living_player();
			std::vector<wedge::Battle_Entity *> spell_targets;
			spell_targets.push_back(target);
			SPELLS->play_sound("Bolt");
			BATTLE->set_indicator(GLOBALS->game_t->translate(6)/* Originally: Bolt */, true);
			SPELLS->start_effect("Bolt", spell_targets, spell_effect_callback, this);
			sprite->set_animation("attack");
		}
	}
	else if (attacking == false) {
		if (spell_effect_done) {
			spell_effect_done = false;
			BATTLE->hit(this, target, 1.333f);
			turn_started = false;
			sprite->set_animation("idle");
			return true;
		}
	}

	if (attacking) {
		bool ret = turn_attack();
		if (ret == true) {
			turn_started = false;
		}
		return ret;
	}

	return false;
}

void Enemy_Shocker::draw_back()
{
	gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, 1.0f);
	gfx::update_projection();

	int index = get_index();

	bool dead = stats->hp <= 0;

	util::Point<float> draw_pos = position;
	draw_pos *= shim::scale;

	if (dead) {
		draw_enemy(shim::white, prerendered[index], draw_pos, shim::scale);
	}
	else {
		float a = util::rand(0, 1000) / 1000.0f * 0.25f + 0.75f;
		SDL_Colour colour = shim::white;
		colour.r *= a;
		colour.g *= a;
		colour.b *= a;
		colour.a *= a;
		draw_enemy(colour, prerendered[index], draw_pos, shim::scale);
	}

	gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
	gfx::update_projection();
}

int Enemy_Shocker::get_index()
{
	int nblurs = util::rand(MIN_BLURS, MAX_BLURS);
	int n = nblurs - 3;
	int ring_frames = shocker_ring->get_num_frames();
	int index = n * ring_frames;
	index += shocker_ring->get_current_frame();
	return index;
}

void Enemy_Shocker::prerender()
{
	allocate_prerendered();
	util::Size<int> size = (util::Size<float>)shocker_ring->get_current_image()->size * shim::scale;

	gfx::Image *img_src = new gfx::Image(size);
	gfx::Image *img_dest = new gfx::Image(size);

	gfx::Shader *tmp_shader = NULL;

	const int ring_frames = shocker_ring->get_num_frames();

	gfx::Image *old_target = gfx::get_target_image();

	int index = 0;

	for (int NLOOPS = MIN_BLURS; NLOOPS <= MAX_BLURS; NLOOPS++) {
		for (int frame = 0; frame < ring_frames; frame++) {
			gfx::Image *target = prerendered[index];

			for (int loops = 0; loops < NLOOPS; loops++) {
				if (loops == 0) {
					gfx::set_target_image(img_src);
					gfx::clear(shim::transparent);
					gfx::set_target_image(img_dest);
					gfx::clear(shim::transparent);
					gfx::set_default_projection(size, util::Point<int>(0, 0), shim::scale);
					gfx::update_projection();
				}

				if (loops == 0) {
					shocker_ring->get_image(frame)->draw(util::Point<int>(0, 0));
				}

				if (loops < NLOOPS-1) {
					if (loops == 0) {
						tmp_shader = shim::current_shader;
						shim::current_shader = M3_GLOBALS->blur_shader;
						shim::current_shader->use();
						gfx::update_projection();
					}

					gfx::Image *tmp = img_dest;
					img_dest = img_src;
					img_src = tmp;

					gfx::set_target_image(img_dest);

					const float OFFSET_INC_X = (shim::scale / NLOOPS) / img_src->size.w;
					const float OFFSET_INC_Y = (shim::scale / NLOOPS) / img_src->size.h;
					float offset_x = OFFSET_INC_X * (loops+1);
					float offset_y = OFFSET_INC_Y * (loops+1);
					shim::current_shader->set_float("offset_x", offset_x);
					shim::current_shader->set_float("offset_y", offset_y);

					img_src->draw(util::Point<int>(0, 0));
				}

				if (loops == NLOOPS-1) {
					gfx::set_target_image(target);

					gfx::clear(shim::transparent);

					shim::current_shader = tmp_shader;
					shim::current_shader->use();
					gfx::update_projection();

					img_dest->draw(util::Point<int>(0, 0));
				}
			}

			util::Point<int> topleft, bottomright;
			shocker_ring->get_image(frame)->get_bounds(topleft, bottomright);
			target->set_bounds(util::Point<float>(topleft.x*shim::scale, topleft.y*shim::scale), util::Point<float>(bottomright.x*shim::scale, bottomright.y*shim::scale));

			index++;
		}
	}

	gfx::set_target_image(old_target);

	delete img_src;
	delete img_dest;
}

int Enemy_Shocker::get_num_prerendered_images()
{
	int diff = (MAX_BLURS - MIN_BLURS) + 1;
	return shocker_ring->get_num_frames() * diff;
}

void Enemy_Shocker::delete_prerendered()
{
	int n = get_num_prerendered_images();
	for (int i = 0; i < n; i++) {
		delete prerendered[i];
	}
	delete[] prerendered;
	prerendered = 0;
}

void Enemy_Shocker::allocate_prerendered()
{
	int n = get_num_prerendered_images();
	prerendered = new gfx::Image *[n];
	util::Size<int> size = (util::Size<float>)shocker_ring->get_current_image()->size * shim::scale;
	for (int i = 0; i < n; i++) {
		prerendered[i] = new gfx::Image(size);
	}
}

void Enemy_Shocker::set_spell_effect_done(bool done)
{
	spell_effect_done = done;
}

//--

static void splash_callback(void *data)
{
	Enemy_Monster *e = static_cast<Enemy_Monster *>(data);
	e->set_splash_showing(false);
}

Enemy_Monster::Enemy_Monster() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1356)/* Originally: Monster */),
	turn_type(NONE),
	turn(0)
{
}

Enemy_Monster::~Enemy_Monster()
{
	whirlpool_sound->stop();

	delete screech;
	delete monster_sound;
	delete whirlpool_sound;
	delete whirlpool;
	delete splash;
	delete top;
}

bool Enemy_Monster::start()
{
	boss = true;
	die_time = 1500;

	screech = new audio::Sample("screech.ogg");
	monster_sound = new audio::MML("sfx/monster.mml");
	attack_sound = new audio::MML("sfx/monster.mml");
	whirlpool_sound = new audio::MML("sfx/whirlpool.mml");
	whirlpool = new gfx::Sprite("whirlpool");
	splash = new gfx::Sprite("splash");

	experience = 2500;
	gold = 2500;
	sprite = new gfx::Sprite("monster");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 5000;
	stats->fixed.attack = 140;
	stats->fixed.defense = 10;
	stats->fixed.set_extra(LUCK, 30);
	stats->fixed.weakness = WEAK_STRONG_BOLT;

	top = new gfx::Sprite("monster_top");

	spell_effect_offset = util::Point<float>(30, 0); // Monster is started at -30 x (off the left of the screen)

	return true;
}

bool Enemy_Monster::take_turn()
{
	if (turn_type == NONE) {
		int t = turn % 2;
		if (t == 0) {
			int r = turn == 6 ? 1/*screech on the 7th turn*/ : util::rand(0, 1);
			if (turn < 6 || r == 0) {
				turn_type = WHIRLPOOL;
				splashed = 0;
				splashed_gfx = 0;
				target = rand_living_player();
				whirlpool_sound->play(true);
				monster_sound->play(false);
				sprite->set_animation("whirlpool");
				BATTLE->set_indicator(GLOBALS->game_t->translate(1420)/* Originally: Whirlpool */, true);
				splash_showing = false;
			}
			else {
				turn_type = SCREECH;
				screech->play(false);
				sprite->set_animation("screech");
				BATTLE->set_indicator(GLOBALS->game_t->translate(1421)/* Originally: Screech */, true);

				wind.clear();
				for (size_t i = 0; i < NUM_WIND; i++) {
					Wind w;
					w.start_pos.x = util::rand(shim::tile_size*3, shim::tile_size*8);
					w.start_pos.y = util::rand(position.y, position.y + sprite->get_current_image()->size.h);
					w.len = 0.0f;
					w.x = 0.0f;
					wind.push_back(w);
				}
			}
		}
		else {
			turn_type = ATTACK;
		}
		turn++;
		turn_start = GET_TICKS();
	}

	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - turn_start;
	
	if (turn_type == ATTACK) {
		bool ret = turn_attack();
		if (ret == true) {
			turn_type = NONE;
		}
		return ret;
	}
	else if (turn_type == WHIRLPOOL) {
		if (splashed == 0) {
			splashed++;
			M3_GLOBALS->splash->play(false);
		}
		else if (splashed == 1 && elapsed >= WHIRLPOOL_RAISE+WHIRLPOOL_STATIONARY) {
			splashed++;
			M3_GLOBALS->splash->play(false);
		}

		if (elapsed >= WHIRLPOOL_RAISE+WHIRLPOOL_STATIONARY+WHIRLPOOL_JUMP+WHIRLPOOL_CIRCLE+WHIRLPOOL_LEAVE) {
			whirlpool_sound->stop();
			BATTLE->hit(this, target, 1.05f);
			turn_type = NONE;
			sprite->set_animation("idle");
			return true;
		}
	}
	else {
		if (elapsed >= SCREECH_TIME) {
			std::vector<Battle_Entity *> players = BATTLE->get_players();
			for (size_t i = 0; i < players.size(); i++) {
				if (players[i]->get_stats()->hp > 0) {
					BATTLE->hit(this, players[i], 1.25f);
				}
			}
			turn_type = NONE;
			sprite->set_animation("idle");
			return true;
		}
		else {
			for (size_t i = 0; i < NUM_WIND; i++) {
				Wind &w = wind[i];
				w.len += 1.5f;
				w.x += 0.25f;
			}
		}
	}

	return false;
}

void Enemy_Monster::draw_fore()
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - turn_start;

	if (turn_type == WHIRLPOOL) {
		util::Point<int> topleft;
		util::Point<int> bottomright;
		sprite->get_bounds(topleft, bottomright);
		util::Point<int> whirlpool_start_pos(
			bottomright.x + position.x + shim::tile_size/4,
			shim::tile_size/2+bottomright.y
		);
		gfx::Image *img = whirlpool->get_current_image();
		util::Point<float> target_pos = target->get_draw_pos();
		float dest_angle = float(M_PI / 4.0f);
		int dest_x = target_pos.x + shim::tile_size/2 + cos(dest_angle) * CIRCLE_RADIUS;
		int start_mid_x = whirlpool_start_pos.x + img->size.w/2;
		util::Point<float> centre = target_pos + util::Point<int>(shim::tile_size/2, shim::tile_size-2);
		if (elapsed < WHIRLPOOL_RAISE+WHIRLPOOL_STATIONARY) {
			float p = (float)elapsed / WHIRLPOOL_RAISE;
			if (p > 1.0f) {
				p = 1.0f;
			}
			int pix = MAX(1, p * img->size.h);
			const int num_changes = 4;
			pix /= img->size.h / num_changes;
			pix *= img->size.h / num_changes;
			img->stretch_region(util::Point<int>(0, 0), img->size, whirlpool_start_pos-util::Point<int>(0, pix), util::Size<int>(img->size.w, pix), 0);

			if (splashed_gfx == 0) {
				splashed_gfx++;
				splash_showing = true;
				splash->set_animation("only", splash_callback, this);
				splash->reset();
				splash->start();
			}
		}
		else if (elapsed < WHIRLPOOL_RAISE+WHIRLPOOL_STATIONARY+WHIRLPOOL_JUMP) {
			Uint32 e = elapsed - (WHIRLPOOL_RAISE+WHIRLPOOL_STATIONARY);
			float p = (float)e / WHIRLPOOL_JUMP;
			int half_w = (dest_x - start_mid_x) / 2;
			int half_h = p < 0.5f ? shim::tile_size * 1.5f : (shim::tile_size * 1.5f) - (whirlpool_start_pos.y - (target_pos.y+shim::tile_size)) + sin(dest_angle) * CIRCLE_RADIUS;
			int half_x = (dest_x - start_mid_x) / 2 + start_mid_x;
			int pivot_y = p < 0.5f ? whirlpool_start_pos.y : whirlpool_start_pos.y - (shim::tile_size * 1.5f) + half_h;
			float angle;
			if (p < 0.5f) {
				p = p / 0.5f;
				angle = M_PI + p * M_PI / 2.0f;
			}
			else {
				p = (p - 0.5f) / 0.5f;
				angle = M_PI * 1.5f + p * M_PI / 2.0f;
			}
			util::Point<float> draw_pos;
			draw_pos.x = half_x + cos(angle) * half_w;
			draw_pos.y = pivot_y + sin(angle) * half_h;
			img->draw(draw_pos + util::Point<int>(-img->size.w/2, -img->size.h));

			if (splashed_gfx == 1) {
				splashed_gfx++;
				splash_showing = true;
				splash->set_animation("only", splash_callback, this);
				splash->reset();
				splash->start();
			}
		}
		else if (elapsed < WHIRLPOOL_RAISE+WHIRLPOOL_STATIONARY+WHIRLPOOL_JUMP+WHIRLPOOL_CIRCLE) {
			Uint32 e = elapsed - (WHIRLPOOL_RAISE + WHIRLPOOL_STATIONARY + WHIRLPOOL_JUMP);
			float p = (float)e / WHIRLPOOL_CIRCLE;
			float angle = M_PI / 4.0f + p * M_PI * 4.0f;
			util::Point<float> draw_pos = centre + util::Point<float>(cos(angle) * CIRCLE_RADIUS, sin(angle) * CIRCLE_RADIUS);
			img->draw(draw_pos + util::Point<int>(-img->size.w/2, -img->size.h));
		}
		else {
			Uint32 e = elapsed - (WHIRLPOOL_RAISE+WHIRLPOOL_STATIONARY+WHIRLPOOL_JUMP+WHIRLPOOL_CIRCLE);
			float p = (float)e / WHIRLPOOL_LEAVE;
			util::Point<float> start = centre + util::Point<float>(cos(M_PI/4.0f)*CIRCLE_RADIUS, sin(M_PI/4.0f)*CIRCLE_RADIUS);
			util::Point<float> dest = start + util::Point<int>(shim::tile_size*4, 0);
			util::Point<float> diff = dest - start;
			util::Point<float> draw_pos = start + util::Point<float>(diff.x*p, diff.y*p);
			img->draw(draw_pos + util::Point<int>(-img->size.w/2, -img->size.h));
		}

		if (splash_showing) {
			gfx::Image *splash_img = splash->get_current_image();
			splash_img->draw(whirlpool_start_pos+util::Point<int>(img->size.w/2-splash_img->size.w/2, -splash_img->size.h));
		}
	}
	else if (turn_type == SCREECH) {
		float p = (float)elapsed / SCREECH_TIME;
		if (p > 1.0f) {
			p = 1.0f;
		}
		float alpha;
		if (p < 0.5f) {
			p /= 0.5f;
			alpha = p * 0.5f;
		}
		else {
			p = (p - 0.5f) / 0.5f;
			alpha = (0.5f) - p * 0.5f;
		}
		SDL_Colour colour = shim::white;
		colour.r *= alpha;
		colour.g *= alpha;
		colour.b *= alpha;
		colour.a *= alpha;
		for (size_t i = 0; i < NUM_WIND; i++) {
			Wind &w = wind[i];
			float x = w.start_pos.x + w.x;
			gfx::draw_line(colour, util::Point<float>(x, w.start_pos.y), util::Point<float>(x + w.len, w.start_pos.y), 1.0f);
		}
	}
}

void Enemy_Monster::draw()
{
	Battle_Enemy::draw();

	float alpha;
	bool dead = stats->hp <= 0;
	if (dead) {
		alpha = 1.0f - MIN(1.0f, (GET_TICKS()-dead_start)/(float)die_time);
	}
	else {
		alpha = 1.0f;
	}
	top->sync_with(sprite, true);
	gfx::Image *image = top->get_current_image();
	SDL_Colour tint = shim::white;
	tint.r *= alpha;
	tint.g *= alpha;
	tint.b *= alpha;
	tint.a *= alpha;
	image->draw_tinted(tint, position);
}

void Enemy_Monster::set_splash_showing(bool showing)
{
	splash_showing = showing;
}

//--

void Enemy_Sandworm::hide_done_callback(void *data)
{
	Enemy_Sandworm *worm = static_cast<Enemy_Sandworm *>(data);
	worm->get_sprite()->set_animation("idle");
	worm->set_hide_done(true);
}

Enemy_Sandworm::Enemy_Sandworm() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1422)/* Originally: Sandworm */),
	turn_started(false)
{
}

Enemy_Sandworm::~Enemy_Sandworm()
{
}

bool Enemy_Sandworm::start()
{
	experience = 100;
	gold = 100;
	sprite = new gfx::Sprite("sandworm");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 800;
	stats->fixed.attack = 160;
	stats->fixed.defense = 50;
	stats->fixed.set_extra(LUCK, 30);
	stats->fixed.weakness = WEAK_STRONG_ICE;
	stats->fixed.strength = WEAK_STRONG_FIRE;

	return true;
}

bool Enemy_Sandworm::take_turn()
{
	if (turn_started == false) {
		turn_started = true;
		hiding = util::rand(0, 2) == 0;
		if (hiding) {
			BATTLE->set_indicator(GLOBALS->game_t->translate(1423)/* Originally: Hide */, true);
			hide_done = false;
			played_sound = false;
			sprite->set_animation("hide", hide_done_callback, this);
			GLOBALS->enemy_attack->play(false);
			hide_start = GET_TICKS();
		}
	}

	if (hiding) {
		if (played_sound == false && GET_TICKS() > hide_start+3000) {
			played_sound = true;
			GLOBALS->enemy_attack->play(false);
		}
		if (hide_done) {
			const int amount = 500;
			stats->hp += amount;
			SDL_Colour colour = shim::palette[13];
			SDL_Colour shadow_colour = shim::palette[27];
			std::string text = util::itos(abs(amount));
			util::Point<int> number_pos = get_decoration_offset(shim::font->get_text_width(text), util::Point<int>(shim::tile_size*3/4, 0), NULL);
			NEW_SYSTEM_AND_TASK(BATTLE)
			wedge::Special_Number_Step *step = new wedge::Special_Number_Step(colour, shadow_colour, text, number_pos, wedge::Special_Number_Step::RISE, new_task);
			ADD_STEP(step)
			wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, BATTLE, new_task);
			step->add_monitor(g);
			ADD_STEP(g)
			ADD_TASK(new_task)
			FINISH_SYSTEM(BATTLE)
			BATTLE->inc_waiting_for_hit(1);

			M3_GLOBALS->item_sfx[ITEM_POTION_PLUS]->play(false);

			turn_started = false;

			return true;
		}
	}
	else {
		bool ret = turn_attack();
		if (ret) {
			turn_started = false;
		}
		return ret;
	}

	return false;
}

void Enemy_Sandworm::set_hide_done(bool done)
{
	hide_done = done;
}

//--

void Enemy_Flare::spell_effect_callback(void *data)
{
	Enemy_Flare *e = static_cast<Enemy_Flare *>(data);
	e->set_spell_effect_done();
}

Enemy_Flare::Enemy_Flare() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1424)/* Originally: Flare */),
	spell_effect_done_count(0)
{
}

Enemy_Flare::~Enemy_Flare()
{
	delete bottom;
}

bool Enemy_Flare::start()
{
	experience = 120;
	gold = 90;
	sprite = new gfx::Sprite("flare");
	bottom = new gfx::Sprite("flare_bottom");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 550;
	stats->fixed.attack = 150;
	stats->fixed.defense = 40;
	stats->fixed.set_extra(LUCK, 30);
	stats->fixed.weakness = WEAK_STRONG_ICE;
	stats->fixed.strength = WEAK_STRONG_FIRE;

	start_ticks =  GET_TICKS();

	return true;
}

bool Enemy_Flare::take_turn()
{
	if (targets.size() == 0) {
		std::vector<Battle_Entity *> players = BATTLE->get_players();
		int r = util::rand(0, 2);
		if (r == 2) {
			bool all_alive = true;
			for (size_t i = 0; i < players.size(); i++) {
				if (players[i]->get_stats()->hp <= 0) {
					all_alive = false;
					break;
				}
			}
			if (all_alive) {
				targets.insert(targets.begin(), players.begin(), players.end());
			}
			else {
				targets.push_back(rand_living_player());
			}
		}
		else {
			targets.push_back(rand_living_player()); // don't insert players[r] here, could be dead
		}
		SPELLS->play_sound("Fire");
		BATTLE->set_indicator(GLOBALS->game_t->translate(5)/* Originally: Fire */, true);
		SPELLS->start_effect("Fire", targets, spell_effect_callback, this);
	}
	else {
		if (spell_effect_done_count == (int)targets.size()) {
			spell_effect_done_count = 0;
			float damage = 1.0f + 0.1f * targets.size();
			for (size_t i = 0; i < targets.size(); i++) {
				BATTLE->hit(this, targets[i], damage / targets.size());
			}
			targets.clear();
			return true;
		}
	}

	return false;
}

void Enemy_Flare::draw_back()
{
	Monster_RPG_3_Battle_Game *game = dynamic_cast<Monster_RPG_3_Battle_Game *>(BATTLE);

	if (game == NULL) {
		return;
	}

#ifdef ANDROID
	if (GLOBALS->have_highp_fragment_shaders)
#endif
	{
		gfx::Image *bg = game->get_background();

		util::Point<int> offset = game->get_offset();

		util::Size<int> heat_size(14+10, 40);
		util::Point<float> heat_pos = (position + util::Point<int>(-5, 13)/*down to even middle pixel*/) - util::Point<int>(0, heat_size.h);
		if (heat_pos.y < 0) {
			heat_size.h += heat_pos.y;
			heat_pos.y = 0;
		}

		util::Point<int> bg_pos = heat_pos - offset;

		Uint32 elapsed = GET_TICKS() - start_ticks;
		float t = elapsed / 1000.0f;

		gfx::Shader *bak_shader = shim::current_shader;
		shim::current_shader = M3_GLOBALS->heat_wave_shader;
		shim::current_shader->use();
		gfx::update_projection();
		shim::current_shader->set_float("screen_offset_x", shim::screen_offset.x);
		shim::current_shader->set_float("screen_offset_y", shim::screen_offset.y);
		shim::current_shader->set_float("real_screen_h", shim::real_screen_size.h);
		shim::current_shader->set_float("scale", shim::scale);
		shim::current_shader->set_float("screen_x", heat_pos.x);
		shim::current_shader->set_float("screen_y", heat_pos.y);
		shim::current_shader->set_float("screen_w", shim::screen_size.w);
		shim::current_shader->set_float("screen_h", shim::screen_size.h);
		shim::current_shader->set_float("heat_w", heat_size.w);
		shim::current_shader->set_float("heat_h", heat_size.h);
		shim::current_shader->set_float("t", t);
		shim::current_shader->set_float("inv_tex_w", 1.0f/bg->size.w);
		shim::current_shader->set_float("inv_tex_h", 1.0f/bg->size.h);
		shim::current_shader->set_float("wave_size", 1.5f);

		bg->draw_region(bg_pos, heat_size, heat_pos);

		shim::current_shader = bak_shader;
		shim::current_shader->use();
		gfx::update_projection();
	}
	
	bottom->sync_with(sprite);

	SDL_Colour tint = shim::white;
	tint.r *= 0.5f;
	tint.g *= 0.5f;
	tint.b *= 0.5f;
	tint.a *= 0.5f;
	Uint32 u = GET_TICKS() % 300;
	float f;
	if (u < 150) {
		f = u / 150.0f;
	}
	else {
		f = 1.0f - ((u - 150) / 150.0f);
	}
	f = 0.5f + (0.5f * f);
	tint.r *= f;
	tint.g *= f;
	tint.b *= f;
	tint.a *= f;
	draw_enemy(tint, bottom->get_current_image(), position-util::Point<int>(2, 10), 1.0f);
}

void Enemy_Flare::set_spell_effect_done()
{
	spell_effect_done_count++;
}

//--

int Enemy_Cyclone::count;
audio::Sound *Enemy_Cyclone::loop_sound;

void Enemy_Cyclone::static_start()
{
	count = 0;
	loop_sound = NULL;
}

Enemy_Cyclone::Enemy_Cyclone() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1427)/* Originally: Cyclone */),
	taking_turn(false),
	played_sound(false),
	did_hit(false)
{
}

Enemy_Cyclone::~Enemy_Cyclone()
{
	count--;

	if (count == 0) {
		delete loop_sound;
		loop_sound = NULL;
	}
}

bool Enemy_Cyclone::start()
{
	experience = 90;
	gold = 125;
	sprite = new gfx::Sprite("cyclone");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 600;
	stats->fixed.attack = 140;
	stats->fixed.defense = 50;
	stats->fixed.set_extra(LUCK, 40);
	stats->fixed.weakness = WEAK_STRONG_ICE;
	stats->fixed.strength = WEAK_STRONG_FIRE;

	attack_sound = new audio::MML("sfx/stirupdust.mml");

	if (count == 0) {
		loop_sound = new audio::MML("sfx/cyclone.mml");
		loop_sound->play(true);
	}
	count++;

	return true;
}

bool Enemy_Cyclone::take_turn()
{
	gfx::Image *img = sprite->get_current_image();

	if (taking_turn == false) {
		taking_turn = true;
		turn_start = GET_TICKS();
		pixels.clear();
		for (int i = 0; i < NUM_PIXELS; i++) {
			Pixel p;
			p.angle = util::rand(0, 1000)/1000.0f * M_PI * 2.0f;
			p.dist = util::rand(MIN_DUST_DIST, MAX_DUST_DIST);
			p.start_time = util::rand(THERE, THERE+CIRCLE);
			p.colour = util::rand(0, 1) == 0 ? shim::palette[4] : shim::palette[31];
			pixels.push_back(p);
		}
		target = rand_living_player();
		wedge::Battle_Player *player = static_cast<wedge::Battle_Player *>(target);
		target_pos = player->get_draw_pos() + util::Point<float>(shim::tile_size/2.0f, shim::tile_size);
		start_pos = position + util::Point<float>(img->size.w/2.0f, img->size.h);
		GLOBALS->enemy_attack->play(false);
	}
	else {
		Uint32 elapsed = GET_TICKS() - turn_start;
		if (elapsed > THERE + CIRCLE + BACK) {
			taking_turn = false;
			played_sound = false;
			did_hit = false;
			position = start_pos - util::Point<float>(img->size.w/2.0f, img->size.h);
			return true;
		}
		else {
			if (elapsed < THERE) {
				float p = elapsed / (float)THERE;
				util::Point<float> diff(target_pos.x-start_pos.x-CIRCLE_RADIUS, target_pos.y-start_pos.y);
				util::Point<float> straight = start_pos + diff * p;
				straight.y += sin(p * M_PI) * 10;
				position = straight;
			}
			else if (elapsed < THERE + CIRCLE) {
				if (played_sound == false) {
					attack_sound->play(false);
					played_sound = true;
				}
				float p = (elapsed-THERE) / (float)CIRCLE;
				float x = cos(p*M_PI*5.0f/*2.5 circles, ends of far side*/+M_PI) * CIRCLE_RADIUS + target_pos.x;
				float y = sin(p*M_PI*5.0f/*2.5 circles, ends on far side*/+M_PI) * CIRCLE_RADIUS + target_pos.y;
				position = util::Point<float>(x, y);
			}
			else {
				if (did_hit == false) {
					did_hit = true;
					if (BATTLE->hit(this, target, 1.0f) > 0) {
						wedge::Base_Stats *stats = target->get_stats();
						if (stats->hp > 0 && util::rand(0, 1) == 0) {
							stats->status = STATUS_BLIND;
							gfx::add_notification(GLOBALS->game_t->translate(1428)/* Originally: Blinded! */);
						}
					}
				}
				float p = 1.0f - ((elapsed-(THERE+CIRCLE)) / (float)BACK);
				util::Point<float> diff(target_pos.x-start_pos.x+/*plus, circle ends on far side*/CIRCLE_RADIUS, target_pos.y-start_pos.y);
				util::Point<float> straight = start_pos + diff * p;
				straight.y += sin(p * M_PI) * 10;
				position = straight;
			}

			position -= util::Point<float>(img->size.w/2.0f, img->size.h);
		}
	}

	return false;
}

void Enemy_Cyclone::draw()
{
	if (taking_turn == false) {
		Battle_Enemy::draw();
	}
}

void Enemy_Cyclone::draw_fore()
{
	if (taking_turn == false) {
		return;
	}
		
	gfx::Image *img = sprite->get_current_image();

	draw_enemy(shim::white, img, position, 1.0f);

	Uint32 elapsed = GET_TICKS() - turn_start;

	gfx::draw_primitives_start();
		
	for (size_t i = 0; i < pixels.size(); i++) {
		Pixel &p = pixels[i];
		if (p.start_time > elapsed) {
			continue;
		}

		Uint32 t = elapsed - p.start_time;

		Uint32 life = p.dist / (float)MAX_DUST_DIST * MAX_DUST_LIFE;

		if (t > life) {
			continue;
		}

		float f = t / (float)life;

		util::Point<float> pos = target_pos;
		pos.x += cos(p.angle) * p.dist * f;
		pos.y += sin(p.angle) * p.dist * f - sin(f * M_PI) * MIN_DUST_DIST;

		gfx::draw_filled_rectangle(p.colour, pos, util::Size<int>(1, 1));
	}

	gfx::draw_primitives_end();
}

//--

int Enemy_Bones::count;
gfx::Image *Enemy_Bones::big_bone;
gfx::Image *Enemy_Bones::small_bone;
audio::Sound *Enemy_Bones::throw_sound;

void Enemy_Bones::static_start()
{
	count = 0;
}

Enemy_Bones::Enemy_Bones() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1429)/* Originally: Bones */),
	turn_started(false)
{
	if (count == 0) {
		big_bone = new gfx::Image("misc/big_bone.tga");
		small_bone = new gfx::Image("misc/small_bone.tga");
		throw_sound = new audio::MML("sfx/throw.mml");
	}
	count++;
}

Enemy_Bones::~Enemy_Bones()
{
	count--;
	if (count == 0) {
		delete big_bone;
		delete small_bone;
		delete throw_sound;
	}
}

bool Enemy_Bones::start()
{
	experience = 105;
	gold = 95;
	sprite = new gfx::Sprite("bones");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 650;
	stats->fixed.attack = 150;
	stats->fixed.defense = 40;
	stats->fixed.set_extra(LUCK, 30);

	big_bone_start_pos = util::Point<int>(11, 0);
	small_bone_start_pos = util::Point<int>(2, 4);

	return true;
}

bool Enemy_Bones::take_turn()
{
	if (turn_started == false) {
		turn_started = true;
		big = util::rand(0, 1) == 0;
		hit = false;
		threw = false;
		start_time = GET_TICKS();
		target = rand_living_player();
		wedge::Battle_Player *player = static_cast<wedge::Battle_Player *>(target);
		util::Point<float> pos = player->get_draw_pos();
		dest_pos = pos + util::Point<int>(shim::tile_size/2 - (big ? big_bone->size.h/2 : small_bone->size.h/2), 0);
		stage = 0;
		GLOBALS->enemy_attack->play(false);
	}

	Uint32 elapsed = GET_TICKS() - start_time;

	if (elapsed > RAISE && stage == 0) {
		if (threw == false) {
			threw = true;
			throw_sound->play(false);
		}
		stage++;
	}
	else if (elapsed > RAISE + THERE && stage == 1) {
		if (hit == false) {
			hit = true;
			BATTLE->hit(this, target, big ? 1.25f : 1.0f);
		}
		stage++;
	}
	else if (elapsed > RAISE + THERE + BACK && stage == 2) {
		turn_started = false;
		return true;
	}

	return false;
}

void Enemy_Bones::draw_fore()
{
	if (turn_started) {
		Uint32 elapsed = GET_TICKS() - start_time;
		if (big) {
			small_bone->draw(position+small_bone_start_pos);
		}
		else {
			big_bone->draw(position+big_bone_start_pos);
		}
		const float raise_amount = 8.0f;
		if (elapsed < RAISE) {
			float p = elapsed / (float)RAISE;
			float p2 = fmodf(p, 1.0f);
			if (p2 > 0.5f) {
				p2 -= 0.5f;
				p2 /= 0.5f;
				if (p2 > 0.5f) {
					p2 = 1.0f - ((p2 - 0.5f) / 0.5f);
				}
				else {
					p2 /= 0.5f;
				}
				p2 = -p2;
			}
			else {
				p2 /= 0.5f;
				if (p2 > 0.5f) {
					p2 = 1.0f - ((p2 - 0.5f) / 0.5f);
				}
				else {
					p2 /= 0.5f;
				}
			}
			float y = -raise_amount * p;
			if (big) {
				big_bone->draw_rotated(util::Point<float>(big_bone->size.w/2.0f, big_bone->size.h/2.0f), position + big_bone_start_pos + util::Point<float>(big_bone->size.w/2.0f, big_bone->size.h/2.0f+y), p2, 0);
			}
			else {
				small_bone->draw_rotated(util::Point<float>(small_bone->size.w/2.0f, small_bone->size.h/2.0f), position + small_bone_start_pos + util::Point<float>(small_bone->size.w/2.0f, small_bone->size.h/2.0f+y), p2, 0);
			}
		}
		else {
			elapsed -= RAISE;
			float p = elapsed / (float)(THERE + BACK);
			float angle = p * M_PI * 2.0f * ROTATIONS;
			util::Point<float> start_pos;
			util::Point<float> end_pos;
			if (big) {
				if (elapsed > THERE) {
					start_pos = dest_pos;
					end_pos = position + big_bone_start_pos + util::Point<float>(big_bone->size.w/2.0f, big_bone->size.h/2.0f);
				}
				else {
					start_pos = position + big_bone_start_pos + util::Point<float>(0, -raise_amount) + util::Point<float>(big_bone->size.w/2.0f, big_bone->size.h/2.0f);
					end_pos = dest_pos;
				}
			}
			else {
				if (elapsed > THERE) {
					start_pos = dest_pos;
					end_pos = position + small_bone_start_pos + util::Point<float>(small_bone->size.w/2.0f, small_bone->size.h/2.0f);
				}
				else {
					start_pos = position + small_bone_start_pos + util::Point<float>(0, -raise_amount) + util::Point<float>(small_bone->size.w/2.0f, small_bone->size.h/2.0f);
					end_pos = dest_pos;
				}
			}
			if (elapsed > THERE) {
				p = (elapsed - THERE) / (float)BACK;
			}
			else {
				p = elapsed / (float)THERE;
			}
			util::Point<float> diff = end_pos - start_pos;
			util::Point<float> pos = start_pos + diff * p;
			if (big) {
				big_bone->draw_rotated(util::Point<float>(big_bone->size.w/2.0f, big_bone->size.h/2.0f), pos, angle, 0);
			}
			else {
				small_bone->draw_rotated(util::Point<float>(small_bone->size.w/2.0f, small_bone->size.h/2.0f), pos, angle, 0);
			}
		}
	}
	else {
		draw_enemy(shim::white, big_bone, position + big_bone_start_pos, 1.0f);
		draw_enemy(shim::white, small_bone, position + small_bone_start_pos, 1.0f);
	}
}

//--

Enemy_Does_Darkness::Enemy_Does_Darkness() :
	taking_turn(false)
{
	ghost = new gfx::Image("battle/darkness_ghost.tga");
	darkness_sfx = new audio::MML("sfx/darkness.mml");
}

Enemy_Does_Darkness::~Enemy_Does_Darkness()
{
	delete ghost;
	delete darkness_sfx;
}

void Enemy_Does_Darkness::start_darkness()
{
	turn_start = GET_TICKS();
	finished_cast = false;

	ghosts.clear();

	int total_peaks = 0;
	for (int i = 0; i < PHASES; i++) {
		if (i % 2 == 0) {
			total_peaks += PEAKS;
		}
		else {
			total_peaks += PEAKS-1;
		}
	}

	for (int i = 0; i < NUM_GHOSTS; i++) {
		Ghost g;
		g.y = util::rand(5, shim::screen_size.h-ghost->size.h-5);
		g.dist = util::rand(WIDTH, WIDTH * 2);
		g.delay = util::rand(0, DARKNESS_TIME-GHOST_TIME);
		ghosts.push_back(g);
	}

	// remove ones that are too close
	for (size_t i = 0; i < ghosts.size(); i++) {
		int index = (int)i;
		while (true) {
			Ghost &a = ghosts[index];
			bool found = false;
			for (size_t j = 0; j < ghosts.size(); j++) {
				if ((int)j == index) {
					continue;
				}
				Ghost &b = ghosts[j];
				if (std::abs((a.y+ghost->size.h/2)-(b.y+ghost->size.h/2)) < ghost->size.h*2/3 && std::abs(int(a.delay)-int(b.delay)) < GHOST_TIME/4) {
					found = true;
					if ((int)j < index) {
						index--;
					}
					ghosts.erase(ghosts.begin() + j);
					break;
				}
			}
			if (found == false) {
				break;
			}
		}
	}
}

bool Enemy_Does_Darkness::turn_darkness()
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - turn_start;
	if ((int)elapsed >= cast_time && finished_cast == false) {
		finished_cast = true;
		enemy_sprite->set_animation("idle");
		BATTLE->set_indicator(GLOBALS->game_t->translate(1655)/* Originally: Darkness */, true);
		darkness_sfx->play(false);
	}
	if (elapsed >= (Uint32)DARKNESS_TIME+cast_time) {
		std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();
		for (size_t i = 0; i < players.size(); i++) {
			if (players[i]->get_stats()->hp > 0) {
				BATTLE->hit(enemy, players[i], darkness_mult);
			}
		}
		taking_turn = false;
		return true;
	}
	return false;
}

void Enemy_Does_Darkness::draw_darkness()
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - turn_start;
	if ((int)elapsed < cast_time) {
		return;
	}
	// started this effect without the cast anim, this compensates for it
	now -= cast_time;
	elapsed -= cast_time;
	elapsed = MIN(DARKNESS_TIME, elapsed);
	Uint32 per_phase = DARKNESS_TIME / PHASES;
	int phase = elapsed / per_phase;
	int peak_h = shim::screen_size.h / (PEAKS*2-1);
	Uint32 t = elapsed - (phase * per_phase);
	float p = (float)t / per_phase;
	float p2;
	if (p < 0.5f) {
		p2 = p / 0.5f;
	}
	else {
		p2 = 1 - ((p - 0.5f) / 0.5f);
	}
	int peaks;
	int start_y;
	if (phase % 2 == 0) {
		peaks = PEAKS * 2;
		start_y = -shim::screen_size.h;
	}
	else {
		peaks = PEAKS * 2 - 1;
		start_y = -shim::screen_size.h + peak_h;
	}
	start_y += p * shim::screen_size.h;
	gfx::draw_primitives_start();
	int y = start_y;
	for (int i = 0; i < peaks; i++) {
		for (int j = 0; j < peak_h; j++) {
			float x = shim::screen_size.w-sin((float)j/(peak_h-1)*M_PI) * WIDTH * p2;
			if (y+j >= 0 && y+j < shim::screen_size.h) {
				gfx::draw_line(shim::black, util::Point<float>(x, y+j+0.5f), util::Point<float>(shim::screen_size.w, y+j+0.5f));
			}
		}
		y += peak_h * 2;
	}
	gfx::draw_primitives_end();
	ghost->start_batch();
	for (size_t i = 0; i < ghosts.size(); i++) {
		Ghost &g = ghosts[i];
		if (g.delay > elapsed || now > turn_start+g.delay+GHOST_TIME) {
			continue;
		}
		float p = (elapsed-g.delay)/(float)GHOST_TIME;
		float alpha = 1.0f - p;
		float x = shim::screen_size.w - p * g.dist;
		SDL_Colour tint = shim::white;
		tint.r *= alpha;
		tint.g *= alpha;
		tint.b *= alpha;
		tint.a *= alpha;
		ghost->draw_tinted(tint, util::Point<float>(x, g.y));
	}
	ghost->end_batch();
}

//--

Enemy_Reaper::Enemy_Reaper() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1649)/* Originally: Reaper */)
{
	darkness_mult = 0.75f;
	cast_time = CAST_TIME;
}

Enemy_Reaper::~Enemy_Reaper()
{
	delete cast;
}

bool Enemy_Reaper::start()
{
	experience = 200;
	gold = 150;
	attack_sound = new audio::MML("sfx/growl.mml");
	sprite = new gfx::Sprite("reaper");
	cast = new audio::MML("sfx/reaper_cast.mml");
	
	enemy = this;
	enemy_sprite = sprite;

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 1800;
	stats->fixed.attack = 450;
	stats->fixed.defense = 60;
	stats->fixed.set_extra(LUCK, 40);

	return true;
}

bool Enemy_Reaper::take_turn()
{
	if (taking_turn == false) {
		taking_turn = true;
		attacking = util::rand(0, 1) == 0;
		if (attacking == false) {
			sprite->set_animation("cast");
			cast->play(false);
			start_darkness();
		}
	}

	if (attacking) {
		bool ret = turn_attack();
		if (ret == true) {
			taking_turn = false;
		}
		return ret;
	}
	else {
		return turn_darkness();
	}

	return false;
}

void Enemy_Reaper::draw_fore()
{
	if (taking_turn && attacking == false) {
		draw_darkness();
	}
}

//--

int Enemy_Rocky::count;
audio::Sound *Enemy_Rocky::slam;

void Enemy_Rocky::static_start()
{
	count = 0;
}

void Enemy_Rocky::sprite_callback(void *data)
{
	Enemy_Rocky *e = static_cast<Enemy_Rocky *>(data);
	e->get_sprite()->set_animation("idle");
	e->set_sprite_done();
}

Enemy_Rocky::Enemy_Rocky() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1647)/* Originally: Rocky */),
	taking_turn(false)
{
}

Enemy_Rocky::~Enemy_Rocky()
{
	count--;
	if (count == 0) {
		delete slam;
	}
}

bool Enemy_Rocky::start()
{
	if (count == 0) {
		slam = new audio::MML("sfx/slam.mml");
	}
	count++;

	experience = 175;
	gold = 125;
	attack_sound = new audio::MML("sfx/growl.mml");
	sprite = new gfx::Sprite("rocky");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 1800;
	stats->fixed.attack = 350;
	stats->fixed.defense = 75;
	stats->fixed.set_extra(LUCK, 40);
	stats->fixed.strength = WEAK_STRONG_MELEE;

	return true;
}

bool Enemy_Rocky::take_turn()
{
	if (taking_turn == false) {
		taking_turn = true;
		attacking = util::rand(0, 1) == 0;
		turn_start = GET_TICKS();
		if (attacking == false) {
			target = rand_living_player();
			slam_phase = 0;
			sprite->set_animation("slam", sprite_callback, this);
			slam_length = sprite->get_length();
			BATTLE->set_indicator(GLOBALS->game_t->translate(1654)/* Originally: Slam */, true);
		}
	}

	if (attacking) {
		bool ret = turn_attack();
		if (ret == true) {
			taking_turn = false;
		}
		return ret;
	}
	else {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - turn_start;
		if (slam_phase == JUMP2 && elapsed >= JUMP_TIME*2+slam_length*2) {
			taking_turn = false;
			return true;
		}
		else if (slam_phase == JUMP1 && elapsed >= JUMP_TIME+slam_length) {
			sprite->set_animation("slam", sprite_callback, this);
			slam_phase++;
			BATTLE->hit(this, target, 1.333f);
		}
	}

	return false;
}

void Enemy_Rocky::draw()
{
	if (taking_turn == false || attacking == true) {
		wedge::Battle_Enemy::draw();
	}
}

void Enemy_Rocky::draw_fore()
{
	if (taking_turn && attacking == false) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - turn_start;
		util::Point<float> p1 = position;
		wedge::Battle_Player *player = static_cast<wedge::Battle_Player *>(target);
		gfx::Sprite *player_sprite = player->get_sprite();
		gfx::Image *img = player_sprite->get_current_image();
		gfx::Image *rocky_img = sprite->get_current_image();
		util::Point<float> p2 = player->get_draw_pos() + util::Point<int>(img->size.w/2, img->size.h) + util::Point<int>(-rocky_img->size.w/2, -rocky_img->size.h*4/5);
		switch (slam_phase) {
			case 0:
				wedge::Battle_Enemy::draw();
				break;
			case 1: {
				elapsed -= slam_length;
				float p = (float)elapsed / JUMP_TIME;
				util::Point<float> pos = p1 + (p2 - p1) * p;
				pos.y -= sin(p*M_PI) * JUMP_H;
				util::Point<float> bak = position;
				position = pos;
				wedge::Battle_Enemy::draw();
				position = bak;
				break;
			}
			case 2: {
				util::Point<float> bak = position;
				position = p2;
				wedge::Battle_Enemy::draw();
				position = bak;
				break;
			}
			case 3: {
				elapsed -= slam_length * 2 + JUMP_TIME;
				float p = (float)elapsed / JUMP_TIME;
				util::Point<float> pos = p1 + (p2 - p1) * (1.0f - p);
				pos.y -= sin((1.0f-p)*M_PI) * JUMP_H;
				util::Point<float> bak = position;
				position = pos;
				wedge::Battle_Enemy::draw();
				position = bak;
				break;
			}
		}
	}
}

void Enemy_Rocky::set_sprite_done()
{
	slam_phase++;
	slam->play(false);
}

//--

static void wraith_sprite_callback(void *data)
{
	Enemy_Wraith *e = static_cast<Enemy_Wraith *>(data);
	e->get_sprite()->set_animation("idle");
}

void Enemy_Wraith::spell_effect_callback(void *data)
{
	Enemy_Wraith *e = static_cast<Enemy_Wraith *>(data);
	e->set_spell_effect_done();
}

Enemy_Wraith::Enemy_Wraith() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1646)/* Originally: Wraith */),
	spell_effect_done_count(0),
	taking_turn(false)
{
}

Enemy_Wraith::~Enemy_Wraith()
{
}

bool Enemy_Wraith::start()
{
	experience = 225;
	gold = 220;
	sprite = new gfx::Sprite("wraith");
	attack_sound = new audio::MML("sfx/growl.mml");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 2500;
	stats->fixed.attack = 325;
	stats->fixed.defense = 50;
	stats->fixed.set_extra(LUCK, 40);
	stats->fixed.weakness = WEAK_STRONG_ICE;
	stats->fixed.strength = WEAK_STRONG_FIRE;

	return true;
}

bool Enemy_Wraith::take_turn()
{
	if (taking_turn == false) {
		taking_turn = true;
		attacking = util::rand(0, 1) == 0;
		if (attacking == false) {
			target = rand_living_player();
			SPELLS->play_sound("Fire");
			BATTLE->set_indicator(GLOBALS->game_t->translate(1653)/* Originally: Fireball */, true);
			std::vector<wedge::Battle_Entity *> targets;
			targets.push_back(target);
			std::vector<wedge::Step *> v = SPELLS->start_effect("Fire", targets, spell_effect_callback, this);
			Fire_Step *f = static_cast<Fire_Step *>(v[0]);
			util::Point<float> start_pos = position + util::Point<int>(43, 17);
			f->set_start_pos(start_pos);
			wedge::Battle_Player *p = static_cast<wedge::Battle_Player *>(target);
			gfx::Sprite *player_sprite = p->get_sprite();
			gfx::Image *img = player_sprite->get_current_image();
			util::Point<float> end_pos = p->get_draw_pos() + util::Point<int>(img->size.w/2, img->size.h);
			f->set_end_pos(end_pos);
			f->set_start_angle((end_pos-start_pos).angle());

			sprite->set_animation("cast", wraith_sprite_callback, this);
		}
		else {
			last_frame = 0;
			attack_sound->play(false);
			num_hits = 0;
			first_target = NULL;
			second_target = NULL;
		}
	}

	if (attacking) {
		int frame = sprite->get_current_frame();
		if (last_frame == 1 && frame == 2) {
			first_target = rand_living_player();
			BATTLE->hit(this, first_target, 1.0f, num_hits * -shim::font->get_height());
			num_hits++;
			if (rand_living_player() == NULL) {
				taking_turn = false;
				return true;
			}
			second_target = rand_living_player();
		}
		last_frame = frame;
		bool ret = turn_attack(second_target, first_target != second_target ? 0 : num_hits * -shim::font->get_height());
		if (ret == true) {
			num_hits++;
			taking_turn = false;
		}
		return ret;
	}
	else if (spell_effect_done_count == 1) {
		spell_effect_done_count = 0;
		BATTLE->hit(this, target, 2.0f);
		taking_turn = false;
		return true;
	}

	return false;
}

void Enemy_Wraith::set_spell_effect_done()
{
	spell_effect_done_count++;
}

//--

int Enemy_Shadow::count;
audio::Sample *Enemy_Shadow::footsteps;

void Enemy_Shadow::static_start()
{
	count = 0;
}

Enemy_Shadow::Enemy_Shadow() :
	Monster_RPG_3_Battle_Enemy(GLOBALS->game_t->translate(1648)/* Originally: Shadow */),
	taking_turn(false)
{
}

Enemy_Shadow::~Enemy_Shadow()
{
	count--;
	if (count == 0) {
		delete footsteps;
	}
}

bool Enemy_Shadow::start()
{
	if (count == 0) {
		footsteps = new audio::Sample("footsteps.ogg");
	}
	count++;

	experience = 250;
	gold = 250;
	sprite = new gfx::Sprite("shadow");
	attack_sound = new audio::MML("sfx/growl.mml");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 2000;
	stats->fixed.attack = 350;
	stats->fixed.defense = 75;
	stats->fixed.set_extra(LUCK, 40);
	stats->fixed.weakness = WEAK_STRONG_FIRE;

	return true;
}

bool Enemy_Shadow::take_turn()
{
	if (taking_turn == false) {
		taking_turn = true;
		hits = util::rand(0, 4) <= 1 ? 2 : 1;
		hits_done = 0;
		turn_start = GET_TICKS();
		target = static_cast<wedge::Battle_Player *>(rand_living_player());
		gfx::Image *img = target->get_sprite()->get_current_image();
		gfx::Image *my_img = sprite->get_current_image();
		start_pos = position;
		end_pos = target->get_draw_pos() + util::Point<int>(-my_img->size.w-3, img->size.h-my_img->size.h/2);
		did_attack = false;
		played_footsteps = false;
		footsteps->play_stretched(2.0f, 0, audio::millis_to_samples(WALK_TIME), audio::SAMPLE_TYPE_SFX);
		played_attack_sound = false;
		num_hits = 0;
	}

	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - turn_start;

	if (elapsed >= WALK_TIME && played_attack_sound == false) {
		played_attack_sound = true;
		attack_sound->play(false);
	}

	if ((int)elapsed >= WALK_TIME*2 + hits*HIT_TIME) {
		position = start_pos;
		taking_turn = false;
		return true;
	}
	else if (elapsed < WALK_TIME) {
		float p = (float)elapsed / WALK_TIME;
		float f, p2;
		if (p >= 0.667f) {
			f = p - 0.667f;
			p2 = 0.667f;
		}
		else if (p >= 0.333f) {
			f = p - 0.333f;
			p2 = 0.333f;
		}
		else {
			f = p;
			p2 = 0.0f;
		}
		util::Point<float> diff = end_pos - start_pos;
		f = f / 0.333f;
		f = 1.0f - (1.0f - f) * (1.0f - f);
		f /= 3.0f;
		position = start_pos + diff * (p2 + f);
		//position = start_pos + diff * (p2 + sin(f/0.333f*M_PI/2)/3.0f);
	}
	else if ((int)elapsed > WALK_TIME + hits*HIT_TIME) {
		if (played_footsteps == false) {
			played_footsteps = true;
			footsteps->play_stretched(2.0f, 0, audio::millis_to_samples(WALK_TIME), audio::SAMPLE_TYPE_SFX);
		}
		float p = (float)(elapsed-WALK_TIME-hits*HIT_TIME) / WALK_TIME;
		if (p > 1.0f) {
			p = 1.0f;
		}
		p = 1.0f - p;
		float f, p2;
		if (p >= 0.667f) {
			f = p - 0.667f;
			p2 = 0.667f;
		}
		else if (p >= 0.333f) {
			f = p - 0.333f;
			p2 = 0.333f;
		}
		else {
			f = p;
			p2 = 0.0f;
		}
		util::Point<float> diff = end_pos - start_pos;
		f = f / 0.333f;
		f = 1.0f - (1.0f - f) * (1.0f - f);
		f /= 3.0f;
		position = start_pos + diff * (p2 + f);
		//position = start_pos + diff * (p2 + sin(f/0.333f*M_PI/2)/3.0f);
	}

	if (hits > 1 && hits_done < hits-1 && (int)elapsed >= WALK_TIME + hits_done*HIT_TIME) {
		int damage = BATTLE->hit(this, target, 1.0f, num_hits * -shim::font->get_height());
		num_hits++;
		hits_done++;
		if (target->get_stats()->hp <= 0) {
			target->get_sprite()->set_animation("dead");
			turn_start -= HIT_TIME * (hits-hits_done);
			hits_done = hits;
			did_attack = true;
		}
		else if (damage != 0) {
			target->get_sprite()->set_animation("hit");
		}
	}
	else if (hits_done == hits-1 && (int)elapsed >= WALK_TIME + (hits-1)*HIT_TIME && did_attack == false) {
		target->get_sprite()->set_animation("idle");
		bool ret = turn_attack(target, num_hits * -shim::font->get_height());
		if (ret == true) {
			num_hits++;
			did_attack = true;
			hits_done++;
		}
	}

	return false;
}

//--

void Enemy_Gayan::attack_callback(void *data)
{
	Enemy_Gayan *e = static_cast<Enemy_Gayan *>(data);
	e->dec_num_attacks();
}

void Enemy_Gayan::cry_callback(void *data)
{
	Enemy_Gayan *e = static_cast<Enemy_Gayan *>(data);
	e->set_idle();
}

Enemy_Gayan::Enemy_Gayan() :
	Enemy_Does_Creepy(GLOBALS->game_t->translate(1645)/* Originally: Gayan */),
	final_attack(false),
	deadly_attack(false),
	turn_count(0),
	drawing_hook(NULL)
{
	boss = true;
	use_death_shader = false;
	_use_death_sound = false;
	_remove_when_dead = false;
	creepy_mult = 1.75f;
	darkness_mult = 2.5f;
	cast_time = 0;
	buffer = NULL;
}

Enemy_Gayan::~Enemy_Gayan()
{
	delete cry_big;
	delete darkness_plus_sound;
}

bool Enemy_Gayan::start()
{
	cry_big = new audio::Sample("cry_big.ogg");

	experience = 0;
	gold = 0;
	sprite = new gfx::Sprite("gayan-battle");
	sprite->set_animation("grow");
	die_sound = new audio::MML("sfx/shrink.mml");
	darkness_plus_sound = new audio::MML("sfx/darkness.mml");

	enemy = this;
	enemy_sprite = sprite;

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 20000;
	stats->fixed.attack = 400;
	stats->fixed.defense = 75;
	stats->fixed.set_extra(LUCK, 40);

	return true;
}

bool Enemy_Gayan::take_turn()
{
	if (taking_turn == false) {
		taking_turn = true;
		if (stats->hp <= 9999 && INSTANCE->is_milestone_complete(MS_CAN_USE_VFIRE) == false) {
			if (INSTANCE->inventory.find(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_SECOND_CHANCE, 1)) >= 0) {
				deadly_attack = true;
			}
			else {
				INSTANCE->set_milestone_complete(MS_CAN_USE_VFIRE, true);
				deadly_attack = false;
			}
			action = DARKNESS_PLUS;
			draw_count = 0;
		}
		else if (INSTANCE->is_milestone_complete(MS_CAN_USE_VFIRE)) {
			final_attack = true;
			action = DARKNESS_PLUS;
			draw_count = 0;
		}
		else {
			if (turn_count != 0 && turn_count % 3 == 0) {
				action = DARKNESS;
			}
			else {
				action = util::rand(0, 1) == 0 ? ATTACK : CREEPY;
			}
			turn_count++;
		}

		if (action != ATTACK) {
			cry_big->play(2.5f, false);
			sprite->set_animation("cry", cry_callback, this);
			cry_ended = false;
			cry_start = GET_TICKS();
		}
		else {
			num_hits[0] = num_hits[1] = 0;
			num_attacks = util::rand(1, 3);
			prev_num_attacks = num_attacks;
			attacked = rand_living_player();
			sprite->set_animation("attack", attack_callback, this);
		}
	}

	bool ret;

	if (action != ATTACK && cry_ended == false) {
		if (int(GET_TICKS()-cry_start) >= audio::samples_to_millis(cry_big->get_length(), cry_big->get_frequency())) {
			end_cry();
		}
	}
	
	if (action == ATTACK) {
		std::vector<Battle_Entity *> players = BATTLE->get_players();
		int player_index = (attacked == players[0]) ? 0 : 1;
		int nh = num_hits[player_index];
		if (num_attacks >= 1 && prev_num_attacks != num_attacks) {
			BATTLE->hit(this, attacked, 1.0f, nh * -shim::font->get_height());
			num_hits[player_index]++;
			attacked = rand_living_player();
			if (attacked == NULL) {
				ret = true;
			}
			else {
				ret = false;
			}
		}
		else if (num_attacks == 1) {
			ret = turn_attack(attacked, nh * -shim::font->get_height());
		}
		else {
			ret = false;
		}
		prev_num_attacks = num_attacks;
	}
	else if (action == CREEPY && cry_ended) {
		ret = turn_creepy();
	}
	else if (action == DARKNESS && cry_ended) {
		ret = turn_darkness();
	}
	else if (cry_ended) {
		Uint32 now = GET_TICKS() - darkness_plus_start;
		if (now >= DARKNESS_TIME) {
			if (buffer) {
				delete buffer;
				buffer = NULL;
			}
			std::vector<Battle_Entity *> players = BATTLE->get_players();
			for (size_t i = 0; i < players.size(); i++) {
				wedge::Battle_Entity *player = players[i];
				int hp = player->get_stats()->hp;
				if (hp > 0) {
					if (final_attack || deadly_attack) {
						BATTLE->hit(this, player, 1.0f, 0, hp);
					}
					else {
						BATTLE->hit(this, player, 1.0f, 0, hp-1);
					}
				}
			}
			ret = true;
		}
		else {
			ret = false;
		}
	}
	else {
		ret = false;
	}

	if (ret == true && final_attack == false && INSTANCE->is_milestone_complete(MS_CAN_USE_VFIRE)) {
		std::vector<Battle_Entity *> players = BATTLE->get_players();
		for (size_t i = 0; i < players.size(); i++) {
			players[i]->get_stats()->hp = 1;
			players[i]->get_sprite()->set_animation("stand_w");
		}
		BATTLE->show_enemy_stats(false);
		BATTLE->show_player_stats(false);
	}

	if (ret == true) {
		taking_turn = false;
	}

	return ret;
}

void Enemy_Gayan::dec_num_attacks()
{
	num_attacks--;
	if (num_attacks > 0) {
		sprite->set_animation("attack", attack_callback, this);
	}
	else {
		sprite->set_animation("idle");
	}
}

void Enemy_Gayan::end_cry()
{
	cry_ended = true;
	if (action == CREEPY) {
		start_creepy();
	}
	else if (action == DARKNESS) {
		start_darkness();
	}
	else {
		darkness_plus_start = GET_TICKS();
		darkness_plus_sound->play(false);
	}
}

void Enemy_Gayan::draw_fore()
{
	if (cry_ended == false || taking_turn == false) {
		return;
	}
	
	if (action == CREEPY) {
		draw_creepy();
	}
	else if (action == DARKNESS) {
		draw_darkness();
	}
	else {
		int mod = draw_count % 2;
		draw_count++;
		if (mod == 0) {
			if (drawing_hook == NULL) {
				drawing_hook = new Enemy_Drawing_Hook_Step(this, true);
			}
			drawing_hook->hook();
			return;
		}
		Uint32 now = GET_TICKS() - darkness_plus_start;
		if (now < DARKNESS_TIME) {
			if (buffer != NULL && buffer_size != shim::screen_size) {
				delete buffer;
				buffer = NULL;
			}
			if (buffer == NULL) {
				buffer = new gfx::Image(shim::screen_size*2);
				buffer_size = shim::screen_size;
			}

			gfx::set_target_image(buffer);
			gfx::clear(shim::transparent);
		
			shim::current_shader = M3_GLOBALS->darkness_shader;
			shim::current_shader->use();
			gfx::update_projection();
			const int phase = DARKNESS_TIME;
			const int half_phase = phase / 2;
			Uint32 mod = now % half_phase;
			float t = mod / (float)half_phase;
			shim::current_shader->set_int("screen_w", buffer->size.w);
			shim::current_shader->set_int("screen_h", buffer->size.h);
			shim::current_shader->set_float("t", t);

			float mush_x = float(buffer->size.w/2+6);
			float mush_y = float(buffer->size.h/2+18);
			float max_x = mush_x;
			float max_y = mush_y;
			float max = sqrt(max_x*max_x + max_y*max_y);
			float j = fmodf(float(now), float(phase));
			bool b;
			if (j >= float(phase/2)) {
				b = false;
			}
			else {
				b = true;
			}
			SDL_Colour colour1;
			SDL_Colour colour2;
			if (b) {
				colour1 = shim::black;
				colour2 = shim::transparent;
			}
			else {
				colour1 = shim::transparent;
				colour2 = shim::black;
			}

			shim::current_shader->set_float("mush_x", mush_x);
			shim::current_shader->set_float("mush_y", mush_y);
			shim::current_shader->set_float("maxx", max);
			shim::current_shader->set_colour("colour1", colour1);
			shim::current_shader->set_colour("colour2", colour2);

			gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), buffer->size);

			shim::current_shader = shim::default_shader;
			shim::current_shader->use();
			gfx::update_projection();

			gfx::set_target_backbuffer();

			buffer->stretch_region(util::Point<int>(0, 0), buffer->size, util::Point<int>(0, 0), shim::screen_size);
		}
	}
}

void Enemy_Gayan::set_idle()
{
	sprite->set_animation("idle");
}
