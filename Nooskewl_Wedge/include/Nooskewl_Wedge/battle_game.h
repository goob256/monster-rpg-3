#ifndef NOOSKEWL_WEDGE_BATTLE_GAME_H
#define NOOSKEWL_WEDGE_BATTLE_GAME_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/inventory.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Battle_End_Step;
class Battle_Entity;
class Battle_Player;

void NOOSKEWL_WEDGE_EXPORT battle_hit_callback(void *data);

class NOOSKEWL_WEDGE_EXPORT Battle_Game : public Game
{
public:
	//-- define these in your subclass
	virtual Battle_Player *create_player(int index) = 0;
	virtual void start_transition_in(); // make sure to call this superclass to get a rumble
	virtual void start_transition_out() = 0;
	virtual void draw() = 0;
	virtual void start_hit_effect(Battle_Entity *attacked) = 0;
	virtual void show_retry_boss_gui() = 0;
	//--

	Battle_Game(std::string bg, int bg_delay);
	virtual ~Battle_Game();

	virtual bool start();
	virtual void handle_event(TGUI_Event *event);
	virtual bool run();
	virtual void draw_fore();
	virtual void lost_device();
	virtual void found_device();
	virtual void resize(util::Size<int> new_size);

	virtual int hit(Battle_Entity *attacker, Battle_Entity *attacked, float mult = 1.0f, int y_offset = 0, int exact_damage = 0);
	virtual int get_damage(Battle_Entity *attacker, Battle_Entity *attacked, int y_offset = 0);

	util::Point<int> get_offset();

	std::vector<Battle_Entity *> get_players();
	std::vector<Battle_Entity *> get_enemies();
	std::vector<Battle_Entity *> get_all_entities();

	void show_enemy_stats(bool show);
	void show_player_stats(bool show);

	bool get_enemy_stats_shown();
	bool get_player_stats_shown();

	bool is_boss_battle();

	void set_done(bool done);
	void set_gameover(bool gameover);

	void add_entity(Battle_Entity *entity);

	void add_gold(int gold);
	void add_experience(int experience);

	void set_indicator(std::string text, bool left);

	void start_delay(int millis); // no actions until delay is up

	void inc_waiting_for_hit(int waiting_for_hit);
	void hit_done();
	void battle_end_signal();

	void set_retry_boss_choice_received(bool received);

	void set_music_backup(audio::MML *music_backup);

protected:
	virtual Object get_found_object();
	void sort_enemies();
	int level_up(int player_index);
	
	int indicator_time;

	std::vector<Battle_Entity *> entities;
	int turn;
	int started_turn;
	std::vector<gfx::Image *> backgrounds;
	int bg_delay;
	Uint32 battle_start_time;
	bool done;
	audio::MML *music;
	audio::MML *music_backup;
	bool enemy_stats_shown;
	bool player_stats_shown;
	bool gold_shown;
	bool experience_shown;
	bool object_shown;
	int battle_end_count;
	int gold;
	int experience;
	Battle_End_Step *battle_end_step;
	bool boss_battle;
	bool gameover;
	Uint32 gameover_time;
	Uint32 indicator_start;
	std::string indicator_text;
	bool indicator_left;
	std::vector<bool> levelup_shown;
	int waiting_for_hit;
	Uint32 delay;
	bool offered_retry;
	bool retry_choice_received;
};

}

#endif // NOOSKEWL_WEDGE_BATTLE_GAME_H
