#ifndef BATTLE_PLAYER_H
#define BATTLE_PLAYER_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/battle_player.h>
#include <Nooskewl_Wedge/stats.h>

#include "gui.h"

class Battle_List_GUI;
class Battle_Multiple_Choice_GUI;
class Label;
class My_Window;
class Player_Stats_GUI;
class Ran_Away_Step;
class Text_Button;

class Monster_RPG_3_Battle_Player : public wedge::Battle_Player
{
public:
	enum Action {
		UNINITIALIZED = -1,
		NONE = 0,
		ATTACK,
		ITEM,
		SPELL,
		VAMPIRE,
		DEFEND,
		RUN,
		PLAYER_SELECT
	};

	enum Attack_Phase {
		ATTACK_SELECT_TARGET = 0,
		ATTACK_WALK_FORWARD,
		ATTACK_ANIMATION,
		ATTACK_WALK_BACK
	};

	enum Item_Phase {
		ITEM_SELECT = 0,
		ITEM_SELECT_TARGET,
		ITEM_WALK_FORWARD,
		ITEM_ANIMATION,
		ITEM_WALK_BACK
	};

	enum Spell_Phase {
		SPELL_SELECT = 0,
		SPELL_SELECT_TARGET,
		SPELL_WALK_FORWARD,
		SPELL_ANIMATION,
		SPELL_EFFECT,
		SPELL_WALK_BACK
	};

	enum Vampire_Phase {
		VAMPIRE_SELECT = 0,
		VAMPIRE_SELECT_TARGET,
		VAMPIRE_WALK_FORWARD,
		VAMPIRE_ANIMATION,
		VAMPIRE_EFFECT,
		VAMPIRE_WALK_BACK
	};

	Monster_RPG_3_Battle_Player(int index);
	virtual ~Monster_RPG_3_Battle_Player();

	bool start();
	void handle_event(TGUI_Event *event);
	void draw();
	void draw_fore();
	void run();
	bool take_turn();
	void take_damage(int hp, int type, int y_offset = 0);

	util::Point<float> get_draw_pos();

	void show_player_stats(int player_index);

	// most of these are called from callbacks
	void set_action(int choice);
	void set_item_index(int choice, bool cancelled);
	void set_spell_index(int choice, bool cancelled);
	void set_vampire_index(int choice, bool cancelled);
	void ran_away();
	void set_running(bool running);
	void set_spell_effect_done();
	void set_vampire_effect_done();
	void start_selecting_action();
	void cast_spell_now();
	void cast_vampire_now();
	void set_ignore_next_escape(bool ignore_next_escape);
	void cancel_tutorial_notification();
	void end_player_stats_gui(bool cancelled);

private:
	void start_selecting_item();
	void start_selecting_spell();
	void start_selecting_vampire();
	void handle_action();
	void handle_attack();
	void handle_item();
	void handle_spell();
	void handle_vampire();
	void handle_run();
	std::vector<wedge::Battle_Entity *> get_entities_that_can_be_selected();
	bool enemies_all_dead();

	float walk_speed;

	bool turn_started;
	bool turn_done;
	int turn_choice;
	Action current_action;
	Attack_Phase attack_phase;
	util::Point<float> walk_offset;
	bool can_select_enemies;
	bool can_select_players;
	int target;
	Item_Phase item_phase;
	bool item_selector_up;
	std::vector<int> inventory_indices;
	int item_index;
	bool showed_run_message;
	Ran_Away_Step *ran_away_step;
	Battle_Multiple_Choice_GUI *action_gui;
	Battle_List_GUI *item_gui;
	bool set_item_gui_null;
	bool running;
	
	gfx::Sprite *weapon_sprite;

	Spell_Phase spell_phase;
	bool spell_selector_up;
	int spell_index;
	bool set_spell_gui_null;
	Battle_List_GUI *spell_gui;
	std::vector<Battle_Entity *> spell_targets;
	int spell_effect_done;

	Vampire_Phase vampire_phase;
	bool vampire_selector_up;
	int vampire_index;
	bool set_vampire_gui_null;
	Battle_Vampire_List_GUI *vampire_gui;
	std::vector<Battle_Entity *> vampire_targets;
	int vampire_effect_done;
	
	bool ignore_next_escape;

	int selected_player;
	Player_Stats_GUI *player_stats_gui;
	int last_action_gui_selected;
	bool talked_about_vfire;
};

#endif // BATTLE_PLAYER_H
