#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/battle_enemy.h>
#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/battle_player.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/generic_callback.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/inventory.h>
#include <Nooskewl_Wedge/omnipresent.h>
#include <Nooskewl_Wedge/spells.h>
#include <Nooskewl_Wedge/special_number.h>

#include "achievements.h"
#include "battle_game.h"
#include "battle_player.h"
#include "dialogue.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "inventory.h"
#include "milestones.h"
#include "monster_rpg_3.h"
#include "ran_away.h"
#include "settings.h"
#include "vampires.h"
#include "widgets.h"

#define SELECT_A_TARGET GLOBALS->game_t->translate(1332)/* Originally: Select a target... */
#define SELECT_AN_ITEM GLOBALS->game_t->translate(1333)/* Originally: Select an item... */
#define SELECT_A_SPELL GLOBALS->game_t->translate(1334)/* Originally: Select a spell... */
#define SELECT_A_VAMPIRE GLOBALS->game_t->translate(1335)/* Originally: Select a vampire... */
#define SELECT_AN_ACTION GLOBALS->game_t->translate(1336)/* Originally: Select an action... */

static bool enemy_x_compare(wedge::Battle_Entity *a, wedge::Battle_Entity *b)
{
	wedge::Battle_Enemy *e1 = dynamic_cast<wedge::Battle_Enemy *>(a);
	wedge::Battle_Enemy *e2 = dynamic_cast<wedge::Battle_Enemy *>(b);

	if (e1 == NULL || e2 == NULL) {
		return false;
	}

	util::Point<int> pos1 = e1->get_position();
	pos1.x += e1->get_sprite()->get_current_image()->size.w;
	util::Point<int> pos2 = e2->get_position();
	pos2.x += e2->get_sprite()->get_current_image()->size.w;

	if (pos1.x > pos2.x) {
		return true;
	}
	else if (pos1.x < pos2.x) {
		return false;
	}
	else {
		return pos1.y <= pos2.y;
	}
}

static bool is_tutorial_notification(std::string notification)
{
	return notification == SELECT_A_TARGET || notification == SELECT_AN_ITEM || notification == SELECT_AN_ACTION || notification == SELECT_A_SPELL || notification == SELECT_A_VAMPIRE;
}

static void action_callback(void *data)
{
	Battle_Multiple_Choice_GUI::Callback_Data *d = (Battle_Multiple_Choice_GUI::Callback_Data *)data;
	Monster_RPG_3_Battle_Player *p = (Monster_RPG_3_Battle_Player *)d->userdata;
	p->set_action(d->choice);
}

static void item_callback(void *data)
{
	Battle_List_GUI::Callback_Data *d = (Battle_List_GUI::Callback_Data *)data;
	Monster_RPG_3_Battle_Player *p = (Monster_RPG_3_Battle_Player *)d->userdata;
	p->set_item_index(d->choice, d->cancelled);
}

static void spell_callback(void *data)
{
	Battle_List_GUI::Callback_Data *d = (Battle_List_GUI::Callback_Data *)data;
	Monster_RPG_3_Battle_Player *p = (Monster_RPG_3_Battle_Player *)d->userdata;
	p->set_spell_index(d->choice, d->cancelled);
}

static void spell_effect_callback(void *data)
{
	Monster_RPG_3_Battle_Player *p = static_cast<Monster_RPG_3_Battle_Player *>(data);
	p->set_spell_effect_done();
}

static void vampire_callback(void *data)
{
	Battle_List_GUI::Callback_Data *d = (Battle_List_GUI::Callback_Data *)data;
	Monster_RPG_3_Battle_Player *p = (Monster_RPG_3_Battle_Player *)d->userdata;
	p->set_vampire_index(d->choice, d->cancelled);
}

static void vampire_effect_callback(void *data)
{
	Monster_RPG_3_Battle_Player *p = static_cast<Monster_RPG_3_Battle_Player *>(data);
	p->set_vampire_effect_done();
}

static void spell_mouse_callback(void *data)
{
	Button_GUI::Callback_Data *d = static_cast<Button_GUI::Callback_Data *>(data);
	Monster_RPG_3_Battle_Player *p = static_cast<Monster_RPG_3_Battle_Player *>(d->userdata);
	p->cancel_tutorial_notification();
	if (d->cancelled) {
		p->start_selecting_action();
		p->set_ignore_next_escape(true);
	}
	else {
		p->cast_spell_now();
	}
}

static void vampire_mouse_callback(void *data)
{
	Button_GUI::Callback_Data *d = static_cast<Button_GUI::Callback_Data *>(data);
	Monster_RPG_3_Battle_Player *p = static_cast<Monster_RPG_3_Battle_Player *>(d->userdata);
	p->cancel_tutorial_notification();
	if (d->cancelled) {
		p->start_selecting_action();
		p->set_ignore_next_escape(true);
	}
	else {
		p->cast_vampire_now();
	}
}

static void player_stats_callback(void *data)
{
	Player_Stats_GUI::Callback_Data *d = static_cast<Player_Stats_GUI::Callback_Data *>(data);
	Monster_RPG_3_Battle_Player *p = static_cast<Monster_RPG_3_Battle_Player *>(d->userdata);
	p->end_player_stats_gui(d->cancelled);
}

static void vfire_callback(void *data)
{
	BATTLE->hit_done();
	Monster_RPG_3_Battle_Player *p = static_cast<Monster_RPG_3_Battle_Player *>(data);
	p->start_selecting_action();
}

Monster_RPG_3_Battle_Player::Monster_RPG_3_Battle_Player(int index) :
	wedge::Battle_Player(index == ENY ? "Eny" : "Tiggy", index),
	walk_speed(0.05f),
	turn_started(false),
	turn_done(false),
	current_action(UNINITIALIZED),
	walk_offset(0.0f, 0.0f),
	item_selector_up(false),
	showed_run_message(false),
	action_gui(NULL),
	item_gui(NULL),
	set_item_gui_null(false),
	running(false),
	spell_selector_up(false),
	set_spell_gui_null(false),
	spell_gui(NULL),
	spell_effect_done(0),
	vampire_selector_up(false),
	set_vampire_gui_null(false),
	vampire_gui(NULL),
	vampire_effect_done(0),
	ignore_next_escape(false),
	player_stats_gui(NULL),
	last_action_gui_selected(0),
	talked_about_vfire(false)
{
	if (player_stats->weapon.id == wedge::WEAPON_NONE) {
		weapon_sprite = NULL;
	}
	else {
		weapon_sprite = OBJECT->get_sprite(OBJECT->make_object(wedge::OBJECT_WEAPON, player_stats->weapon.id, 1));
	}
}

Monster_RPG_3_Battle_Player::~Monster_RPG_3_Battle_Player()
{
	if (action_gui != NULL) {
		action_gui->exit();
	}
	delete weapon_sprite;
}

bool Monster_RPG_3_Battle_Player::start()
{
	NEW_SYSTEM_AND_TASK(BATTLE)
	ran_away_step = new Ran_Away_Step(new_task);
	ADD_STEP(ran_away_step)
	ADD_TASK(new_task)
	FINISH_SYSTEM(BATTLE)

	return wedge::Battle_Player::start();
}

void Monster_RPG_3_Battle_Player::handle_event(TGUI_Event *event)
{
	if (
		item_gui == NULL && spell_gui == NULL && vampire_gui == NULL && ignore_next_escape == false &&
		(
			current_action == NONE ||
			current_action == DEFEND ||
			current_action == RUN ||
			(current_action == ATTACK && attack_phase >= ATTACK_WALK_FORWARD) ||
			(current_action == ITEM && item_phase >= ITEM_WALK_FORWARD) ||
			(current_action == SPELL && spell_phase >= SPELL_WALK_FORWARD) ||
			(current_action == VAMPIRE && vampire_phase >= VAMPIRE_WALK_FORWARD)
		) &&
		(
			(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
			(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)
		)
	) {
		GLOBALS->mini_pause();
		return;
	}

	if (GLOBALS->dialogue_active(BATTLE)) {
		return;
	}

	if (enemies_all_dead()) {
		return;
	}
	
	if (
		current_action == NONE && turn_started == true && enemies_all_dead() == false &&
		((event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_b4) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b4))
	) {
		show_settings();
	}
	else if (settings_active()) {
		return;
	}
	
	std::vector<wedge::Battle_Entity *> enemies = BATTLE->get_enemies();

	if (
		(current_action == ATTACK && attack_phase == ATTACK_SELECT_TARGET) ||
		(current_action == ITEM && item_phase == ITEM_SELECT_TARGET)
	) {
		std::vector<wedge::Battle_Entity *> entities = get_entities_that_can_be_selected();
		bool do_attack = false;
		bool do_item = false;
		if (event->type == TGUI_MOUSE_DOWN) {
			for (size_t i = 0; i < entities.size(); i++) {
				wedge::Battle_Entity *entity = entities[i];
				gfx::Sprite *sprite = entity->get_sprite();
				gfx::Image *image = sprite->get_current_image();
				int o;
				util::Point<int> topleft, bottomright;
				image->get_bounds(topleft, bottomright);
				if (entity->get_type() == wedge::Battle_Entity::ENEMY) {
					o = -(bottomright.x-topleft.x);
				}
				else {
					o = 0;
				}
				util::Point<int> pos = entity->get_decoration_offset(0, util::Point<int>(o, 0), NULL);
				if (event->mouse.x >= pos.x && event->mouse.y >= pos.y && event->mouse.x < pos.x+(bottomright.x-topleft.x) && event->mouse.y < pos.y+(bottomright.y-topleft.y)) {
					target = (int)i;
					if (current_action == ATTACK) {
						do_attack = true;
					}
					else {
						do_item = true;
					}
					M3_GLOBALS->button->play(false);
					cancel_tutorial_notification();
					break;
				}
			}
		}
		else if (
			(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b1) ||
			(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b1)
		) {
			M3_GLOBALS->button->play(false);
			if (current_action == ATTACK) {
				do_attack = true;
			}
			else if (current_action == ITEM) {
				do_item = true;
			}
			cancel_tutorial_notification();
		}
		else if (
			(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
			(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)
		) {
			M3_GLOBALS->button->play(false);
			//target--; ??
			start_selecting_action();
			return;
		}
		else if (event->type == TGUI_FOCUS) {
			if (event->focus.type == TGUI_FOCUS_UP || event->focus.type == TGUI_FOCUS_LEFT) {
				shim::widget_mml->play(false);
				if (current_action == ATTACK) {
					target++;
					target %= entities.size();
				}
				else {
					if (target == 0) {
						target = (int)MAX_PARTY;
					}
					else if (target < (int)MAX_PARTY) {
						target--;
					}
					else {
						target++;
						if (target == (int)entities.size()) {
							target = (int)MAX_PARTY-1;
						}
					}
				}
			}
			else if (event->focus.type == TGUI_FOCUS_DOWN || event->focus.type == TGUI_FOCUS_RIGHT) {
				shim::widget_mml->play(false);
				if (current_action == ATTACK) {
					target--;
					if (target < 0) {
						target = (int)entities.size()-1;
					}
				}
				else {
					if (target <= (int)MAX_PARTY-2) {
						target++;
					}
					else if (target == (int)MAX_PARTY-1) {
						target = (int)entities.size()-1;
					}
					else {
						target--;
						if (target < (int)MAX_PARTY) {
							target = 0;
						}
					}
				}
			}
		}
		if (do_attack) {
			attack_phase = ATTACK_WALK_FORWARD;
			walk_offset = util::Point<float>(0.0f, 0.0f);
			sprite->set_animation("walk_w");
		}
		else if (do_item) {
			item_phase = ITEM_WALK_FORWARD;
			walk_offset = util::Point<float>(0.0f, 0.0f);
			sprite->set_animation("walk_w");
		}
		std::vector<Battle_Entity *> highlighted;
		highlighted.push_back(entities[target]);
		M3_BATTLE->set_highlighted_entities(highlighted);
	}
	else if (
		(current_action == SPELL && spell_phase == SPELL_SELECT_TARGET) ||
		(current_action == VAMPIRE && vampire_phase == VAMPIRE_SELECT_TARGET)
	) {
		std::vector<Battle_Entity *> &targets = (current_action == SPELL) ? spell_targets : vampire_targets;
		std::vector<wedge::Battle_Entity *> entities = get_entities_that_can_be_selected();
		std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();
		std::vector<wedge::Battle_Entity *> enemies = BATTLE->get_enemies();
		if (event->type == TGUI_MOUSE_DOWN) {
			for (size_t i = 0; i < entities.size(); i++) {
				wedge::Battle_Entity *entity = entities[i];
				gfx::Sprite *sprite = entity->get_sprite();
				gfx::Image *image = sprite->get_current_image();
				int o;
				util::Point<int> topleft, bottomright;
				image->get_bounds(topleft, bottomright);
				if (entity->get_type() == wedge::Battle_Entity::ENEMY) {
					o = -(bottomright.x-topleft.x);
				}
				else {
					o = 0;
				}
				util::Point<int> pos = entity->get_decoration_offset(0, util::Point<int>(o, 0), NULL);
				if (event->mouse.x >= pos.x && event->mouse.y >= pos.y && event->mouse.x < pos.x+(bottomright.x-topleft.x) && event->mouse.y < pos.y+(bottomright.y-topleft.y)) {
					M3_GLOBALS->button->play(false);
					std::vector<wedge::Battle_Entity *>::iterator it;
					if ((it = std::find(targets.begin(), targets.end(), entity)) == targets.end()) {
						targets.push_back(entity);
					}
					else {
						targets.erase(it);
					}
				}
			}
		}
		else {
			// -2 is all players if both selectable ELSE not used
			// -1 is all enemies if both selectable ELSE all entities
			if (event->type == TGUI_FOCUS) {
				if (event->focus.type == TGUI_FOCUS_UP || event->focus.type == TGUI_FOCUS_LEFT) {
					shim::widget_mml->play(false);
					if (target == -2) {
						target = (int)MAX_PARTY-1;
					}
					else if (target == -1) {
						if (can_select_players && can_select_enemies) {
							target = -2;
						}
						else if (can_select_players) {
							target = (int)MAX_PARTY-1;
						}
						else {
							target = 0;
						}
					}
					else if (can_select_enemies && target == (int)entities.size()-1) {
						if (enemies.size() == 1) {
							if (can_select_players) {
								target = -2;
							}
							else {
								// only entity already selected
							}
						}
						else {
							target = -1;
						}
					}
					else if (can_select_players && target == 0) {
						if (can_select_enemies) {
							target = (int)MAX_PARTY;
						}
						else {
							target = -1;
						}
					}
					else if (can_select_players && target < (int)MAX_PARTY) {
						target--;
					}
					else {
						target++;
					}
				}
				else if (event->focus.type == TGUI_FOCUS_DOWN || event->focus.type == TGUI_FOCUS_RIGHT) {
					shim::widget_mml->play(false);
					if (target == -2) {
						if (enemies.size() == 1) {
							target = (int)entities.size()-1;
						}
						else {
							target = -1;
						}
					}
					else if (target == -1) {
						if (can_select_enemies == false) {
							target = 0;
						}
						else {
							target = (int)entities.size() - 1;
						}
					}
					else if (can_select_players && target == (int)MAX_PARTY) {
						target = 0;
					}
					else if (can_select_players && target == (int)MAX_PARTY-1) {
						if (can_select_enemies) {
							target = -2;
						}
						else {
							target = -1;
						}
					}
					else if (can_select_players && target < (int)MAX_PARTY) {
						target++;
					}
					else if (can_select_players == false && target == 0) {
						target = -1;
					}
					else {
						target--;
					}
				}
				targets.clear();
				if (target == -1) {
					// select all
					if (can_select_players && can_select_enemies) {
						// select all enemies
						targets.insert(targets.begin(), enemies.begin(), enemies.end());
					}
					else {
						targets.insert(targets.begin(), entities.begin(), entities.end());
					}
				}
				else if (target == -2) {
					// select all players
					targets.insert(targets.begin(), players.begin(), players.end());
				}
				else {
					targets.push_back(entities[target]);
				}
			}
		}
		M3_BATTLE->set_highlighted_entities(targets);
	}
	else {
		std::vector<wedge::Battle_Entity *> highlighted;
		M3_BATTLE->set_highlighted_entities(highlighted);
		if (event->type == TGUI_MOUSE_DOWN) {
			if (action_gui != NULL) {
				std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();
				for (size_t i = 0; i < players.size(); i++) {
					Monster_RPG_3_Battle_Player *player = static_cast<Monster_RPG_3_Battle_Player *>(players[i]);
					gfx::Image *current_image = player->get_sprite()->get_current_image();
					util::Size<int> offset = (util::Size<int>(shim::tile_size, shim::tile_size) - current_image->size) / 2;
					util::Point<float> pos = player->get_draw_pos() + util::Point<int>(offset.w, offset.h);
					util::Size<int> size = current_image->size;
					if (event->mouse.x >= pos.x && event->mouse.y >= pos.y && event->mouse.x < pos.x+size.w && event->mouse.y < pos.y+size.h) {
						show_player_stats((int)i);
						break;
					}
				}
			}
		}
		else if (current_action == PLAYER_SELECT) {
			if (player_stats_gui == NULL) {
				if (
					(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b1) ||
					(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b1)
				) {
					show_player_stats(selected_player);
				}
				else if (event->type == TGUI_FOCUS) {
					if (event->focus.type == TGUI_FOCUS_UP || event->focus.type == TGUI_FOCUS_LEFT) {
						selected_player--;
						if (selected_player < 0) {
							selected_player = 0;
						}
						else {
							shim::widget_mml->play(false);
						}
					}
					else if (event->focus.type == TGUI_FOCUS_DOWN || event->focus.type == TGUI_FOCUS_RIGHT) {
						shim::widget_mml->play(false);
						selected_player++;
						if (selected_player >= (int)MAX_PARTY) {
							start_selecting_action();
						}
					}
				}
			}
		}
		else if (current_action == NONE) {
			if (event->type == TGUI_FOCUS && event->focus.type == TGUI_FOCUS_UP) {
				if (action_gui != NULL) {
					int selected = action_gui->get_selected();
					if (selected == 0 && last_action_gui_selected == 0) {
						shim::widget_mml->play(false);
						current_action = PLAYER_SELECT;
						selected_player = (int)MAX_PARTY-1;
						action_gui->exit();
						action_gui = NULL;
						BATTLE->show_enemy_stats(true);
						BATTLE->show_player_stats(true);
					}
				}
			}
		}
	}

	// must come after bringing the item gui up above in this function
	if (set_item_gui_null) {
		set_item_gui_null = false;
		item_gui = NULL;
	}
	// this too
	if (set_spell_gui_null) {
		set_spell_gui_null = false;
		spell_gui = NULL;
	}
	// this too
	if (set_vampire_gui_null) {
		set_vampire_gui_null = false;
		vampire_gui = NULL;
	}

	// events can be processed in GUIs and then come here in the same frame, some extra handling may be required (below)

	ignore_next_escape = false;

	if (action_gui != NULL) {
		last_action_gui_selected = action_gui->get_selected();
	}
	else {
		last_action_gui_selected = -1;
	}
}

void Monster_RPG_3_Battle_Player::draw()
{
	gfx::Image *current_image = sprite->get_current_image();
	util::Size<int> offset = (util::Size<int>(shim::tile_size, shim::tile_size) - current_image->size) / 2;
	util::Point<float> pos = get_draw_pos() + util::Point<int>(offset.w, offset.h);

	current_image->draw(pos);

	if (weapon_sprite != NULL && current_action == ATTACK && attack_phase == ATTACK_ANIMATION) {
		weapon_sprite->sync_with(sprite);
		util::Size<int> offset = weapon_sprite->get_current_image()->size - current_image->size;
		weapon_sprite->get_current_image()->draw(pos - offset);
	}
}

void Monster_RPG_3_Battle_Player::draw_fore()
{
	if (
		(current_action == ATTACK && attack_phase == ATTACK_SELECT_TARGET) ||
		(current_action == ITEM && item_phase == ITEM_SELECT_TARGET)
	) {
		std::vector<wedge::Battle_Entity *> entities = get_entities_that_can_be_selected();
		M3_BATTLE->draw_selection_arrow(entities[target]);
	}
	else if (current_action == SPELL && spell_phase == SPELL_SELECT_TARGET) {
		for (size_t i = 0; i < spell_targets.size(); i++) {
			M3_BATTLE->draw_selection_arrow(spell_targets[i]);
		}
	}
	else if (current_action == VAMPIRE && vampire_phase == VAMPIRE_SELECT_TARGET) {
		for (size_t i = 0; i < vampire_targets.size(); i++) {
			M3_BATTLE->draw_selection_arrow(vampire_targets[i]);
		}
	}
	else if (current_action == PLAYER_SELECT)
	{
		std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();
		Monster_RPG_3_Battle_Player *player = static_cast<Monster_RPG_3_Battle_Player *>(players[selected_player]);
		gfx::Image *current_image = player->get_sprite()->get_current_image();
		util::Size<int> offset = (util::Size<int>(shim::tile_size, shim::tile_size) - current_image->size) / 2;
		util::Point<float> pos = player->get_draw_pos() + util::Point<int>(offset.w, offset.h);
		float p;
		const int duration = 1000;
		const int half = duration / 2;
		Uint32 ticks = GET_TICKS() % duration;
		if (ticks >= half) {
			p = 1.0f - ((ticks-half) / (float)half);
		}
		else {
			p = ticks / (float)half;
		}
		SDL_Colour colour = shim::palette[12];
		colour.r *= p;
		colour.g *= p;
		colour.b *= p;
		colour.a = p * 255;
		gfx::draw_rectangle(colour, pos, current_image->size);
	}


	util::Size<int> PAD(shim::screen_size.w*0.03f, shim::screen_size.h*0.03f);
	int HEIGHT = shim::font->get_height() * 2 + 4 + Dialogue_Step::BORDER * 2;
	int y = shim::screen_size.h - HEIGHT - PAD.h - 1;
	int WIN_H = shim::font->get_height() + Dialogue_Step::BORDER * 2;
	y = y - WIN_H + 1;
	util::Point<int> text_pos(PAD.w + Dialogue_Step::BORDER, y+Dialogue_Step::BORDER);

	float little = get_little_bander_offset();

	if (spell_selector_up) {
		std::string text = GLOBALS->game_t->translate(1337)/* Originally: MP */ + TAG_END + util::itos(stats->mp) + "/" + util::itos(stats->fixed.max_mp);
		int win_w = Dialogue_Step::BORDER * 2 + 1 + shim::font->get_text_width(text);

		SDL_Colour *colours = start_bander(num_bands(WIN_H-4), shim::palette[24], shim::palette[22]);
		gfx::draw_filled_rectangle(colours, util::Point<float>(PAD.w+2-little, y+2-little), util::Size<float>(win_w-4+little*2.0f, WIN_H-4+little*2.0f));
		end_bander();
		gfx::draw_rectangle(shim::palette[24], util::Point<int>(PAD.w, y)+util::Point<int>(1, 1), util::Size<int>(win_w, WIN_H)-util::Size<int>(2, 2));
		gfx::draw_rectangle(shim::palette[27], util::Point<int>(PAD.w, y), util::Size<int>(win_w, WIN_H));

		shim::font->enable_shadow(shim::palette[24], gfx::Font::DROP_SHADOW);
		shim::font->draw(shim::white, text, text_pos);
		shim::font->disable_shadow();
	}

	if (vampire_selector_up) {
		int vampire = vampire_gui->get_selected();
		int hp, mp;
		get_vampire_cost(M3_INSTANCE->vampire(vampire), hp, mp);
		std::string text = util::itos(hp) + " " + GLOBALS->game_t->translate(1338)/* Originally: HP */ + "/" + util::itos(mp) + " " + GLOBALS->game_t->translate(1337)/* Originally: MP */;
		int win_w = Dialogue_Step::BORDER * 2 + 1 + shim::font->get_text_width(text);

		SDL_Colour *colours = start_bander(num_bands(WIN_H-4), shim::palette[24], shim::palette[22]);
		gfx::draw_filled_rectangle(colours, util::Point<float>(PAD.w+2-little, y+2-little), util::Size<float>(win_w-4+little*2.0f, WIN_H-4+little*2.0f));
		end_bander();
		gfx::draw_rectangle(shim::palette[24], util::Point<int>(PAD.w, y)+util::Point<int>(1, 1), util::Size<int>(win_w, WIN_H)-util::Size<int>(2, 2));
		gfx::draw_rectangle(shim::palette[27], util::Point<int>(PAD.w, y), util::Size<int>(win_w, WIN_H));

		shim::font->enable_shadow(shim::palette[24], gfx::Font::DROP_SHADOW);
		shim::font->draw(shim::white, text, text_pos);
		shim::font->disable_shadow();
	}
}

bool Monster_RPG_3_Battle_Player::take_turn()
{
	if (name == "Eny" && INSTANCE->is_milestone_complete(MS_CAN_USE_VFIRE)) {
		if (talked_about_vfire && GLOBALS->dialogue_active(BATTLE) == true) {
			return false;
		}
		else if (talked_about_vfire == false) {
			talked_about_vfire = true;
			NEW_SYSTEM_AND_TASK(BATTLE)
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1664)/* Originally: Tiggy... you know what we have to do! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1665)/* Originally: *nods* */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(BATTLE)
			return false;
		}
		else if (turn_started == false) {
			start_selecting_action();
		}
	}
	else if (turn_started == false) {
		if (BATTLE->get_enemy_stats_shown() == false && BATTLE->get_player_stats_shown() == false) {
			return false; // transitioning into battle
		}
		start_selecting_action();
	}

	if (turn_done) {
		turn_started = false;
		turn_done = false;
		if (gfx::get_current_notification() != "") {
			// This happens when a strength/weakness was found with attack
			BATTLE->start_delay(2000);
		}
		return true; // when totally done
	}
	else if (current_action != NONE) {
		handle_action();
		return false;
	}
	else {
		return false;
	}
}

void Monster_RPG_3_Battle_Player::set_action(int choice)
{
	action_gui = NULL;

	cancel_tutorial_notification();

	if (player_stats->base.num_spells() == 0) { // adjust for missing "Spell" option
		if (choice >= 2) {
			choice++;
		}
	}

	if (M3_INSTANCE->num_vampires() == 0) { // adjust for missing "Vampire" option
		if (choice >= 3) {
			choice++;
		}
	}

	switch (choice) {
		case 0:
			if (INSTANCE->is_milestone_complete(MS_BATTLE_TUTORIAL_ATTACK_SELECT_TARGET) == false) {
				INSTANCE->set_milestone_complete(MS_BATTLE_TUTORIAL_ATTACK_SELECT_TARGET, true);
				gfx::add_notification(SELECT_A_TARGET);
			}
			current_action = ATTACK;
			BATTLE->show_enemy_stats(true);
			attack_phase = ATTACK_SELECT_TARGET;
			can_select_enemies = true;
			can_select_players = false;
			target = 0;
			break;
		case 1:
			current_action = ITEM;
			item_phase = ITEM_SELECT;
			item_index = -1;
			if (INSTANCE->is_milestone_complete(MS_BATTLE_TUTORIAL_SELECT_ITEM) == false && INSTANCE->inventory.count(wedge::OBJECT_ITEM) > 0) {
				INSTANCE->set_milestone_complete(MS_BATTLE_TUTORIAL_SELECT_ITEM, true);
				gfx::add_notification(SELECT_AN_ITEM);
			}
			break;
		case 2:
			current_action = SPELL;
			spell_phase = SPELL_SELECT;
			spell_index = -1;
			if (INSTANCE->is_milestone_complete(MS_BATTLE_TUTORIAL_SELECT_SPELL) == false) {
				INSTANCE->set_milestone_complete(MS_BATTLE_TUTORIAL_SELECT_SPELL, true);
				gfx::add_notification(SELECT_A_SPELL);
			}
			break;
		case 3:
			current_action = VAMPIRE;
			vampire_phase = VAMPIRE_SELECT;
			vampire_index = -1;
			if (INSTANCE->is_milestone_complete(MS_BATTLE_TUTORIAL_SELECT_VAMPIRE) == false) {
				INSTANCE->set_milestone_complete(MS_BATTLE_TUTORIAL_SELECT_VAMPIRE, true);
				gfx::add_notification(SELECT_A_VAMPIRE);
			}
			break;
		case 4:
			current_action = DEFEND;
			defending = true;
			BATTLE->show_enemy_stats(true);
			gfx::add_notification(GLOBALS->game_t->translate(1340)/* Originally: Defending... */);
			BATTLE->start_delay(1000);
			break;
		case 5:
			if (util::rand(0, 1) == 0) {
				current_action = RUN;
				M3_GLOBALS->run->play(false);
				BATTLE->show_enemy_stats(false);
				BATTLE->show_player_stats(false);
				std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();
				for (size_t i = 0; i < players.size(); i++) {
					Monster_RPG_3_Battle_Player *p = static_cast<Monster_RPG_3_Battle_Player *>(players[i]);
					if (p->get_stats()->hp > 0) {
						p->set_running(true);
					}
				}
			}
			else {
				gfx::add_notification(GLOBALS->game_t->translate(1341)/* Originally: Couldn't run... */);
				BATTLE->show_enemy_stats(true);
				BATTLE->start_delay(1000);
				turn_done = true;
			}
			break;
	};
}

void Monster_RPG_3_Battle_Player::set_item_index(int choice, bool cancelled)
{
	if (cancelled) {
		set_item_gui_null = true;
	}
	else {
		item_gui = NULL;
	}

	cancel_tutorial_notification();

	if (choice >= wedge::Inventory::MAX_OBJECTS) {
		item_selector_up = false;
		start_selecting_action();
	}
	else {
		item_index = inventory_indices[choice];
	}
}

void Monster_RPG_3_Battle_Player::set_spell_index(int choice, bool cancelled)
{
	if (cancelled) {
		set_spell_gui_null = true;
	}
	else {
		spell_gui = NULL;
	}

	cancel_tutorial_notification();

	if (choice >= (int)player_stats->base.num_spells()) {
		spell_selector_up = false;
		start_selecting_action();
	}
	else {
		spell_index = choice;
	}
}

void Monster_RPG_3_Battle_Player::set_vampire_index(int choice, bool cancelled)
{
	if (cancelled) {
		set_vampire_gui_null = true;
	}
	else {
		vampire_gui = NULL;
	}

	cancel_tutorial_notification();

	if (choice >= (int)M3_INSTANCE->num_vampires()) {
		vampire_selector_up = false;
		start_selecting_action();
	}
	else {
		vampire_index = choice;
	}
}

void Monster_RPG_3_Battle_Player::handle_action()
{
	switch (current_action) {
		case ATTACK:
			handle_attack();
			break;
		case ITEM:
			handle_item();
			break;
		case SPELL:
			handle_spell();
			break;
		case VAMPIRE:
			handle_vampire();
			break;
		case RUN:
			// handled in run()
			break;
		case PLAYER_SELECT:
			break;
		default: // DEFEND
			turn_done = true;
			break;
	}
}

void Monster_RPG_3_Battle_Player::handle_attack()
{
	switch (attack_phase) {
		case ATTACK_SELECT_TARGET:
			break;
		case ATTACK_WALK_FORWARD:
			walk_offset.x -= walk_speed;
			if (walk_offset.x <= -1.0f) {
				walk_offset.x = -1.0f;
				attack_phase = ATTACK_ANIMATION;
				if (player_stats->weapon.id == wedge::WEAPON_NONE) {
					sprite->set_animation("punch");
				}
				else {
					if (is_knife(player_stats->weapon.id)) {
						sprite->set_animation("stab");
					}
					else if (is_sickle(player_stats->weapon.id)) {
						sprite->set_animation("reap");
					}
					else {
						sprite->set_animation("attack");
					}
				}
				M3_GLOBALS->melee->play(false);
			}
			break;
		case ATTACK_ANIMATION:
			if (sprite->is_finished()) {
				std::vector<wedge::Battle_Entity *> entities = get_entities_that_can_be_selected();
				BATTLE->hit(this, entities[target]);
				sprite->set_animation("walk_e");
				attack_phase = ATTACK_WALK_BACK;
			}
			break;
		case ATTACK_WALK_BACK:
			walk_offset.x += walk_speed;
			if (walk_offset.x >= 0.0f) {
				walk_offset.x = 0.0f;
				sprite->set_animation("stand_w");
				turn_done = true;
			}
			break;
	}
}

void Monster_RPG_3_Battle_Player::handle_item()
{
	switch (item_phase) {
		case ITEM_SELECT:
			if (item_selector_up == false) {
				start_selecting_item();
			}
			else if (item_index >= 0) {
				if (INSTANCE->is_milestone_complete(MS_BATTLE_TUTORIAL_ITEM_SELECT_TARGET) == false)  {
					INSTANCE->set_milestone_complete(MS_BATTLE_TUTORIAL_ITEM_SELECT_TARGET, true);
					gfx::add_notification(SELECT_A_TARGET);
				}
				item_phase = ITEM_SELECT_TARGET;
				item_selector_up = false;
				BATTLE->show_enemy_stats(true);
				BATTLE->show_player_stats(true);
				can_select_enemies = true;
				can_select_players = true;
				target = 0;
			}
			else {
				BATTLE->show_enemy_stats(true);
				BATTLE->show_player_stats(true);
			}
			break;
		case ITEM_SELECT_TARGET:
			break;
		case ITEM_WALK_FORWARD:
			walk_offset.x -= walk_speed;
			if (walk_offset.x <= -1.0f) {
				walk_offset.x = -1.0f;
				item_phase = ITEM_ANIMATION;
				sprite->set_animation("use");
			}
			break;
		case ITEM_ANIMATION:
			if (sprite->is_finished()) {
				int id = INSTANCE->inventory.get_all()[item_index].id;
				std::vector<wedge::Battle_Entity *> entities = get_entities_that_can_be_selected();
				int amount = INSTANCE->inventory.use(item_index, entities[target]->get_stats());
				SDL_Colour colour, shadow_colour;
				std::string text;
				get_use_item_info(amount, id, colour, shadow_colour, text);
				if (colour.a != 0 || shadow_colour.a != 0) {
					util::Point<int> number_pos = entities[target]->get_decoration_offset(shim::font->get_text_width(text), util::Point<int>(shim::tile_size*3/4, 0), NULL);
					NEW_SYSTEM_AND_TASK(BATTLE)
					wedge::Special_Number_Step *step = new wedge::Special_Number_Step(colour, shadow_colour, text, number_pos, wedge::Special_Number_Step::RISE, new_task);
					ADD_STEP(step)
					wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, BATTLE, new_task);
					step->add_monitor(g);
					ADD_STEP(g)
					ADD_TASK(new_task)
					FINISH_SYSTEM(BATTLE)
					BATTLE->inc_waiting_for_hit(1);
				}
				if (entities[target] != this && entities[target]->get_type() == wedge::Battle_Entity::PLAYER) {
					if (entities[target]->get_stats()->hp > 0) {
						entities[target]->get_sprite()->set_animation("stand_w");
					}
				}
				sprite->set_animation("walk_e");
				item_phase = ITEM_WALK_BACK;
			}
			break;
		case ITEM_WALK_BACK:
			walk_offset.x += walk_speed;
			if (walk_offset.x >= 0.0f) {
				walk_offset.x = 0.0f;
				sprite->set_animation("stand_w");
				turn_done = true;
			}
			break;
	}
}

void Monster_RPG_3_Battle_Player::handle_spell()
{
	switch (spell_phase) {
		case SPELL_SELECT:
			if (spell_selector_up == false) {
				start_selecting_spell();
			}
			else if (spell_index >= 0) {
				if (INSTANCE->is_milestone_complete(MS_BATTLE_TUTORIAL_SPELL_SELECT_TARGET) == false)  {
					INSTANCE->set_milestone_complete(MS_BATTLE_TUTORIAL_SPELL_SELECT_TARGET, true);
					gfx::add_notification(SELECT_A_TARGET);
				}
				spell_phase = SPELL_SELECT_TARGET;
				spell_selector_up = false;
				BATTLE->show_enemy_stats(true);
				BATTLE->show_player_stats(true);
				if (SPELLS->is_white_magic(player_stats->base.spell(spell_index))) {
					can_select_enemies = true;
					can_select_players = true;
				}
				else {
					can_select_enemies = true;
					can_select_players = false;
				}
				spell_targets.clear();
				spell_targets.push_back(get_entities_that_can_be_selected()[0]);
				target = 0;
				util::Size<int> PAD(shim::screen_size.w*0.03f, shim::screen_size.h*0.03f);
				int HEIGHT = shim::font->get_height() * 2 + 4 + Dialogue_Step::BORDER * 2;
				int y = shim::screen_size.h - HEIGHT - PAD.h - 1;
				util::Point<int> text_pos(PAD.w + Dialogue_Step::BORDER, y+Dialogue_Step::BORDER);
				int WIN_H = shim::font->get_height() + 6 + Dialogue_Step::BORDER * 2 - 2;
				Button_GUI *gui = new Button_GUI(GLOBALS->game_t->translate(1342)/* Originally: Go! */, 0, 1, 0, 0, 0, HEIGHT/2 + PAD.h - WIN_H/2, 0, 0, spell_mouse_callback, this);
				shim::guis.push_back(gui);
			}
			else {
				BATTLE->show_enemy_stats(true);
				BATTLE->show_player_stats(true);
			}
			break;
		case SPELL_SELECT_TARGET:
			break;
		case SPELL_WALK_FORWARD:
			walk_offset.x -= walk_speed;
			if (walk_offset.x <= -1.0f) {
				walk_offset.x = -1.0f;
				spell_phase = SPELL_ANIMATION;
				sprite->set_animation("cast");
			}
			break;
		case SPELL_ANIMATION:
			if (sprite->is_finished()) {
				sprite->set_animation("stand_w");

				std::string spell = player_stats->base.spell(spell_index);

				SPELLS->play_sound(spell);
				BATTLE->set_indicator(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(spell)), false);
				SPELLS->start_effect(spell, spell_targets, spell_effect_callback, this);

				spell_phase = SPELL_EFFECT;
			}
			break;
		case SPELL_EFFECT:
			if (spell_effect_done >= (int)spell_targets.size()) {
				std::string spell = player_stats->base.spell(spell_index);
				spell_effect_done = 0;
				std::vector<wedge::Base_Stats *> target_stats;
				bool hit_something = false;
				for (size_t i = 0; i < spell_targets.size(); i++) {
					if (SPELLS->is_white_magic(spell) == false) {
						bool miss, lucky_hit;
						M3_BATTLE->get_lucky_misses(this, spell_targets[i], &miss, &lucky_hit);
						if (miss) {
							target_stats.push_back(NULL);
							continue;
						}
					}
					wedge::Base_Stats *s = spell_targets[i]->get_stats();
					target_stats.push_back(s);
					if (s != NULL) {
						hit_something = true;
					}
				}
				if (SPELLS->is_white_magic(spell) == false && hit_something) {
					GLOBALS->hit->play(false);
				}
				std::vector<int> v = SPELLS->use(spell, target_stats);
				BATTLE->inc_waiting_for_hit((int)v.size());
				for (size_t i = 0; i < spell_targets.size(); i++) {
					wedge::Battle_Entity *target = spell_targets[i];
					SDL_Colour colour, shadow_colour;
					bool white = SPELLS->is_white_magic(spell);
					if (target_stats[i] == NULL) {
						colour = shim::palette[13];
						shadow_colour = shim::palette[27];
					}
					else if (white || v[i] < 0) {
						colour = shim::palette[13];
						shadow_colour = shim::palette[27];
						v[i] = abs(v[i]); // abs strengths that get turned into +hp on attacks
					}
					else {
						colour = shim::palette[20];
						shadow_colour = shim::palette[27];
					}
					if (white == false || v[i] != 0) {
						std::string text = target_stats[i] == NULL ? GLOBALS->game_t->translate(1329)/* Originally: MISS! */ : util::itos(v[i]);
						util::Point<int> number_pos = target->get_decoration_offset(shim::font->get_text_width(text), util::Point<int>(shim::tile_size*3/4, 0), NULL);
						NEW_SYSTEM_AND_TASK(BATTLE)
						wedge::Special_Number_Step *step = new wedge::Special_Number_Step(colour, shadow_colour, text, number_pos, white ? wedge::Special_Number_Step::RISE : wedge::Special_Number_Step::SHAKE, new_task);
						ADD_STEP(step)
						wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, BATTLE, new_task);
						step->add_monitor(g);
						ADD_STEP(g)
						ADD_TASK(new_task)
						FINISH_SYSTEM(BATTLE)
					}
					else {
						wedge::battle_hit_callback(BATTLE);
					}
					if (target != this && target->get_type() == wedge::Battle_Entity::PLAYER) {
						if (target->get_stats()->hp > 0) {
							target->get_sprite()->set_animation("stand_w");
						}
					}
					else if (target->get_type() == wedge::Battle_Entity::ENEMY && target->get_stats()->hp <= 0) {
						wedge::Battle_Enemy *enemy = static_cast<wedge::Battle_Enemy *>(target);
						enemy->play_die_sound();
						enemy->set_dead();
						gfx::Sprite *sprite = target->get_sprite();
						sprite->set_animation("die");
					}
				}
				for (size_t i = 0; i < spell_targets.size(); i++) {
					if (SPELLS->is_white_magic(spell) == false) {
						if (target_stats[i] != NULL) {
							BATTLE->start_hit_effect(spell_targets[i]);
						}
					}
					wedge::Base_Stats *s = spell_targets[i]->get_stats();
					target_stats.push_back(s);
				}
				sprite->set_animation("walk_e");
				spell_phase = SPELL_WALK_BACK;
			}
			break;
		case SPELL_WALK_BACK:
			walk_offset.x += walk_speed;
			if (walk_offset.x >= 0.0f) {
				walk_offset.x = 0.0f;
				sprite->set_animation("stand_w");
				turn_done = true;
			}
			break;
	}
}

void Monster_RPG_3_Battle_Player::handle_vampire()
{
	switch (vampire_phase) {
		case VAMPIRE_SELECT:
			if (vampire_selector_up == false) {
				start_selecting_vampire();
			}
			else if (vampire_index >= 0) {
				if (M3_INSTANCE->vampire(vampire_index) == "vFire" && INSTANCE->is_milestone_complete(MS_CAN_USE_VFIRE) == false) {
					vampire_selector_up = false;
					current_action = NONE;
					BATTLE->show_player_stats(false);
					BATTLE->show_enemy_stats(false);

					std::string name_to_use = name == "Eny" ? GLOBALS->game_t->translate(1)/* Originally: Tiggy */ : GLOBALS->game_t->translate(0)/* Originally: Eny */;

					NEW_SYSTEM_AND_TASK(BATTLE)
					wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(vfire_callback, this, new_task);
					ADD_STEP(g)
					ADD_TASK(new_task)
					FINISH_SYSTEM(BATTLE)

					GLOBALS->do_dialogue(name_to_use + TAG_END, GLOBALS->game_t->translate(1708)/* Originally: Are you crazy?! It'll kill us both! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, g);

					BATTLE->inc_waiting_for_hit(1);
				}
				else {
					if (M3_INSTANCE->vampire(vampire_index) == "vFire") {
						INSTANCE->set_milestone_complete(MS_USED_VFIRE, true);
					}
					if (INSTANCE->is_milestone_complete(MS_BATTLE_TUTORIAL_VAMPIRE_SELECT_TARGET) == false)  {
						INSTANCE->set_milestone_complete(MS_BATTLE_TUTORIAL_VAMPIRE_SELECT_TARGET, true);
						gfx::add_notification(SELECT_A_TARGET);
					}
					vampire_phase = VAMPIRE_SELECT_TARGET;
					vampire_selector_up = false;
					BATTLE->show_enemy_stats(true);
					BATTLE->show_player_stats(true);
					if (SPELLS->is_white_magic(M3_INSTANCE->vampire(vampire_index))) {
						can_select_enemies = true;
						can_select_players = true;
					}
					else {
						can_select_enemies = true;
						can_select_players = false;
					}
					vampire_targets.clear();
					vampire_targets.push_back(get_entities_that_can_be_selected()[0]);
					target = 0;
					util::Size<int> PAD(shim::screen_size.w*0.03f, shim::screen_size.h*0.03f);
					int HEIGHT = shim::font->get_height() * 2 + 4 + Dialogue_Step::BORDER * 2;
					int y = shim::screen_size.h - HEIGHT - PAD.h - 1;
					util::Point<int> text_pos(PAD.w + Dialogue_Step::BORDER, y+Dialogue_Step::BORDER);
					int WIN_H = shim::font->get_height() + 6 + Dialogue_Step::BORDER * 2 - 2;
					Button_GUI *gui = new Button_GUI(GLOBALS->game_t->translate(1342)/* Originally: Go! */, 0, 1, 0, 0, 0, HEIGHT/2 + PAD.h - WIN_H/2, 0, 0, vampire_mouse_callback, this);
					shim::guis.push_back(gui);
				}
			}
			else {
				BATTLE->show_enemy_stats(true);
				BATTLE->show_player_stats(true);
			}
			break;
		case VAMPIRE_SELECT_TARGET:
			break;
		case VAMPIRE_WALK_FORWARD:
			walk_offset.x -= walk_speed;
			if (walk_offset.x <= -1.0f) {
				walk_offset.x = -1.0f;
				vampire_phase = VAMPIRE_ANIMATION;
				sprite->set_animation("cast");
			}
			break;
		case VAMPIRE_ANIMATION:
			if (sprite->is_finished()) {
				sprite->set_animation("stand_w");

				std::string vampire = M3_INSTANCE->vampire(vampire_index);

				SPELLS->play_sound(vampire);
				BATTLE->set_indicator(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(vampire)), false);
				SPELLS->start_effect(vampire, vampire_targets, vampire_effect_callback, this);

				sprite->set_animation("walk_e");
				vampire_phase = VAMPIRE_WALK_BACK;
			}
			break;
		case VAMPIRE_WALK_BACK:
			walk_offset.x += walk_speed;
			if (walk_offset.x >= 0.0f) {
				walk_offset.x = 0.0f;
				sprite->set_animation("stand_w");
				vampire_phase = VAMPIRE_EFFECT;
			}
			break;
		case VAMPIRE_EFFECT:
			if (vampire_effect_done >= (int)vampire_targets.size()) {
				std::string vampire = M3_INSTANCE->vampire(vampire_index);
				vampire_effect_done = 0;
				std::vector<wedge::Base_Stats *> target_stats;
				bool hit_something = false;
				for (size_t i = 0; i < vampire_targets.size(); i++) {
					bool miss, lucky_hit;
					M3_BATTLE->get_lucky_misses(this, vampire_targets[i], &miss, &lucky_hit);
					if (miss) {
						target_stats.push_back(NULL);
						continue;
					}
					wedge::Base_Stats *s = vampire_targets[i]->get_stats();
					target_stats.push_back(s);
					hit_something = true;
				}
				if (hit_something) {
					GLOBALS->hit->play(false);
				}
				std::vector<int> v = SPELLS->use(vampire, target_stats);
				BATTLE->inc_waiting_for_hit((int)v.size());
				for (size_t i = 0; i < vampire_targets.size(); i++) {
					wedge::Battle_Entity *target = vampire_targets[i];
					SDL_Colour colour, shadow_colour;
					bool white = SPELLS->is_white_magic(vampire);
					if (target_stats[i] == NULL) {
						colour = shim::palette[13];
						shadow_colour = shim::palette[27];
					}
					else if (white || v[i] < 0) {
						colour = shim::palette[13];
						shadow_colour = shim::palette[27];
						v[i] = abs(v[i]); // abs strengths that get turned into +hp on attacks
					}
					else {
						colour = shim::palette[20];
						shadow_colour = shim::palette[27];
					}
					if (white == false || v[i] != 0) {
						std::string text = target_stats[i] == NULL ? GLOBALS->game_t->translate(1329)/* Originally: MISS! */ : util::itos(v[i]);
						util::Point<int> number_pos = target->get_decoration_offset(shim::font->get_text_width(text), util::Point<int>(shim::tile_size*3/4, 0), NULL);
						NEW_SYSTEM_AND_TASK(BATTLE)
						wedge::Special_Number_Step *step = new wedge::Special_Number_Step(colour, shadow_colour, text, number_pos, white ? wedge::Special_Number_Step::RISE : wedge::Special_Number_Step::SHAKE, new_task);
						ADD_STEP(step)
						wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(wedge::battle_hit_callback, BATTLE, new_task);
						step->add_monitor(g);
						ADD_STEP(g)
						ADD_TASK(new_task)
						FINISH_SYSTEM(BATTLE)
					}
					else {
						wedge::battle_hit_callback(BATTLE);
					}
					if (target != this && target->get_type() == wedge::Battle_Entity::PLAYER) {
						if (target->get_stats()->hp > 0) {
							target->get_sprite()->set_animation("stand_w");
						}
					}
					else if (target->get_type() == wedge::Battle_Entity::ENEMY && target->get_stats()->hp <= 0) {
						wedge::Battle_Enemy *enemy = static_cast<wedge::Battle_Enemy *>(target);
						enemy->play_die_sound();
						enemy->set_dead();
						gfx::Sprite *sprite = target->get_sprite();
						sprite->set_animation("die");
					}
				}
				for (size_t i = 0; i < vampire_targets.size(); i++) {
					if (target_stats[i] != NULL) {
						BATTLE->start_hit_effect(vampire_targets[i]);
					}
				}

				std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();

				wedge::Battle_Player *other_player;

				if (players[0] == static_cast<wedge::Battle_Entity *>(this)) {
					other_player = static_cast<wedge::Battle_Player *>(players[1]);
				}
				else {
					other_player = static_cast<wedge::Battle_Player *>(players[0]);
				}

				wedge::Base_Stats *stats = other_player->get_stats();

				int hp, mp;
				get_vampire_cost(vampire, hp, mp);

				if (vampire == "vFire") {
					take_damage(hp, 0);
				}
				other_player->take_damage(hp, 0);
				stats->mp -= mp;
				stats->mp = MAX(0, stats->mp);
				
				BATTLE->start_hit_effect(other_player);

				turn_done = true;
			}
			break;
	}
}

void Monster_RPG_3_Battle_Player::handle_run()
{
	walk_offset.x += walk_speed;
	std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();
	wedge::Battle_Entity *first = players[0];
	for (size_t i = 0; i < players.size(); i++) {
		if (players[i]->get_stats()->hp > 0) {
			first = players[i];
			break;
		}
	}
	if (this == first && walk_offset.x >= 1.0f && showed_run_message == false) {
		showed_run_message = true;
		int dropped = INSTANCE->get_gold() * 0.05f;
		if (dropped > 0) {
			wedge::globals->do_dialogue("", util::string_printf(GLOBALS->game_t->translate(1346)/* Originally: Dropped %d gold! */.c_str(), dropped), wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_BOTTOM, ran_away_step);
			INSTANCE->add_gold(-dropped);
		}
		else {
			BATTLE->set_done(true);
		}
	}
}

void Monster_RPG_3_Battle_Player::start_selecting_action()
{
	if (INSTANCE->is_milestone_complete(MS_BATTLE_TUTORIAL_SELECT_ACTION) == false) {
		INSTANCE->set_milestone_complete(MS_BATTLE_TUTORIAL_SELECT_ACTION, true);
		gfx::add_notification(SELECT_AN_ACTION);
	}

	defending = false;

	if (enemies_all_dead()) {
		return;
	}

	current_action = NONE;
	std::vector<std::string> choices;
	choices.push_back(GLOBALS->game_t->translate(1347)/* Originally: Attack */);
	choices.push_back(GLOBALS->game_t->translate(1348)/* Originally: Item */);
	if (player_stats->base.num_spells() > 0) {
		choices.push_back(GLOBALS->game_t->translate(1349)/* Originally: Spell */);
	}
	if (M3_INSTANCE->num_vampires() > 0) {
		choices.push_back(GLOBALS->game_t->translate(1350)/* Originally: Vampire */);
	}
	choices.push_back(GLOBALS->game_t->translate(1351)/* Originally: Defend */);
	if (BATTLE->is_boss_battle() == false && M3_INSTANCE->boatin == false) {
		choices.push_back(GLOBALS->game_t->translate(1352)/* Originally: Run */);
	}
	action_gui = new Battle_Multiple_Choice_GUI("", choices, -1, action_callback, this);
	action_gui->resize(shim::screen_size);
	action_gui->gui->layout();
	shim::guis.push_back(action_gui);
	BATTLE->show_player_stats(true);
	BATTLE->show_enemy_stats(false);
	turn_started = true;
	last_action_gui_selected = 0;
}

void Monster_RPG_3_Battle_Player::start_selecting_item()
{
	inventory_indices.clear();

	std::vector< std::pair<int, std::string> > v;
	std::vector<std::string> descriptions;

	wedge::Object *objects = INSTANCE->inventory.get_all();

	for (int i = 0; i < wedge::Inventory::MAX_OBJECTS; i++) {
		std::pair<int, std::string> p;
		if (objects[i].type == wedge::OBJECT_ITEM && is_scroll(objects[i].id) == false) {
			p.first = objects[i].quantity;
			//p.second = objects[i].name;
			p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(objects[i].name));
			v.push_back(p);
			inventory_indices.push_back(i);
			descriptions.push_back(objects[i].description);
		}
	}

	if (v.size() == 0) {
		gfx::add_notification(GLOBALS->game_t->translate(1353)/* Originally: Inventory empty... */);
		start_selecting_action();
		return;
	}

	item_index = -1;

	item_gui = new Battle_List_GUI(v, wedge::Inventory::MAX_OBJECTS, item_callback, this);
	item_gui->set_descriptions(descriptions);
	item_gui->resize(shim::screen_size);
	item_gui->gui->layout();
	shim::guis.push_back(item_gui);
	BATTLE->show_enemy_stats(false);
	BATTLE->show_player_stats(false);

	item_selector_up = true;
}

void Monster_RPG_3_Battle_Player::start_selecting_spell()
{
	std::vector< std::pair<int, std::string> > v;

	std::vector<bool> disabled;

	for (int i = 0; i < player_stats->base.num_spells(); i++) {
		std::pair<int, std::string> p;
		std::string spell = player_stats->base.spell(i);
		int cost = SPELLS->get_cost(spell);
		p.first = cost;
		p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(spell));
		if (cost > stats->mp) {
			disabled.push_back(true);
		}
		else {
			disabled.push_back(false);
		}
		v.push_back(p);
	}

	spell_index = -1;

	spell_gui = new Battle_List_GUI(v, player_stats->base.num_spells(), spell_callback, this);
	spell_gui->resize(shim::screen_size);
	spell_gui->gui->layout();
	shim::guis.push_back(spell_gui);
	BATTLE->show_enemy_stats(false);
	BATTLE->show_player_stats(false);

	for (size_t i = 0; i < disabled.size(); i++) {
		spell_gui->set_disabled((int)i, disabled[i]);
	}

	spell_selector_up = true;
}

void Monster_RPG_3_Battle_Player::start_selecting_vampire()
{
	std::vector< std::pair<std::string, std::string> > v;

	for (int i = 0; i < M3_INSTANCE->num_vampires(); i++) {
		std::pair<std::string, std::string> p;
		std::string vampire = M3_INSTANCE->vampire(i);
		int hp, mp;
		get_vampire_cost(vampire, hp, mp);
		p.first = util::itos(hp) + "/" + util::itos(mp);
		p.second = vampire;
		v.push_back(p);
	}

	vampire_index = -1;

	vampire_gui = new Battle_Vampire_List_GUI(v, M3_INSTANCE->num_vampires(), vampire_callback, this);
	vampire_gui->resize(shim::screen_size);
	vampire_gui->gui->layout();
	shim::guis.push_back(vampire_gui);
	BATTLE->show_enemy_stats(false);
	BATTLE->show_player_stats(false);

	std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();
	bool all_alive = true;
	for (size_t i = 0; i < players.size(); i++) {
		if (players[i]->get_stats()->hp <= 0) {
			all_alive = false;
			break;
		}
	}
	if (all_alive == false) {
		for (size_t i = 0; i < v.size(); i++) {
			vampire_gui->set_disabled((int)i, true);
		}
	}
	for (size_t i = 0; i < v.size(); i++) {
		int hp, mp;
		get_vampire_cost(v[i].second, hp, mp);
		wedge::Battle_Entity *other;
		if (this == players[0]) {
			other = players[1];
		}
		else {
			other = players[0];
		}
		if (mp > other->get_stats()->mp) {
			vampire_gui->set_disabled((int)i, true);
		}
	}

	if (INSTANCE->is_milestone_complete(MS_CAN_USE_VFIRE)) {
		vampire_gui->set_disabled(0, false);
	}

	vampire_selector_up = true;
}

std::vector<wedge::Battle_Entity *> Monster_RPG_3_Battle_Player::get_entities_that_can_be_selected()
{
	std::vector<wedge::Battle_Entity *> entities;
	if (can_select_players) {
		std::vector<wedge::Battle_Entity *> v = BATTLE->get_players();
		entities.insert(entities.end(), v.begin(), v.end());
	}
	if (can_select_enemies) {
		std::vector<wedge::Battle_Entity *> v = BATTLE->get_enemies();
		std::sort(v.begin(), v.end(), enemy_x_compare);
		entities.insert(entities.end(), v.begin(), v.end());
	}
	return entities;
}

void Monster_RPG_3_Battle_Player::run()
{
	if (running) {
		handle_run();
	}
}

void Monster_RPG_3_Battle_Player::set_running(bool running)
{
	this->running = running;
	if (running) {
		sprite->set_animation("walk_e");
	}
}

void Monster_RPG_3_Battle_Player::set_spell_effect_done()
{
	spell_effect_done++;
}

void Monster_RPG_3_Battle_Player::set_vampire_effect_done()
{
	vampire_effect_done++;
}

void Monster_RPG_3_Battle_Player::cast_spell_now()
{
	if (spell_targets.size() == 0) {
		start_selecting_action();
	}
	else {
		std::string spell = player_stats->base.spell(spell_index);
		int cost = SPELLS->get_cost(spell);
		stats->mp -= cost;
		spell_phase = SPELL_WALK_FORWARD;
		walk_offset = util::Point<float>(0.0f, 0.0f);
		sprite->set_animation("walk_w");
	}
}

void Monster_RPG_3_Battle_Player::cast_vampire_now()
{
	if (vampire_targets.size() == 0) {
		start_selecting_action();
	}
	else {
		std::string vampire = M3_INSTANCE->vampire(vampire_index);
		vampire_phase = VAMPIRE_WALK_FORWARD;
		walk_offset = util::Point<float>(0.0f, 0.0f);
		sprite->set_animation("walk_w");
	}
}

void Monster_RPG_3_Battle_Player::set_ignore_next_escape(bool ignore_next_escape)
{
	this->ignore_next_escape = ignore_next_escape;
}

void Monster_RPG_3_Battle_Player::cancel_tutorial_notification()
{
	if (is_tutorial_notification(gfx::get_current_notification())) {
		gfx::cancel_current_notification();
	}
}

void Monster_RPG_3_Battle_Player::show_player_stats(int player_index)
{
	if (shim::guis.size() == 0 || dynamic_cast<Player_Stats_GUI *>(shim::guis.back()) == NULL) {
		M3_GLOBALS->button->play(false);
		player_stats_gui = new Player_Stats_GUI(player_index, player_stats_callback, this);
		shim::guis.push_back(player_stats_gui);
	}
}

void Monster_RPG_3_Battle_Player::end_player_stats_gui(bool cancelled)
{
	player_stats_gui = NULL;
	if (cancelled) {
		set_ignore_next_escape(true);
	}
}

util::Point<float> Monster_RPG_3_Battle_Player::get_draw_pos()
{
	int off_y = MAX(0, (shim::screen_size.h - SCR_H) / 2);
	return wedge::Battle_Player::get_draw_pos() + util::Point<int>(0, off_y) + walk_offset * shim::tile_size;
}

bool Monster_RPG_3_Battle_Player::enemies_all_dead()
{
	std::vector<wedge::Battle_Entity *> enemies = BATTLE->get_enemies();
	bool all_dead = true;

	for (size_t i = 0; i < enemies.size(); i++) {
		if (enemies[i]->get_stats()->hp > 0) {
			all_dead = false;
			break;
		}
	}

	return all_dead;
}

void Monster_RPG_3_Battle_Player::take_damage(int hp, int type, int y_offset)
{
	if (hp != 0) {
		wedge::Battle_Entity::take_damage(hp, type, y_offset);
	}
	if (stats->hp <= 0 && INSTANCE->is_milestone_complete(MS_USED_VFIRE) == true) {
		sprite->set_animation("dead");
	}
	if (stats->hp <= 0 && INSTANCE->is_milestone_complete(MS_USED_VFIRE) == false) {
		bool all_dead = true;
		for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
			if (INSTANCE->stats[i].base.hp > 0) {
				all_dead = false;
				break;
			}
		}
		bool used_second_chance = false;
		if (all_dead) {
			wedge::Object o = OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_SECOND_CHANCE, 1);
			int index = INSTANCE->inventory.find(o);
			if (index < 0) {
				used_second_chance = false;
			}
			else {
				used_second_chance = true;
				INSTANCE->inventory.remove(index, 1);
				for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
					INSTANCE->stats[i].base.hp = INSTANCE->stats[i].base.fixed.max_hp;
					INSTANCE->stats[i].base.mp = INSTANCE->stats[i].base.fixed.max_mp;
				}
				std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();
				for (size_t i = 0; i < players.size(); i++) {
					players[i]->get_sprite()->set_animation("stand_w");
				}
				M3_GLOBALS->second_chance->play(false);
				gfx::add_notification(GLOBALS->game_t->translate(1354)/* Originally: Raised from the dead! */);
				util::achieve((void *)ACHIEVE_RESURRECTED);
			}
		}
		if (used_second_chance == false) {
			sprite->set_animation("dead");
			stats->status = wedge::STATUS_OK;
			if (all_dead) {
				audio::stop_music();
				GLOBALS->gameover->play(shim::music_volume, true);
				gfx::add_notification(GLOBALS->game_t->translate(1355)/* Originally: You died in battle! */);
				BATTLE->set_gameover(true);
				OMNIPRESENT->start_fade(GLOBALS->gameover_fade_colour, GLOBALS->gameover_timeout-GLOBALS->gameover_fade_time, GLOBALS->gameover_fade_time);
			}
		}
		wedge::rumble(1.0f, 1000);
	}
	else {
		wedge::rumble(1.0f, 500);
	}
}
