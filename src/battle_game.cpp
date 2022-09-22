#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/battle_enemy.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/generic_callback.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/input.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/pause_task.h>
#include <Nooskewl_Wedge/special_number.h>

#include "achievements.h"
#include "battle_game.h"
#include "battle_player.h"
#include "battle_transition_in.h"
#include "battle_transition_out.h"
#include "dialogue.h"
#include "general.h"
#include "globals.h"
#include "hit.h"
#include "stats.h"

static void retry_boss_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = (Yes_No_GUI::Callback_Data *)data;
	GLOBALS->retry_boss = d->cancelled == false && d->choice;
	BATTLE->set_retry_boss_choice_received(true);
}

Monster_RPG_3_Battle_Game::Monster_RPG_3_Battle_Game(std::string bg, int bg_delay) :
	wedge::Battle_Game(bg, bg_delay),
	curr_bg_frame(0),
	started_taking_turns(false)
{
	if (bg == "at_sea") {
		tint = shim::palette[25];
	}
	else if (bg == "desert") {
		tint = shim::palette[4];
	}
	else if (bg == "forest") {
		tint = shim::palette[15];
	}
	else if (bg == "hh") {
		tint = shim::palette[25];
	}
	else {
		// FIXME: mountains
		tint = shim::palette[23];
	}
}

Monster_RPG_3_Battle_Game::~Monster_RPG_3_Battle_Game()
{
}

void Monster_RPG_3_Battle_Game::start_transition_in()
{
	wedge::Battle_Game::start_transition_in(); // this does a rumble on joystick

	NEW_SYSTEM_AND_TASK(AREA)
	wedge::Map_Entity *player = AREA->get_player(0);
	wedge::Pause_Task_Step *pause1 = new wedge::Pause_Task_Step(player->get_input_step()->get_task(), true, new_task);
	Battle_Transition_In_Step *battle_step = new Battle_Transition_In_Step(this, new_task);
	wedge::Pause_Task_Step *pause2 = new wedge::Pause_Task_Step(player->get_input_step()->get_task(), false, new_task);
	ADD_STEP(pause1)
	ADD_STEP(battle_step)
	ADD_STEP(pause2)
	ADD_TASK(new_task)
	FINISH_SYSTEM(AREA)
}

void Monster_RPG_3_Battle_Game::start_transition_out()
{
	GLOBALS->retried_boss = false;

	NEW_SYSTEM_AND_TASK(this)
	Battle_Transition_Out_Step *step = new Battle_Transition_Out_Step(new_task);
	ADD_STEP(step)
	ADD_TASK(new_task)
	FINISH_SYSTEM(this)
}

wedge::Battle_Player *Monster_RPG_3_Battle_Game::create_player(int index)
{
	return new Monster_RPG_3_Battle_Player(index);
}

void Monster_RPG_3_Battle_Game::draw()
{
	Uint32 now = GET_TICKS();
	int elapsed = now - battle_start_time;
	curr_bg_frame = bg_delay <= 0 ? 0 : (elapsed / bg_delay) % backgrounds.size();
	backgrounds[curr_bg_frame]->draw(get_offset());

	for (size_t i = 0; i < entities.size(); i++) {
		wedge::Battle_Entity *entity = entities[i];
		entity->draw_back();
	}

	for (size_t i = 0; i < entities.size(); i++) {
		wedge::Battle_Entity *entity = entities[i];
		wedge::Base_Stats *stats = entity->get_stats();
		gfx::Sprite *sprite = entity->get_sprite();
		entity->draw();
		if (stats->status == wedge::STATUS_POISONED || stats->status == STATUS_BLIND) {
			gfx::Image *img;
			if (stats->status == wedge::STATUS_POISONED) {
				img = wedge::globals->poison_sprite->get_current_image();
			}
			else {
				img = M3_GLOBALS->blind_sprite->get_current_image();
			}
			util::Point<int> topleft, bottomright;
			sprite->get_bounds(topleft, bottomright);
			util::Point<int> sz = bottomright - topleft;
			util::Point<int> pos;
			if (entity->get_type() == wedge::Battle_Entity::PLAYER) {
				wedge::Battle_Player *player = static_cast<wedge::Battle_Player *>(entity);
				pos = player->get_draw_pos();
			}
			else {
				wedge::Battle_Enemy *enemy = static_cast<wedge::Battle_Enemy *>(entity);
				pos = enemy->get_position();
			}
			if (stats->status == wedge::STATUS_POISONED) {
				pos += topleft + (sz/2) - img->size/2;
			}
			else {
				pos += topleft + util::Point<int>(sz.x/2 - img->size.w/2, 0);
			}
			img->draw(pos);
		}
	}

	Game::draw();
	
	for (size_t i = 0; i < entities.size(); i++) {
		wedge::Battle_Entity *entity = entities[i];
		entity->draw_fore();
	}
}

void Monster_RPG_3_Battle_Game::draw_fore()
{
	wedge::Game::draw_fore();

	if (started_taking_turns == false) {
		return;
	}
	
	std::map<std::string, int> enemies;

	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->get_type() == wedge::Battle_Entity::ENEMY) {
			wedge::Battle_Enemy *enemy = static_cast<wedge::Battle_Enemy *>(entities[i]);
			enemies[enemy->get_name()]++;
		}
	}

	util::Size<int> PAD(shim::screen_size.w*0.03f, shim::screen_size.h*0.03f);
	int win_w = (shim::screen_size.w-PAD.w*2)/2;
	int HEIGHT = shim::font->get_height() * 2 + 4 + Dialogue_Step::BORDER * 2;
	int y = shim::screen_size.h - HEIGHT - PAD.h - 1;
	util::Point<int> text_pos(PAD.w + Dialogue_Step::BORDER, y+Dialogue_Step::BORDER);
	util::Point<int> text_pos_bak = text_pos;

	SDL_Colour colour;
	int i = 0;

	float little = get_little_bander_offset();

	if (enemy_stats_shown) {
		SDL_Colour *colours = start_bander(num_bands(HEIGHT-4), shim::palette[24], shim::palette[22]);
		gfx::draw_filled_rectangle(colours, util::Point<float>(PAD.w+2-little, y+2-little), util::Size<float>(win_w-4+little*2.0f, HEIGHT-4+little*2.0f));
		end_bander();
		gfx::draw_rectangle(shim::palette[24], util::Point<int>(PAD.w, y)+util::Point<int>(1, 1), util::Size<int>(win_w, HEIGHT)-util::Size<int>(2, 2));
		gfx::draw_rectangle(shim::palette[27], util::Point<int>(PAD.w, y), util::Size<int>(win_w, HEIGHT));

		std::map<std::string, int>::iterator it;
		for (it = enemies.begin(); it != enemies.end(); it++) {
			std::pair<std::string, int> p = *it;
			bool found = false;
			for (size_t j = 0; j < highlighted_entities.size(); j++) {
				if (highlighted_entities[j]->get_name() == p.first) {
					found = true;
					break;
				}
			}
			if (found) {
				colour = shim::palette[13];
			}
			else if (p.first == entities[turn]->get_name()) {
				colour = shim::palette[11];
			}
			else {
				colour = shim::palette[20];
			}
			i++;
			shim::font->enable_shadow(shim::palette[24], gfx::Font::DROP_SHADOW);
			shim::font->draw(colour, p.first, text_pos);
			std::string n_s = util::itos(p.second);
			int w = shim::font->get_text_width(n_s);
			shim::font->draw(colour, n_s, util::Point<int>(PAD.w + win_w - Dialogue_Step::BORDER - w - 1, text_pos.y));
			text_pos.y += shim::font->get_height() + 1;
			shim::font->disable_shadow();
		}
	}

	text_pos = text_pos_bak;
	text_pos.x = PAD.w+win_w+Dialogue_Step::BORDER - 1;

	if (player_stats_shown) {
		SDL_Colour *colours = start_bander(num_bands(HEIGHT-4), shim::palette[24], shim::palette[22]);
		gfx::draw_filled_rectangle(colours, util::Point<float>(PAD.w+win_w+1-little, y+2-little), util::Size<float>(win_w-3+little*2.0f, HEIGHT-4+little*2.0f));
		end_bander();
		gfx::draw_rectangle(shim::palette[24], util::Point<int>(PAD.w+win_w-1, y)+util::Point<int>(1, 1), util::Size<int>(win_w+1, HEIGHT)-util::Size<int>(2, 2));
		gfx::draw_rectangle(shim::palette[27], util::Point<int>(PAD.w+win_w-1, y), util::Size<int>(win_w+1, HEIGHT));
		std::vector<wedge::Battle_Entity *> players = get_players();

		for (size_t i = 0; i < players.size(); i++) {
			wedge::Base_Stats *base = players[i]->get_stats();
			if (std::find(highlighted_entities.begin(), highlighted_entities.end(), players[i]) != highlighted_entities.end()) {
				colour = shim::palette[13];
			}
			else if (turn == (int)i) {
				colour = shim::palette[11];
			}
			else {
				colour = get_status_colour(base->hp - base->fixed.max_hp * 0.15f, wedge::STATUS_OK);
			}
			draw_shadowed_image(shim::palette[24], M3_GLOBALS->mini_profile_images[i], text_pos + util::Point<int>(0, 2), gfx::Font::DROP_SHADOW);
			wedge::Base_Stats *stats = players[i]->get_stats();
			int w = shim::font->get_text_width("5555");
			std::string hp = util::itos(stats->hp) + "/";
			int w2 = shim::font->get_text_width(hp);
			shim::font->enable_shadow(shim::palette[24], gfx::Font::DROP_SHADOW);
			shim::font->draw(colour, hp, util::Point<int>(PAD.w+win_w*2-Dialogue_Step::BORDER-w-w2-2, text_pos.y));
			shim::font->draw(colour, util::itos(stats->fixed.max_hp), util::Point<int>(PAD.w+win_w*2-Dialogue_Step::BORDER-w-1, text_pos.y));
			shim::font->disable_shadow();
			text_pos.y += shim::font->get_height() + 1;
		}
	}
	
	if (indicator_text != "" && GET_TICKS() < indicator_start+indicator_time) {
		int w = shim::font->get_text_width(indicator_text) + 2 * Dialogue_Step::BORDER + 1;
		int h = shim::font->get_height() + 2 * Dialogue_Step::BORDER;
		int x = indicator_left ? PAD.w : shim::screen_size.w - w - PAD.w;
		SDL_Colour *colours = start_bander(num_bands(h-4), shim::palette[24], shim::palette[22]);
		gfx::draw_filled_rectangle(colours, util::Point<float>(x+2-little, PAD.h+2-little), util::Size<float>(w-4+little*2.0f, h-4+little*2.0f));
		end_bander();
		gfx::draw_rectangle(shim::palette[24], util::Point<int>(x, PAD.h)+util::Point<int>(1, 1), util::Size<int>(w, h)-util::Size<int>(2, 2));
		gfx::draw_rectangle(shim::palette[27], util::Point<int>(x, PAD.h), util::Size<int>(w, h));
		shim::font->enable_shadow(shim::palette[24], gfx::Font::DROP_SHADOW);
		shim::font->draw(shim::palette[20], indicator_text, util::Point<int>(x+Dialogue_Step::BORDER, PAD.h+Dialogue_Step::BORDER));
		shim::font->disable_shadow();
	}
	
	if (gameover && GET_TICKS() >= gameover_time + GLOBALS->gameover_timeout) {
		gfx::Font::end_batches();
		gfx::clear(GLOBALS->gameover_fade_colour);
	}
}


int Monster_RPG_3_Battle_Game::get_damage(wedge::Battle_Entity *attacker, wedge::Battle_Entity *attacked, int y_offset)
{
	USE_WEAK_STRONG

	bool miss, lucky_hit;

	get_lucky_misses(attacker, attacked, &miss, &lucky_hit);

	wedge::Base_Stats *attacker_stats = attacker->get_stats();
	wedge::Base_Stats *target_stats = attacked->get_stats();

	if (miss) {
		std::string text = GLOBALS->game_t->translate(1329)/* Originally: MISS! */;

		util::Point<int> topleft, bottomright;
		attacked->get_sprite()->get_bounds(topleft, bottomright);
			
		util::Point<int> number_pos = attacked->get_decoration_offset(shim::font->get_text_width(text), util::Point<int>(shim::tile_size*3/4, 0), NULL);

		number_pos.y += y_offset;

		SDL_Colour colour;
		SDL_Colour shadow_colour;

		colour = shim::palette[13];
		shadow_colour = shim::palette[27];

		NEW_SYSTEM_AND_TASK(BATTLE)
		wedge::Special_Number_Step *step = new wedge::Special_Number_Step(colour, shadow_colour, text, number_pos, wedge::Special_Number_Step::RISE, new_task);
		ADD_STEP(step)
		wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, BATTLE, new_task);
		step->add_monitor(g);
		ADD_STEP(g)
		ADD_TASK(new_task)
		FINISH_SYSTEM(BATTLE)

		waiting_for_hit++;
	}
	else if (lucky_hit) {
		if (dynamic_cast<wedge::Battle_Player *>(attacker)) {
			M3_INSTANCE->lucky_hits++;
			if (M3_INSTANCE->lucky_hits >= 10) {
				util::achieve((void *)ACHIEVE_LUCKY);
			}
		}
		gfx::add_notification(GLOBALS->game_t->translate(1330)/* Originally: LUCKY HIT! */);
	}

	int damage;

	if (miss) {
		damage = 0;
	}
	else {
		bool gave_hp = false;

		damage = attacker_stats->fixed.attack;

		if (attacker->get_type() == wedge::Battle_Entity::PLAYER) {
			wedge::Battle_Player *p = static_cast<wedge::Battle_Player *>(attacker);
			wedge::Player_Stats *player_stats = p->get_player_stats();
			damage += player_stats->weapon.stats.attack;
			damage += player_stats->armour.stats.attack;
			if (target_stats->fixed.weakness != wedge::WEAK_STRONG_NONE && target_stats->fixed.weakness == player_stats->weapon.stats.strength) {
				damage *= 1.5f;
				WEAK_STRONG(WS_WEAK)
			}
			if (target_stats->fixed.strength != WEAK_STRONG_MELEE && target_stats->fixed.strength != wedge::WEAK_STRONG_NONE && target_stats->fixed.strength == player_stats->weapon.stats.strength) {
				damage = -damage/2;
				WEAK_STRONG(WS_STRONG)
				gave_hp = true;
			}
			if (target_stats->fixed.strength == WEAK_STRONG_MELEE) {
				damage = 1;
			}
		}

		damage -= target_stats->fixed.defense;
		
		if (attacked->get_type() == wedge::Battle_Entity::PLAYER) {
			wedge::Battle_Player *p = static_cast<wedge::Battle_Player *>(attacked);
			wedge::Player_Stats *player_stats = p->get_player_stats();
			damage -= player_stats->armour.stats.defense;
			damage -= player_stats->weapon.stats.defense;
			if (player_stats->armour.stats.weakness != wedge::WEAK_STRONG_NONE && attacker_stats->fixed.strength == player_stats->armour.stats.weakness) {
				damage *= 1.5f;
				WEAK_STRONG(WS_WEAK)
			}
			if (player_stats->armour.stats.strength != WEAK_STRONG_MELEE && player_stats->armour.stats.strength != wedge::WEAK_STRONG_NONE && attacker_stats->fixed.strength == player_stats->weapon.stats.strength) {
				damage = -damage/2;
				WEAK_STRONG(WS_STRONG)
				gave_hp = true;
			}
			if (player_stats->armour.stats.strength == WEAK_STRONG_MELEE) {
				damage = 1;
			}
		}

		damage *= (1.0f + util::rand(0, 1000) / 5000.0f);

		if (attacked->is_defending()) {
			damage /= 2;
		}

		if (lucky_hit) {
			damage *= 1.5f;
		}

		if (gave_hp == false && damage <= 0) {
			damage = 1;
		}
	}

	WEAK_STRONG_NOTIFICATION

	return damage;
}

void Monster_RPG_3_Battle_Game::start_hit_effect(wedge::Battle_Entity *attacked)
{
	NEW_SYSTEM_AND_TASK(this)
	Hit_Step *hit = new Hit_Step(attacked, 0, new_task);
	ADD_STEP(hit)
	wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, this, new_task);
	hit->add_monitor(g);
	ADD_STEP(g)
	ADD_TASK(new_task)
	FINISH_SYSTEM(this)
	waiting_for_hit++;
}

void Monster_RPG_3_Battle_Game::draw_selection_arrow(wedge::Battle_Entity *entity)
{
	gfx::Sprite *entity_sprite = entity->get_sprite();
	util::Size<int> entity_size = entity_sprite->get_current_image()->size;
	util::Size<int> arrow_size = M3_GLOBALS->selection_arrow->size;
	int flags;
	util::Point<int> draw_pos = entity->get_decoration_offset(arrow_size.w, util::Point<int>(shim::tile_size/2, arrow_size.h > entity_size.h ? entity_size.h-arrow_size.h : 0), &flags);
	start_pulse_brighten(0.25f, false, true);
	M3_GLOBALS->selection_arrow->draw(draw_pos, flags);
	end_pulse_brighten();
}

void Monster_RPG_3_Battle_Game::get_lucky_misses(wedge::Battle_Entity *attacker, wedge::Battle_Entity *attacked, bool *miss, bool *lucky_hit)
{
	*miss = false;
	*lucky_hit = false;

	wedge::Base_Stats *attacker_stats = attacker->get_stats();
	wedge::Base_Stats *attacked_stats = attacked->get_stats();

	int attacker_luck = attacker_stats->fixed.get_extra(LUCK);
	int attacked_luck = attacked_stats->fixed.get_extra(LUCK);

	bool is_blind = false;

	if (attacker->get_type() == wedge::Battle_Entity::PLAYER) {
		wedge::Player_Stats *stats = dynamic_cast<wedge::Battle_Player *>(attacker)->get_player_stats();
		attacker_luck += stats->weapon.stats.get_extra(LUCK);
		if (attacker_stats->status == STATUS_BLIND) {
			is_blind = true;
			attacker_luck = 0;
			attacked_luck += 1000000;
		}
	}
	if (attacked->get_type() == wedge::Battle_Entity::PLAYER) {
		wedge::Player_Stats *stats = dynamic_cast<wedge::Battle_Player *>(attacked)->get_player_stats();
		attacked_luck += stats->weapon.stats.get_extra(LUCK);
	}

	int luck_diff = attacker_luck - attacked_luck;
	float miss_max = is_blind ? 0.333f : 0.1f;
	float lucky_hit_max = 0.1f;

	if (luck_diff < 0) {
		float p = MIN(miss_max, fabsf((float)luck_diff) / attacker_stats->fixed.get_extra(LUCK));
		if (util::rand(0, 1000) / 1000.0f < p) {
			*miss = true;
		}
	}
	else if (luck_diff > 0) {
		float p = MIN(lucky_hit_max, fabsf((float)luck_diff) / attacked_stats->fixed.get_extra(LUCK));
		if (util::rand(0, 1000) / 1000.0f < p) {
			*lucky_hit = true;
		}
	}
}

void Monster_RPG_3_Battle_Game::show_retry_boss_gui()
{
	Yes_No_GUI *yes_no_gui = new Yes_No_GUI(GLOBALS->game_t->translate(1331)/* Originally: Retry boss? */, true, retry_boss_callback, NULL);
	yes_no_gui->hook_omnipresent(true);
	shim::guis.push_back(yes_no_gui);
}
	
void Monster_RPG_3_Battle_Game::set_highlighted_entities(std::vector<wedge::Battle_Entity *> entities)
{
	highlighted_entities = entities;
}

void Monster_RPG_3_Battle_Game::add_dialogue(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position)
{
	dialogue_tags.push_back(tag);
	dialogue_texts.push_back(text);
	dialogue_types.push_back(type);
	dialogue_positions.push_back(position);
}

bool Monster_RPG_3_Battle_Game::run()
{
	if (enemy_stats_shown && player_stats_shown) {
		started_taking_turns = true;
	}

	if (dialogue_tags.size() > 0 && waiting_for_hit == 0) {
		NEW_SYSTEM_AND_TASK(BATTLE)
		wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, this, new_task);
		ADD_STEP(g)
		ADD_TASK(new_task)
		FINISH_SYSTEM(BATTLE)

		GLOBALS->do_dialogue(dialogue_tags[0], dialogue_texts[0], dialogue_types[0], dialogue_positions[0], g);

		dialogue_tags.erase(dialogue_tags.begin());
		dialogue_texts.erase(dialogue_texts.begin());
		dialogue_types.erase(dialogue_types.begin());
		dialogue_positions.erase(dialogue_positions.begin());

		waiting_for_hit++;
	}
	
	return wedge::Battle_Game::run();
}

gfx::Image *Monster_RPG_3_Battle_Game::get_background()
{
	if (curr_bg_frame < 0 || curr_bg_frame >= (int)backgrounds.size()) {
		return NULL;
	}

	return backgrounds[curr_bg_frame];
}

SDL_Colour Monster_RPG_3_Battle_Game::get_tint()
{
	return tint;
}
