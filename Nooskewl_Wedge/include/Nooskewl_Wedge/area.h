#ifndef NOOSKEWL_WEDGE_AREA_H
#define NOOSKEWL_WEDGE_AREA_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/globals.h"

namespace wedge {

class Area_Hooks;
class Battle_Game;
class Map_Entity;
class System;

typedef std::list<Map_Entity *> Map_Entity_List;

class NOOSKEWL_WEDGE_EXPORT Area
{
public:
	enum Layer_Spec {
		ALL = 0,
		BELOW,
		ABOVE
	};

	Area();
	Area(std::string name);
	Area(util::JSON::Node *json);
	virtual ~Area();

	virtual bool start();
	virtual void draw(Layer_Spec spec = ALL); // determinates offset by player[0]
	virtual void draw(util::Point<float> map_offset, Layer_Spec spec = ALL);
	virtual void run();
	virtual void lost_device();
	virtual void found_device();
	virtual void resize(util::Size<int> new_size);
	virtual void handle_event(TGUI_Event *event);
	
	// hook stuff
	//--
	void set_hooks(Area_Hooks *hooks); // call right after start
	Area_Hooks *get_hooks();
	void started();
	void end(); // called just before saving level, then immediately level is deleted
	bool on_tile(Map_Entity *entity); // return true to stop moving
	bool try_tile(Map_Entity *entity, util::Point<int> tile_pos); // right when about to move to tile_pos, return true to cancel/not allow it
	void activate_with(Map_Entity *entity);
	Battle_Game *get_random_battle();
	void dialogue_done(Map_Entity *entity);
	bool has_battles();
	void battle_ended(Battle_Game *battle);
	std::vector<util::A_Star::Way_Point> get_way_points(util::Point<int> from);
	bool is_corner_portal(util::Point<int> pos);
	//--

	std::string get_next_area_name(); // if this is non-"", change areas (call every frame)
	std::vector< util::Point<int> > get_next_area_positions();
	std::vector<Direction> get_next_area_directions();
	bool get_next_area_scrolls_in();

	// call these to change areas
	void set_next_area(std::string name, util::Point<int> position, Direction direction);
	void set_next_area(std::string name, std::vector< util::Point<int> > player_positions, std::vector<Direction> directions);

	bool exit_gameplay(); // if this is true, exit to main menu

	std::string get_name();
	gfx::Tilemap *get_tilemap();
	Map_Entity_List &get_entities();
	Map_Entity *find_entity(std::string name);
	void remove_entity(Map_Entity *entity, bool destroy);

	// get offset to draw area if entity is in centre
	util::Point<float> get_centred_offset(util::Point<int> entity_position, util::Point<float> entity_offset, bool clamp_edges);

	System *get_entity_movement_system();
	void disconnect_player_input_step();

	Map_Entity *entity_on_tile(util::Point<int> pos); // return entity that's on pos if there is one else NULL

	// returns a vector of Rectangles for each entity except ones in ignore
	std::vector< util::Rectangle<int> > get_entity_solids(std::vector<Map_Entity *> ignore);

	bool can_save();
	std::string save(bool save_players);

	void set_overlay_colour(SDL_Colour colour);

	void set_entities_standing();

	void add_entity(Map_Entity *entity);

	bool is_new_game();
	bool was_loaded();

	int get_middle_layer();

protected:
	void order_player_input_steps();

	std::string name;

	System *entity_movement_system;

	gfx::Tilemap *tilemap;
	Map_Entity_List entities;

	std::string next_area_name;
	std::vector< util::Point<int> > next_area_positions;
	std::vector<Direction> next_area_directions;
	bool next_area_scrolls_in;

	bool done; // exit_gameplay()

	Area_Hooks *hooks;

	util::JSON::Node *json;

	SDL_Colour overlay_colour;

	bool new_game;
	bool loaded;
	std::string hook_save;
};

class NOOSKEWL_WEDGE_EXPORT Area_Hooks
{
public:
	struct Scroll_Zone
	{
		util::Rectangle<int> zone;
		std::string area_name;
		util::Point<int> topleft_dest;
		Direction direction;
	};

	struct Fade_Zone
	{
		util::Rectangle<int> zone;
		std::string area_name;
		std::vector< util::Point<int> > player_positions;
		std::vector<Direction> directions;
	};

	Area_Hooks(Area *area);
	virtual ~Area_Hooks();

	virtual bool start(bool new_game, bool loaded, std::string save);
	virtual void started();
	virtual void end();
	virtual bool on_tile(Map_Entity *entity);
	virtual bool try_tile(Map_Entity *entity, util::Point<int> tile_pos);
	virtual bool activate(Map_Entity *activator, Map_Entity *activated);
	virtual bool activate_with(Map_Entity *activator);
	virtual void pre_draw(int layer, util::Point<float> map_offset);
	virtual void post_draw(int layer, util::Point<float> map_offset);
	virtual std::vector<int> get_pre_draw_layers();
	virtual std::vector<int> get_post_draw_layers();
	virtual void remove_entity(Map_Entity *entity);
	virtual void lost_device();
	virtual void found_device();
	virtual std::string save();
	virtual bool has_battles();
	virtual Battle_Game *get_random_battle();
	virtual bool can_save();
	virtual void dialogue_done(Map_Entity *entity);
	virtual void set_animated_tiles();
	virtual void run();
	virtual void resize(util::Size<int> new_size);
	virtual void handle_event(TGUI_Event *event);
	virtual void battle_ended(Battle_Game *battle);
	virtual std::vector<util::A_Star::Way_Point> get_way_points(util::Point<int> from);
	virtual bool is_corner_portal(util::Point<int> pos);

	void set_player_start_zones(); // called automatically to set some stuff needed

	std::vector<Scroll_Zone *> get_scroll_zones(util::Point<int> pos);
	Fade_Zone *get_fade_zone(util::Point<int> pos);

protected:
	void insert_rand_battle_type(int type);
	void gen_rand_battle_table(int num_types);
	void gen_rand_battle_type(int type);

	Area *area;
	std::vector<Scroll_Zone> scroll_zones;
	std::vector<Fade_Zone> fade_zones;
	bool ignore_next_on_tile;
	std::vector<int> rand_battle_table;
	int rand_battle_last; // never start the vector with the same type as the end of the last iteration
	util::Point<float> map_offset;
	std::vector<Scroll_Zone *> player_start_scroll_zones;
	Fade_Zone *player_start_fade_zone;
};

bool NOOSKEWL_WEDGE_EXPORT entity_y_compare(Map_Entity *a, Map_Entity *b);

}

#endif // NOOSKEWL_WEDGE_AREA_H
