#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/battle_enemy.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/generic_callback.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/input.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/pause_task.h>

#include "battle_game.h"
#include "battle_player.h"
#include "battle_transition_in.h"
#include "battle_transition_out.h"
#include "globals.h"
#include "hit.h"

#define GAMEOVER_FADE_TIME 3000

Battle_Game::Battle_Game(std::string bg, int bg_delay) :
	wedge::Battle_Game(bg, bg_delay)
{
	start_delay(1000);
}

Battle_Game::~Battle_Game()
{
}

void Battle_Game::start_transition_in()
{
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

void Battle_Game::start_transition_out()
{
	NEW_SYSTEM_AND_TASK(this)
	Battle_Transition_Out_Step *step = new Battle_Transition_Out_Step(new_task);
	ADD_STEP(step)
	ADD_TASK(new_task)
	FINISH_SYSTEM(this)
}

wedge::Battle_Player *Battle_Game::create_player(int index)
{
	return new Battle_Player(); // only one player in this example, otherwise we'd use index
}

void Battle_Game::draw()
{
	backgrounds[0]->draw(get_offset());

	for (size_t i = 0; i < entities.size(); i++) {
		wedge::Battle_Entity *entity = entities[i];
		entity->draw();
	}

	std::map<std::string, int> enemies;

	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->get_type() == wedge::Battle_Entity::ENEMY) {
			wedge::Battle_Enemy *enemy = static_cast<wedge::Battle_Enemy *>(entities[i]);
			enemies[enemy->get_name()]++;
		}
	}

	// same as dialogue
	int BORDER = 3;
	int HEIGHT = (shim::font->get_height() + 3) * 2 + BORDER * 2;
	util::Size<int> PAD(3, 3);
	int y = shim::screen_size.h - 1 - PAD.h - HEIGHT;
	util::Point<int> text_pos = util::Point<int>(PAD.w + BORDER, y + BORDER);
	int win_w = (shim::screen_size.w-PAD.w*2)/2;
	SDL_Colour colour;
	int i = 0;

	if (enemy_stats_shown) {
		gfx::draw_filled_rectangle(shim::white, util::Point<int>(PAD.w, y), util::Size<int>(win_w, HEIGHT));
		gfx::draw_rectangle(shim::black, util::Point<int>(PAD.w, y), util::Size<int>(win_w, HEIGHT));

		std::map<std::string, int>::iterator it;
		for (it = enemies.begin(); it != enemies.end(); it++) {
			std::pair<std::string, int> p = *it;
			if (p.first == entities[turn]->get_name()) {
				colour = shim::palette[22];
			}
			else {
				colour = shim::black;
			}
			i++;
			shim::font->draw(colour, p.first, text_pos);
			std::string n_s = util::itos(p.second);
			int w = shim::font->get_text_width(n_s);
			shim::font->draw(colour, n_s, util::Point<int>(PAD.w + win_w - BORDER - w, text_pos.y));
			text_pos.y += shim::font->get_height() + 2;
		}
	}

	if (player_stats_shown) {
		gfx::draw_filled_rectangle(shim::white, util::Point<int>(PAD.w+win_w, y), util::Size<int>(win_w, HEIGHT));
		gfx::draw_rectangle(shim::black, util::Point<int>(PAD.w+win_w, y), util::Size<int>(win_w, HEIGHT));

		text_pos = util::Point<int>(PAD.w+win_w+BORDER, y + BORDER);

		std::vector<wedge::Battle_Entity *> players = get_players();

		for (size_t i = 0; i < players.size(); i++) {
			if (turn == (int)i) {
				colour = shim::palette[22];
			}
			else {
				colour = shim::black;
			}
			wedge::Base_Stats *stats = players[i]->get_stats();
			int w = shim::font->get_text_width("5555");
			std::string hp = util::itos(stats->hp) + "/";
			int w2 = shim::font->get_text_width(hp);
			shim::font->draw(colour, hp, util::Point<int>(PAD.w+win_w*2-BORDER-w-w2-1, text_pos.y));
			shim::font->draw(colour, util::itos(stats->fixed.max_hp), util::Point<int>(PAD.w+win_w*2-BORDER-w, text_pos.y));
			text_pos.y += shim::font->get_height() + 2;
		}
	}

	Game::draw();
	
	for (size_t i = 0; i < entities.size(); i++) {
		wedge::Battle_Entity *entity = entities[i];
		entity->draw_fore();
	}

	if (gameover) {
		Uint32 now = SDL_GetTicks();
		Uint32 end = gameover_time + wedge::globals->gameover_timeout;
		Uint32 diff;
		if (now > end) {
			diff = 0;
		}
		else {
			diff = end-now;
		}
		if (diff <= GAMEOVER_FADE_TIME) {
			float p = 1.0f - ((float)diff / GAMEOVER_FADE_TIME);
			SDL_Colour colour;
			colour.r = shim::palette[9].r * p;
			colour.g = shim::palette[9].g * p;
			colour.b = shim::palette[9].b * p;
			colour.a = shim::palette[9].a * p;
			gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
		}
	}
}

void Battle_Game::start_hit_effect(wedge::Battle_Entity *attacked)
{
	NEW_SYSTEM_AND_TASK(this)
	Hit_Step *hit = new Hit_Step(attacked, 0, new_task);
	ADD_STEP(hit)
	ADD_TASK(new_task)
	ANOTHER_TASK
	wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, this, new_task);
	hit->add_monitor(g);
	ADD_STEP(g)
	ADD_TASK(new_task)
	FINISH_SYSTEM(this)
}

void Battle_Game::show_retry_boss_gui()
{
}
