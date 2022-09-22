#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/map_entity.h"

using namespace wedge;

namespace wedge {

Area_Game::Area_Game() :
	pause_fade_time(250),
	change_area_fade_duration(2000),
	pause_end_time(0),
	pausing(false),
	paused(false),
	current_area(NULL),
	next_area(NULL),
	scrolling_in(false),
	fading_in(false),
	gameover(false),
	half_faded(false),
	pause_entity_movement_on_next_area_change(true),
	set_sfx_volume_to_zero(false),
	fade_sfx_volume(false)
{
	bak_sfx_volume = shim::sfx_volume;
}

Area_Game::~Area_Game()
{
	delete current_area;
}

bool Area_Game::start_area(Area *area)
{
	Game::start();

	bool success;

	if (area == NULL) {
		current_area = create_area("--start--");
		success = current_area->start();
		if (success) {
			current_area->set_hooks(get_area_hooks(current_area->get_name(), current_area));
		}
		return success;
	}
	else {
		current_area = area;
		success = true;
	}
	
	current_area->started();

	return success;
}

void Area_Game::handle_event(TGUI_Event *event)
{
	if (pausing == false && paused == false) {
		Game::handle_event(event);
	}

	if (current_area != NULL) {
		current_area->handle_event(event);
	}
}

bool Area_Game::run()
{
	if (scrolling_in) {
		scroll_offset += scroll_increment;
		for (size_t i = 1; i < players.size(); i++) {
			Map_Entity *p = players[i];
			util::Point<float> p_offset = p->get_offset();
			float speed = p->get_speed();
			if (!(p_offset.x != 0 && p_offset.y != 0)) {
				speed *= 2.0f;
			}
			if (p_offset.x < 0) {
				p_offset.x += speed;
				if (p_offset.x > 0) {
					p_offset.x = 0;
				}
			}
			else if (p_offset.x > 0) {
				p_offset.x -= speed;
				if (p_offset.x < 0) {
					p_offset.x = 0;
				}
			}
			if (p_offset.y < 0) {
				p_offset.y += speed;
				if (p_offset.y > 0) {
					p_offset.y = 0;
				}
			}
			else if (p_offset.y > 0) {
				p_offset.y -= speed;
				if (p_offset.y < 0) {
					p_offset.y = 0;
				}
			}
			p->set_offset(p_offset);
		}
		if (fabs(scroll_offset.x) >= 1.0f || fabs(scroll_offset.y) >= 1.0f) {
			util::Point<int> player_pos = players[0]->get_position();
			for (size_t i = 0; i < players.size(); i++) {
				if (i == 0) {
					players[i]->set_position(next_area_positions[0]);
				}
				else {
					util::Point<int> diff = players[i]->get_position() - player_pos;
					players[i]->set_position(next_area_positions[0] + diff);
				}
				players[i]->set_offset(util::Point<float>(0.0f, 0.0f));
				next_area->add_entity(players[i]);
				players[i]->set_area(next_area);
				players[i]->put_on_middle_layer();
				players[i]->set_direction(next_area_directions[0], true, false);
			}
			current_area->end();
			INSTANCE->saved_levels[current_area->get_name()] =
				util::string_printf("\"%s\": ", current_area->get_name().c_str()) + current_area->save(false);
			delete current_area;
			current_area = next_area;
			next_area = NULL;
			Map_Entity_Input_Step *input_step = players[0]->get_input_step();
			if (input_step != NULL) {
				input_step->pause_presses(false);
			}
			scrolling_in = false;
			current_area->started();
			// Don't repeat pressed if there is dialogue -- input is supposed to be cleared when dialogue starts (see M3 dialogue.cpp) and this repeat pressed is called AFTER that, which makes you move when the dialogue goes away
			if (input_step != NULL && GLOBALS->dialogue_active(AREA) == false) {
				input_step->repeat_pressed();
			}
			pause_entity_movement_on_next_area_change = true;
		}
		if (pause_entity_movement_on_next_area_change == false) {
			Game::run();
		}
		return true;
	}
	else if (fading_in) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - fade_start_time;
		float p;
		if ((int)elapsed >= change_area_fade_duration/2) {
			p = (elapsed - (change_area_fade_duration/2)) / (change_area_fade_duration/2.0f);
		}
		else {
			p = 1.0f - (elapsed / (change_area_fade_duration/2.0f));
		}
		p = MAX(0.0f, MIN(1.0f, p));
		if (fade_sfx_volume) {
			shim::sfx_volume = bak_sfx_volume * p;
		}
		if ((int)elapsed >= change_area_fade_duration/2 && half_faded == false) {
			half_faded = true;
			remove_player_from_current_area();
			for (size_t i = 0; i < players.size(); i++) {
				players[i]->set_position(next_area_positions[i]);
				players[i]->set_offset(util::Point<float>(0.0f, 0.0f));
				next_area->add_entity(players[i]);
				players[i]->set_area(next_area);
				players[i]->put_on_middle_layer();
				players[i]->set_direction(next_area_directions[i], true, false);
			}
			current_area->end();
			INSTANCE->saved_levels[current_area->get_name()] =
				util::string_printf("\"%s\": ", current_area->get_name().c_str()) + current_area->save(false).c_str();
			delete current_area;
			current_area = next_area;
			next_area = NULL;
			
			Map_Entity_Input_Step *input_step = players[0]->get_input_step();
			if (input_step != NULL) {
				input_step->pause_presses(false);
			}
			current_area->started();
			// Don't repeat pressed if there is dialogue -- input is supposed to be cleared when dialogue starts (see M3 dialogue.cpp) and this repeat pressed is called AFTER that, which makes you move when the dialogue goes away
			if (input_step != NULL && GLOBALS->dialogue_active(AREA) == false) {
				input_step->repeat_pressed();
			}
			pause_entity_movement_on_next_area_change = true;
		}
		if (pause_entity_movement_on_next_area_change == false || (int)elapsed >= change_area_fade_duration/2) {
			Game::run();
			if (current_area != NULL) {
				current_area->run();
			}
		}
		if ((int)elapsed >= change_area_fade_duration) {
			fading_in = false;
			half_faded = false;
			shim::sfx_volume = bak_sfx_volume;
			fade_sfx_volume = false;
		}
		return true;
	}
	else {
		bak_sfx_volume = shim::sfx_volume;

		bool ret = true;
	
		if (current_area != NULL) {
			current_area->run();
		}

		if (pausing == false && paused == false) {
			ret = Game::run();
		}
		else if (pausing) {
			Uint32 now = GET_TICKS();
			Uint32 diff = now - pause_start_time;
			if ((int)diff >= pause_fade_time) {
				pausing = false;
				paused = true;
				if (pause_is_for_menu) {
					MENU = create_menu();
					if (MENU) {
						MENU->start();
					}
					else {
						end_menu();
					}
				}
				else {
					SHOP = create_shop(shop_type, shop_items);
					if (SHOP) {
						SHOP->start();
					}
					else {
						end_shop();
					}
				}
			}
		}

		if (current_area) {
			std::string next_area_name;
			if (current_area->exit_gameplay()) {
				return false;
			}
			else if ((next_area_name = current_area->get_next_area_name()) != "") {
				current_area->disconnect_player_input_step();

				next_area_positions = current_area->get_next_area_positions();
				next_area_directions = current_area->get_next_area_directions();
				scrolling_in = current_area->get_next_area_scrolls_in();
				fading_in = scrolling_in == false;
				std::map<std::string, std::string>::iterator it;
				SDL_RWops *file = NULL;
				util::JSON *json = NULL;
				if ((it = INSTANCE->saved_levels.find(next_area_name)) != INSTANCE->saved_levels.end()) {
					std::pair<std::string, std::string> p = *it;
					std::string save = "{ " + p.second + " }";
					file = SDL_RWFromMem((void *)save.c_str(), (int)save.length());
					json = new util::JSON(file);
					util::JSON::Node *n = json->get_root()->find("\"" + next_area_name + "\"");
					next_area = create_area(n);
				}
				else {
					next_area = create_area(next_area_name);
				}

				if (fading_in) {
					bak_sfx_volume = shim::sfx_volume;
					if (set_sfx_volume_to_zero) {
						shim::sfx_volume = 0.0f;
						set_sfx_volume_to_zero = false;
					}
				}

				bool success = next_area->start();
				if (success) {
					next_area->set_hooks(get_area_hooks(next_area->get_name(), next_area));
				}

				if (current_area->has_battles() == false && next_area->has_battles() == true) {
					INSTANCE->step_count = 0;
				}

				if (json != NULL) {
					delete json;
					SDL_RWclose(file);
				}

				Map_Entity_Input_Step *input_step = players[0]->get_input_step();

				if (input_step != NULL) {
					input_step->pause_presses(true);
				}

				if (scrolling_in) {
					remove_player_from_current_area();

					scroll_offset = util::Point<float>(0.0f, 0.0f);
					float speed = players[0]->get_speed();
					
					for (size_t i = 0; i < players.size(); i++) {
						Map_Entity *player = players[i];
						gfx::Sprite *sprite = player->get_sprite();
						switch (next_area_directions[0]) {
							case DIR_N:
								scroll_increment = util::Point<float>(0.0f, -speed);
								sprite->set_animation("walk_n");
								break;
							case DIR_E:
								scroll_increment = util::Point<float>(speed, 0.0f);
								sprite->set_animation("walk_e");
								break;
							case DIR_S:
								scroll_increment = util::Point<float>(0.0f, speed);
								sprite->set_animation("walk_s");
								break;
							default: // case DIR_W:
								scroll_increment = util::Point<float>(-speed, 0.0f);
								sprite->set_animation("walk_w");
								break;
						}
					}

					for (size_t i = 1; i < players.size(); i++) {
						Map_Entity *p = players[i];
						util::Point<int> prev_pos = players[i-1]->get_position();
						util::Point<int> pos = p->get_position();
						bool swap_pos = false;
						util::Point<int> needed_offset;
						std::string anim;
						switch (next_area_directions[0]) {
							case DIR_N:
								if (pos.y < prev_pos.y) {
									swap_pos = true;
									needed_offset = util::Point<int>(0, 2);
									anim = "walk_s";
								}
								break;
							case DIR_E:
								if (pos.x > prev_pos.x) {
									swap_pos = true;
									needed_offset = util::Point<int>(-2, 0);
									anim = "walk_w";
								}
								break;
							case DIR_S:
								if (pos.y > prev_pos.y) {
									swap_pos = true;
									needed_offset = util::Point<int>(0, -2);
									anim = "walk_n";
								}
								break;
							case DIR_W:
								if (pos.x < prev_pos.x) {
									swap_pos = true;
									needed_offset = util::Point<int>(2, 0);
									anim = "walk_e";
								}
								break;
							default:
								break;
						}
						if (swap_pos == false) {
							if ((next_area_directions[0] == DIR_S || next_area_directions[0] == DIR_N) && pos.y == prev_pos.y) {
								swap_pos = true;
								if (next_area_directions[0] == DIR_N) {
									needed_offset = util::Point<int>(prev_pos.x-pos.x, 1);
								}
								else {
									needed_offset = util::Point<int>(prev_pos.x-pos.x, -1);
								}
								if (prev_pos.x < pos.x) {
									anim = "walk_w";
								}
								else {
									anim = "walk_e";
								}
							}
							else if ((next_area_directions[0] == DIR_E || next_area_directions[0] == DIR_W) && pos.x == prev_pos.x) {
								swap_pos = true;
								if (next_area_directions[0] == DIR_W) {
									needed_offset = util::Point<int>(1, prev_pos.y-pos.y);
								}
								else {
									needed_offset = util::Point<int>(-1, prev_pos.y-pos.y);
								}
								if (prev_pos.y < pos.y) {
									anim = "walk_n";
								}
								else {
									anim = "walk_s";
								}
							}
						}
						if (swap_pos) {
							p->set_offset(-needed_offset);
							p->set_position(pos + needed_offset);
							p->get_sprite()->set_animation(anim);
						}
					}
				}
				else if (fading_in) {
					fade_start_time = GET_TICKS();
				}
			}
		}

		if (gameover && GET_TICKS() >= gameover_time + globals->gameover_timeout) {
			return false;
		}

		return ret;
	}
}

void Area_Game::start_pause()
{
	pausing = true;
	pause_start_time = GET_TICKS();
	shim::music->set_master_volume(shim::music_volume/3.333f);
	Uint32 played_time = pause_start_time - INSTANCE->play_start;
	INSTANCE->play_time += (played_time / 1000);
	current_area->set_entities_standing();
}

void Area_Game::end_pause()
{
	paused = false;
	pause_end_time = GET_TICKS();
	shim::music->set_master_volume(shim::music_volume);
	INSTANCE->play_start = pause_end_time;
}

void Area_Game::start_menu()
{
	pause_is_for_menu = true;
	start_pause();
}

void Area_Game::end_menu()
{
	end_pause();
}

void Area_Game::start_shop(Object_Type type, std::vector<Object> items)
{
	pause_is_for_menu = false;
	shop_type = type;
	shop_items = items;
	start_pause();
}

void Area_Game::end_shop()
{
	end_pause();
}

Area *Area_Game::get_current_area()
{
	return current_area;
}

Map_Entity *Area_Game::get_player(int index)
{
	return players[index];
}

std::vector<Map_Entity *> Area_Game::get_players()
{
	return players;
}

void Area_Game::set_player(int index, Map_Entity *player)
{
	for (int i = (int)players.size(); i < index+1; i++) {
		players.push_back(NULL);
	}
	players[index] = player;
}

void Area_Game::set_players(std::vector<Map_Entity *> players)
{
	this->players = players;
}

void Area_Game::set_gameover(bool gameover)
{
	this->gameover = gameover;
	if (gameover) {
		gameover_time = GET_TICKS();
	}
}

void Area_Game::lost_device()
{
	Game::lost_device();

	if (current_area != NULL) {
		current_area->lost_device();
	}
	if (next_area != NULL) {
		next_area->lost_device();
	}
}

void Area_Game::found_device()
{
	Game::found_device();

	if (current_area != NULL) {
		current_area->found_device();
	}
	if (next_area != NULL) {
		next_area->found_device();
	}
}

void Area_Game::resize(util::Size<int> new_size)
{
	Game::resize(new_size);

	if (current_area != NULL) {
		current_area->resize(new_size);
	}
	if (next_area != NULL) {
		next_area->resize(new_size);
	}
}

void Area_Game::remove_player_from_current_area()
{
	Map_Entity_List &entities = current_area->get_entities();
	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end();) {
		Map_Entity *entity = *it;
		if (std::find(players.begin(), players.end(), entity) != players.end()) {
			it = entities.erase(it);
		}
		else {
			it++;
		}
	}
}

Game *Area_Game::create_menu()
{
	return NULL;
}

Game *Area_Game::create_shop(Object_Type type, std::vector<Object> items)
{
	return NULL;
}

Map_Entity *Area_Game::create_entity(std::string type, util::JSON::Node *json)
{
	return NULL;
}

Map_Entity *Area_Game::create_player(std::string entity_name)
{
	return new Map_Entity(entity_name);
}

Area *Area_Game::create_area(std::string name)
{
	return new Area(name);
}

Area *Area_Game::create_area(util::JSON::Node *json)
{
	return new Area(json);
}

void Area_Game::draw_fore()
{
	Game::draw_fore();
}

void Area_Game::battle_ended(Battle_Game *battle)
{
	current_area->battle_ended(battle);
}

void Area_Game::set_pause_entity_movement_on_next_area_change(bool pause)
{
	pause_entity_movement_on_next_area_change = pause;
}

bool Area_Game::get_pause_entity_movement_on_next_area_change()
{
	return pause_entity_movement_on_next_area_change;
}

bool Area_Game::is_pausing()
{
	return pausing;
}

bool Area_Game::is_paused()
{
	return paused;
}

void Area_Game::set_sfx_volume_to_zero_next_area_fade(bool set)
{
	set_sfx_volume_to_zero = set;
}

void Area_Game::set_fade_sfx_volume(bool fade)
{
	fade_sfx_volume = fade;
}

bool Area_Game::changing_areas()
{
	return fading_in || scrolling_in;
}

}
