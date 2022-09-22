#ifndef AREA_GAME_H
#define AREA_GAME_H

#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/battle_game.h>

class Monster_RPG_3_Area_Game : public wedge::Area_Game
{
public:
	Monster_RPG_3_Area_Game();
	virtual ~Monster_RPG_3_Area_Game();

	wedge::Area_Hooks *get_area_hooks(std::string area_name, wedge::Area *area);
	wedge::Game *create_menu();
	wedge::Game *create_shop(wedge::Object_Type type, std::vector<wedge::Object> items);
	wedge::Map_Entity *create_entity(std::string type, util::JSON::Node *json);
	wedge::Map_Entity *create_player(std::string entity_name);
	void draw();
	void draw_fore();
	void handle_event(TGUI_Event *event);
	wedge::Area *create_area(std::string name);
	wedge::Area *create_area(util::JSON::Node *json);
	void battle_ended(wedge::Battle_Game *battle);
	bool run();

	void set_use_camera(bool use_camera);
	void set_camera(util::Point<float> camera);

	void set_next_fadeout_colour(SDL_Colour colour);

	void draw_dust(util::Point<float> dust_offset);

	int get_num_areas_created();

private:
	bool use_camera;
	util::Point<float> camera;
	SDL_Colour fadeout_colour;

	// For Desert levels
	static const int NUM_DUST = 120;
	static const int DUST_AMPLITUDE = 10;
	static const float MIN_DUST_SPEED;
	static const float MAX_DUST_SPEED;
	struct Dust {
		util::Point<float> position;
		int cycles;
		float phase;
		float speed;
	};
	std::vector<Dust> dust;
	gfx::Image *dust_image;
	int prev_chests_opened;
	int area_create_count;
};

#endif // AREA_GAME_H
