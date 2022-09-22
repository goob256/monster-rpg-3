#ifndef BATTLE_GAME_H
#define BATTLE_GAME_H

#include <Nooskewl_Wedge/battle_game.h>

class Monster_RPG_3_Battle_Game : public wedge::Battle_Game
{
public:
	Monster_RPG_3_Battle_Game(std::string bg, int bg_delay);
	virtual ~Monster_RPG_3_Battle_Game();

	void start_transition_in();
	void start_transition_out();
	wedge::Battle_Player *create_player(int index);
	void draw();
	void draw_fore();
	int get_damage(wedge::Battle_Entity *attacker, wedge::Battle_Entity *attacked, int y_offset = 0);
	void start_hit_effect(wedge::Battle_Entity *attacked);
	void show_retry_boss_gui();
	bool run();
	
	void draw_selection_arrow(wedge::Battle_Entity *entity);

	void get_lucky_misses(wedge::Battle_Entity *attacker, wedge::Battle_Entity *attacked, bool *miss, bool *lucky_hit);

	void set_highlighted_entities(std::vector<wedge::Battle_Entity *> entities);

	void add_dialogue(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position);

	gfx::Image *get_background();

	SDL_Colour get_tint();

protected:
	std::vector<wedge::Battle_Entity *> highlighted_entities;
	std::vector<std::string> dialogue_tags;
	std::vector<std::string> dialogue_texts;
	std::vector<wedge::Dialogue_Type> dialogue_types;
	std::vector<wedge::Dialogue_Position> dialogue_positions;
	int curr_bg_frame;
	SDL_Colour tint;
	bool started_taking_turns;
};

#endif // BATTLE_GAME_H
