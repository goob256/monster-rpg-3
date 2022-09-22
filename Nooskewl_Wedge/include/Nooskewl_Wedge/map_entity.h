#ifndef NOOSKEWL_WEDGE_MAP_ENTITY_H
#define NOOSKEWL_WEDGE_MAP_ENTITY_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Area;
class Map_Entity;
class Map_Entity_Input_Step;

class NOOSKEWL_WEDGE_EXPORT Map_Entity
{
public:
	enum Movement_Type {
		MOVEMENT_NONE = 0, // none or custom
		MOVEMENT_DEFAULT,
		MOVEMENT_WANDER,
		MOVEMENT_LOOK_AROUND
	};

	Map_Entity(std::string name);
	Map_Entity(util::JSON::Node *json);
	virtual ~Map_Entity();

	// call before start
	//--
	void set_wanders(bool can_wander_l, bool can_wander_r, bool can_wander_u, bool can_wander_d, util::Point<int> home_position, int max_dist_from_home);
	void set_looks_around(std::vector<Direction> dont_look);
	//--

	virtual bool start(Area *area);
	virtual void draw(util::Point<float> draw_offset);
	virtual void activate(Map_Entity *activator);
	
	std::string get_name();
	void set_name(std::string name);

	util::Point<int> get_position();
	void set_position(util::Point<int> position);

	util::Point<float> get_offset();
	void set_offset(util::Point<float> offset);

	float get_speed();
	void set_speed(float speed);

	bool is_moving();
	void set_moving(bool moving);

	gfx::Sprite *get_sprite();
	void set_sprite(gfx::Sprite *sprite);
	void set_destroy_sprite(bool destroy_sprite);

	Area *get_area();
	void set_area(Area *area);

	Map_Entity_Input_Step *get_input_step();
	void set_input_step(Map_Entity_Input_Step *input_step);

	Direction get_direction();
	void set_direction(Direction direction, bool update_sprite, bool moving);

	bool is_solid();
	void set_solid(bool solid);

	util::Size<int> get_size();
	void set_size(util::Size<int> size);

	virtual std::string save();

	int get_layer();
	void set_layer(int layer);
	void put_on_middle_layer();

	bool is_visible();
	void set_visible(bool visible);

	bool is_jumping();
	void set_jumping(bool jumping);

	void face(Map_Entity *entity, bool moving); // turn and face entity

	void set_movement_type(Movement_Type type); // can be called before start so an input step is never created

	void set_extra_offset(util::Point<int> extra_offset);

protected:
	std::string name;
	util::Point<int> pos;
	util::Point<float> offset; // -1 -> 1
	float speed;
	bool moving;
	gfx::Sprite *sprite;
	Area *area;
	Map_Entity_Input_Step *input_step;
	Direction direction;
	bool solid;
	util::Size<int> size;
	bool destroy_sprite;
	int layer;
	bool visible;
	bool jumping;
	Uint32 jump_time;
	Movement_Type movement_type;
	// wander
	bool can_wander_l;
	bool can_wander_r;
	bool can_wander_u;
	bool can_wander_d;
	util::Point<int> home_position;
	int max_dist_from_home;
	// end wander
	// look_around
	std::vector<Direction> dont_look;
	// end look_around
	util::Point<int> extra_offset;
};

}

#endif // NOOSKEWL_WEDGE_MAP_ENTITY_H
