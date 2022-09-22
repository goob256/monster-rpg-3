#ifndef NOOSKEWL_WEDGE_AREA_GAME_H
#define NOOSKEWL_WEDGE_AREA_GAME_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/inventory.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Area;
class Area_Hooks;
class Map_Entity;

class NOOSKEWL_WEDGE_EXPORT Area_Game : public Game
{
public:
	// Implement these in your subclass! Area_Game should be created when the user starts/loads a game from your title screen
	//--
	virtual Area_Hooks *get_area_hooks(std::string area_name, Area *area) = 0;
	virtual void draw() = 0;
	virtual void draw_fore();
	//--

	Area_Game();
	virtual ~Area_Game();

	virtual bool start_area(Area *area);
	virtual void handle_event(TGUI_Event *event);
	virtual bool run();
	virtual void lost_device();
	virtual void found_device();
	virtual void resize(util::Size<int> new_size);
	virtual Game *create_menu();
	virtual Game *create_shop(Object_Type type, std::vector<Object> items);
	virtual Map_Entity *create_entity(std::string type, util::JSON::Node *json);
	virtual Map_Entity *create_player(std::string entity_name);
	virtual Area *create_area(std::string name);
	virtual Area *create_area(util::JSON::Node *json);
	virtual void battle_ended(Battle_Game *battle); // not called on gameover

	void start_menu();
	void end_menu();

	void start_shop(Object_Type type, std::vector<Object> items);
	void end_shop();

	Area *get_current_area();

	Map_Entity *get_player(int index);
	std::vector<Map_Entity *> get_players();
	void set_player(int index, Map_Entity *player);
	void set_players(std::vector<Map_Entity *> players);

	void set_gameover(bool gameover);

	void set_pause_entity_movement_on_next_area_change(bool pause);
	bool get_pause_entity_movement_on_next_area_change();

	bool is_pausing();
	bool is_paused();

	void set_sfx_volume_to_zero_next_area_fade(bool set);
	void set_fade_sfx_volume(bool fade);

	bool changing_areas();

protected:
	void start_pause();
	void end_pause();

	void remove_player_from_current_area();
	
	int pause_fade_time;
	int change_area_fade_duration;

	Uint32 pause_start_time;
	Uint32 pause_end_time;
	bool pausing;
	bool paused;

	Area *current_area;

	Area *next_area;
	std::vector< util::Point<int> > next_area_positions;
	std::vector<Direction> next_area_directions;
	bool scrolling_in;
	bool fading_in;
	util::Point<float> scroll_offset;
	util::Point<float> scroll_increment;

	bool gameover;
	Uint32 gameover_time;

	Uint32 fade_start_time;
	bool half_faded;

	bool pause_is_for_menu;
	Object_Type shop_type;
	std::vector<Object> shop_items;

	std::vector<Map_Entity *> players;

	bool pause_entity_movement_on_next_area_change;

	float bak_sfx_volume;

	bool set_sfx_volume_to_zero;
	bool fade_sfx_volume;
};

}

#endif // NOOSKEWL_WEDGE_AREA_GAME_H
