#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/battle_end.h"
#include "Nooskewl_Wedge/battle_enemy.h"
#include "Nooskewl_Wedge/battle_entity.h"
#include "Nooskewl_Wedge/battle_game.h"
#include "Nooskewl_Wedge/battle_player.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/generic_callback.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/omnipresent.h"
#include "Nooskewl_Wedge/stats.h"

using namespace wedge;

static const int last_shall_be_first_percent = 5;

namespace wedge {

// not static!
void battle_hit_callback(void *data)
{
	Battle_Game *g = static_cast<Battle_Game *>(data);
	g->hit_done();
}

Battle_Game::Battle_Game(std::string bg, int bg_delay) :
	indicator_time(1500),
	turn(0),
	started_turn(-1),
	bg_delay(bg_delay),
	done(false),
	enemy_stats_shown(true),
	player_stats_shown(true),
	gold_shown(false),
	experience_shown(false),
	object_shown(false),
	battle_end_count(0),
	gold(0),
	experience(0),
	boss_battle(false),
	gameover(false),
	waiting_for_hit(0),
	delay(0),
	offered_retry(false),
	retry_choice_received(false)
{
	for (size_t i = 0; i < MAX_PARTY; i++) {
		levelup_shown.push_back(false);
	}

	for (int i = 1; i <= 1000; i++) {
		gfx::Image *img;
		try {
			img = new gfx::Image("battle_bgs/" + bg + util::itos(i) + ".tga");
		}
		catch (util::Error e) {
			break;
		}
		backgrounds.push_back(img);
	}

	battle_start_time = GET_TICKS();
}

Battle_Game::~Battle_Game()
{
	for (size_t i = 0; i < entities.size(); i++) {
		delete entities[i];
	}

	for (size_t i = 0; i < backgrounds.size(); i++) {
		delete backgrounds[i];
	}

	if (AREA == NULL) { // was a gameover
		delete music_backup;
	}
	else {
		delete shim::music;
		shim::music = music_backup;
		shim::music->play(shim::music_volume, true);
	}
}

bool Battle_Game::start()
{
	INSTANCE->step_count = 0; // reset step count

	globals->battle_start->play(false);

	music_backup = shim::music;
	music_backup->pause();
	if (boss_battle) {
		shim::music = new audio::MML("music/boss.mml");
	}
	else {
		shim::music = new audio::MML("music/battle.mml");
	}
	shim::music->play(shim::music_volume, true);

	for (size_t i = 0; i < MAX_PARTY; i++) {
		Battle_Player *player = create_player((int)i);
		player->start();
		entities.push_back(player);
	}

	NEW_SYSTEM_AND_TASK(this)
	battle_end_step = new Battle_End_Step(new_task);
	ADD_STEP(battle_end_step)
	ADD_TASK(new_task)
	FINISH_SYSTEM(this)

	if (boss_battle == false && util::rand(1, 100) <= last_shall_be_first_percent) {
		turn = 2;
		gfx::add_notification(GLOBALS->game_t->translate(1000)/* Originally: SNEAK ATTACK! */);
		start_delay(2000);
	}

	return true;
}

void Battle_Game::handle_event(TGUI_Event *event)
{
	entities[turn]->handle_event(event);

	if (dynamic_cast<Battle_Enemy *>(entities[turn])) {
		if (
			(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
			(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)
		) {
			if (offered_retry == false) {
				GLOBALS->mini_pause();
				return;
			}
		}
	}
	
	Game::handle_event(event);
}

bool Battle_Game::run()
{
	for (size_t i = 0; i < entities.size(); i++) {
		entities[i]->run();
	}

	std::vector<Battle_Entity *> players = get_players();
	bool all_players_dead = true;
	for (size_t i = 0; i < players.size(); i++) {
		if (players[i]->get_stats()->hp > 0) {
			all_players_dead = false;
			break;
		}
	}

	std::vector<Battle_Entity *> enemies = get_enemies();
	bool all_enemies_dead = true;
	for (size_t i = 0; i < enemies.size(); i++) {
		if (enemies[i]->get_stats()->hp > 0) {
			all_enemies_dead = false;
			break;
		}
	}

	if (turn >= (int)entities.size()) {
		turn = 0;
	}

	Battle_Entity *entity = entities[turn];
	Base_Stats *stats = entity->get_stats();
	int status = stats->status;

	// this is slightly complex...
	if ((all_enemies_dead && all_players_dead) == false && (all_enemies_dead == false || turn == started_turn) && (GET_TICKS() >= delay) && (turn == started_turn || waiting_for_hit == 0) && (turn == started_turn || all_players_dead == false) && (started_turn = turn) >= 0 && (stats->hp <= 0 || entity->take_turn())) {
		if (stats->hp > 0) {
			if (status == STATUS_POISONED) {
				entity->take_damage(stats->fixed.max_hp * 0.05f, STATUS_POISONED);

				if (entity->get_type() == Battle_Entity::ENEMY && entity->get_stats()->hp <= 0) {
					static_cast<Battle_Enemy *>(entity)->play_die_sound();
				}
				else {
					globals->poison->play(false);
				}

				if (stats->hp <= 0) {
					stats->status = STATUS_OK;
				}
			}
		}

		all_players_dead = true;
		for (size_t i = 0; i < players.size(); i++) {
			if (players[i]->get_stats()->hp > 0) {
				all_players_dead = false;
				break;
			}
		}

		all_enemies_dead = true;
		for (size_t i = 0; i < enemies.size(); i++) {
			if (enemies[i]->get_stats()->hp > 0) {
				all_enemies_dead = false;
				break;
			}
		}

		if (all_enemies_dead == false ||  all_players_dead == false) {
			do {
				turn++;
				turn %= entities.size();
				started_turn = -1;
				if (all_players_dead) {
					break;
				}
			} while (dynamic_cast<Battle_Player *>(entities[turn]) != NULL && entities[turn]->get_stats()->hp <= 0);
		}
	}

	if (all_players_dead == false) {
		bool all_enemies_dead = true; // must do this again after taking turn

		int count = 0;
		std::vector<Battle_Entity *>::iterator it;
		for (it = entities.begin(); it != entities.end();) {
			Battle_Entity *entity = *it;
			if (entity->is_dead()) {
				if (dynamic_cast<Battle_Enemy *>(entity) != NULL && static_cast<Battle_Enemy *>(entity)->remove_when_dead()) {
					delete entity;
					it = entities.erase(it);
				}
				else {
					it++;
				}
				if (turn > count) {
					turn--;
				}
				if (turn >= (int)entities.size()) {
					turn = 0;
				}
			}
			else {
				if (entity->get_type() == Battle_Entity::ENEMY) {
					all_enemies_dead = false;
				}
				it++;
			}
			count++;
		}

		if (all_enemies_dead) {
			enemy_stats_shown = false;
			player_stats_shown = false;

			if (gold_shown == false) {
				gold_shown = true;
				if (gold == 0) {
					battle_end_count++;
				}
				else {
					delete shim::music;
					shim::music = new audio::MML("music/victory.mml");
					shim::music->play(shim::music_volume, true);

					for (size_t i = 0; i < MAX_PARTY; i++) {
						if (INSTANCE->stats[i].base.hp > 0) {
							players[i]->get_sprite()->set_animation("victory");
						}
					}

					INSTANCE->add_gold(gold);
					globals->do_dialogue("", util::string_printf(GLOBALS->game_t->translate(1001)/* Originally: Received %d gold! */.c_str(), gold), DIALOGUE_MESSAGE, DIALOGUE_BOTTOM, battle_end_step);
				}
			}
			else if (experience_shown == false && battle_end_count > 0) {
				experience_shown = true;
				if (experience == 0) {
					battle_end_count++;
				}
				else {
					for (size_t i = 0; i < MAX_PARTY; i++) {
						if (INSTANCE->stats[i].base.hp > 0) {
							INSTANCE->stats[i].experience += experience;
						}
					}
					globals->do_dialogue("", util::string_printf(GLOBALS->game_t->translate(1002)/* Originally: Received %d experience! */.c_str(), experience), DIALOGUE_MESSAGE, DIALOGUE_BOTTOM, battle_end_step);
				}
			}
			else if (object_shown == false && battle_end_count > 1) {
				object_shown = true;
				Object found_object = get_found_object();
				if (found_object.type != OBJECT_NONE) {
					std::string message;
					int can_add = INSTANCE->inventory.add(found_object);
					if (can_add != found_object.quantity) {
						if (can_add > 0) {
							found_object.quantity = can_add;
							INSTANCE->inventory.add(found_object);
						}
						message = GLOBALS->game_t->translate(1003)/* Originally: Inventory full! */;
					}
					else {
						if (found_object.quantity > 1) {
							if (GLOBALS->language == "Spanish") {
								message = util::string_printf(GLOBALS->game_t->translate(1004)/* Originally: Received %d %s! */.c_str(), GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(found_object.name)).c_str(), found_object.quantity);
							}
							else {
								message = util::string_printf(GLOBALS->game_t->translate(1004)/* Originally: Received %d %s! */.c_str(), found_object.quantity, GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(found_object.name)).c_str());
							}
						}
						else {
							message = util::string_printf(GLOBALS->game_t->translate(1005)/* Originally: Received %s! */.c_str(), GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(found_object.name)).c_str());
						}
					}
					globals->do_dialogue("", message, DIALOGUE_MESSAGE, DIALOGUE_BOTTOM, battle_end_step);
				}
				else {
					battle_end_count++;
				}
			}
			else if (battle_end_count > 2 && battle_end_count < 3 + (int)MAX_PARTY) {
				for (size_t i = 0; i < MAX_PARTY; i++) {
					if (levelup_shown[i] == false && (size_t)battle_end_count > 2+i) {
						int num_levelups = level_up((int)i);
						if (num_levelups > 0) {
							std::string text;
							if (num_levelups > 1) {
								text = util::string_printf(GLOBALS->game_t->translate(1006)/* Originally: %s gained %d levels! */.c_str(), GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(INSTANCE->stats[i].name)).c_str(), num_levelups);
							}
							else {
								text = util::string_printf(GLOBALS->game_t->translate(1007)/* Originally: %s gained a level! */.c_str(), GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(INSTANCE->stats[i].name)).c_str());
							}
							globals->do_dialogue("", text, DIALOGUE_MESSAGE, DIALOGUE_BOTTOM, battle_end_step);
						}
						else {
							battle_end_count++;
						}
						levelup_shown[i] = true;
					}
				}
			}
			else if (battle_end_count == 3+(int)MAX_PARTY) {
				set_done(true);
			}
		}
	}

	if (gameover && GET_TICKS() >= gameover_time + globals->gameover_timeout) {
		delete AREA;
		AREA = NULL;
		if (boss_battle == false) {
			globals->retried_boss = false;
			return false;
		}
		else if (offered_retry == false) {
			offered_retry = true;
			show_retry_boss_gui();
			OMNIPRESENT->end_fade();
		}
		else if (retry_choice_received) {
			globals->retried_boss = globals->retry_boss;
			return false;
		}
	}

	Game::run();

	return true; // the transition out handles destruction of the battle game in normal circumstances
}

util::Point<int> Battle_Game::get_offset()
{
	util::Size<int> offset = (shim::screen_size-backgrounds[0]->size)/2;
	return util::Point<int>(offset.w, offset.h);
}
	
std::vector<Battle_Entity *> Battle_Game::get_players()
{
	std::vector<Battle_Entity *> v;
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->get_type() == Battle_Entity::PLAYER) {
			v.push_back(entities[i]);
		}
	}
	return v;
}

std::vector<Battle_Entity *> Battle_Game::get_enemies()
{
	std::vector<Battle_Entity *> v;
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->get_type() == Battle_Entity::ENEMY) {
			Base_Stats *stats = entities[i]->get_stats();
			if (stats->hp > 0) {
				v.push_back(entities[i]);
			}
		}
	}
	return v;
}

void Battle_Game::show_enemy_stats(bool show)
{
	enemy_stats_shown = show;
}

void Battle_Game::show_player_stats(bool show)
{
	player_stats_shown = show;
}

bool Battle_Game::get_enemy_stats_shown()
{
	return enemy_stats_shown;
}

bool Battle_Game::get_player_stats_shown()
{
	return player_stats_shown;
}

int Battle_Game::hit(Battle_Entity *attacker, Battle_Entity *attacked, float mult, int y_offset, int exact_damage)
{
	int damage = exact_damage != 0 ? exact_damage : get_damage(attacker, attacked, y_offset);

	if (damage == 0) {
		return 0;
	}

	damage = (damage > 0 && int(damage * mult) <= 0) ? damage : damage * mult;

	attacked->take_damage(damage, STATUS_OK, y_offset);

	if (attacked->get_stats()->hp > 0 && (util::rand(0, 1000)/1000.0f) < attacker->get_poison_odds()) {
		gfx::add_notification(GLOBALS->game_t->translate(1008)/* Originally: Poisoned! */);
		attacked->get_stats()->status = STATUS_POISONED;
	}

	if (attacked->get_type() == Battle_Entity::ENEMY && attacked->get_stats()->hp <= 0) {
		Battle_Enemy *e = static_cast<Battle_Enemy *>(attacked);
		if (e->use_death_sound()) {
			e->play_die_sound();
		}
		else {
			globals->hit->play(false);
		}
		gfx::Sprite *sprite = attacked->get_sprite();
		sprite->set_animation("die");
	}
	else {
		globals->hit->play(false);
	}
	
	start_hit_effect(attacked);

	return damage;
}

void Battle_Game::battle_end_signal()
{
	battle_end_count++;
}

bool Battle_Game::is_boss_battle()
{
	return boss_battle;
}

void Battle_Game::set_done(bool done)
{
	if (this->done == done) {
		return;
	}

	this->done = done;

	if (done) {
		start_transition_out();
	}
}

Object Battle_Game::get_found_object()
{
	Object o;
	o.type = OBJECT_NONE;
	return o;
}

void Battle_Game::set_gameover(bool gameover)
{
	this->gameover = gameover;
	if (gameover) {
		gameover_time = GET_TICKS();
	}
}

void Battle_Game::add_entity(Battle_Entity *entity)
{
	entities.push_back(entity);
}

void Battle_Game::add_gold(int gold)
{
	this->gold += gold;
}

void Battle_Game::add_experience(int experience)
{
	this->experience += experience;
}

void Battle_Game::set_indicator(std::string text, bool left)
{
	indicator_text = text;
	indicator_start = GET_TICKS();
	indicator_left = left;
}

int Battle_Game::level_up(int player_index)
{
	Player_Stats *s = &INSTANCE->stats[player_index];
	int levelups = 0;

	for (size_t i = 0; i < globals->levels.size(); i++) {
		if ((size_t)s->level < i+2 && s->experience >= globals->levels[i].experience) {
			levelups++;
			s->level++;
			s->base.fixed += globals->levels[i].stat_increases;
		}
	}

	if (levelups > 0) {
		globals->levelup->play(false);
	}

	return levelups;
}

void Battle_Game::hit_done()
{
	waiting_for_hit--;
	if (waiting_for_hit < 0) {
		waiting_for_hit = 0;
	}
}

std::vector<Battle_Entity *> Battle_Game::get_all_entities()
{
	return entities;
}

void Battle_Game::start_delay(int millis)
{
	Uint32 d = GET_TICKS() + millis;
	delay = MAX(d, delay);
}

void Battle_Game::inc_waiting_for_hit(int waiting_for_hit)
{
	this->waiting_for_hit += waiting_for_hit;
}

void Battle_Game::draw_fore()
{
	Game::draw_fore();
}

int Battle_Game::get_damage(Battle_Entity *attacker, Battle_Entity *attacked, int y_offset)
{
	USE_WEAK_STRONG

	Base_Stats *attacker_stats = attacker->get_stats();
	Base_Stats *target_stats = attacked->get_stats();

	int damage;

	damage = attacker_stats->fixed.attack;

	if (attacker->get_type() == Battle_Entity::PLAYER) {
		Battle_Player *p = static_cast<Battle_Player *>(attacker);
		Player_Stats *player_stats = p->get_player_stats();
		damage += player_stats->weapon.stats.attack;
		damage += player_stats->armour.stats.attack;
		if (target_stats->fixed.weakness != wedge::WEAK_STRONG_NONE && target_stats->fixed.weakness == player_stats->weapon.stats.strength) {
			damage *= 1.5f;
			WEAK_STRONG(WS_WEAK)
		}
		if (target_stats->fixed.strength != wedge::WEAK_STRONG_NONE && target_stats->fixed.strength == player_stats->weapon.stats.strength) {
			damage = -damage/2;
			WEAK_STRONG(WS_STRONG)
		}
	}

	damage -= target_stats->fixed.defense;
	
	if (attacked->get_type() == Battle_Entity::PLAYER) {
		Battle_Player *p = static_cast<Battle_Player *>(attacked);
		Player_Stats *player_stats = p->get_player_stats();
		damage -= player_stats->armour.stats.defense;
		damage -= player_stats->weapon.stats.defense;
		if (player_stats->armour.stats.weakness != wedge::WEAK_STRONG_NONE && attacker_stats->fixed.strength == player_stats->armour.stats.weakness) {
			damage *= 1.5f;
			WEAK_STRONG(WS_WEAK)
		}
		if (player_stats->armour.stats.strength != wedge::WEAK_STRONG_NONE && attacker_stats->fixed.strength == player_stats->weapon.stats.strength) {
			damage = -damage/2;
			WEAK_STRONG(WS_STRONG)
		}
	}

	damage *= (1.0f + util::rand(0, 1000) / 5000.0f);

	if (damage <= 0) {
		damage = 1;
	}

	WEAK_STRONG_NOTIFICATION

	return damage;
}

void Battle_Game::start_transition_in()
{
	rumble(1.0f, 1000);
}

void Battle_Game::set_retry_boss_choice_received(bool received)
{
	retry_choice_received = received;
}

void Battle_Game::lost_device()
{
	Game::lost_device();

	for (size_t i = 0; i < entities.size(); i++) {
		entities[i]->lost_device();
	}
}

void Battle_Game::found_device()
{
	Game::found_device();

	for (size_t i = 0; i < entities.size(); i++) {
		entities[i]->found_device();
	}
}

void Battle_Game::resize(util::Size<int> new_size)
{
	for (size_t i = 0; i < entities.size(); i++) {
		Battle_Entity *entity = entities[i];
		entity->resize(new_size);
	}
}

void Battle_Game::set_music_backup(audio::MML *music_backup)
{
	delete this->music_backup;
	this->music_backup = music_backup;
}

}
