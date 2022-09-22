#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/look_around_input.h"
#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/stats.h"
#include "Nooskewl_Wedge/wander_input.h"

using namespace wedge;

namespace wedge {

Map_Entity::Map_Entity(std::string name) :
	name(name),
	offset(0.0f, 0.0f),
	speed(0.05f),
	moving(false),
	sprite(NULL),
	area(NULL),
	input_step(NULL),
	direction(DIR_NONE),
	solid(true),
	size(1, 1),
	destroy_sprite(true),
	layer(-1),
	visible(true),
	jumping(false),
	movement_type(MOVEMENT_DEFAULT),
	extra_offset(0, 0)
{
}

Map_Entity::Map_Entity(util::JSON::Node *json) :
	offset(0.0f, 0.0f),
	speed(0.05f),
	moving(false),
	sprite(NULL),
	area(NULL),
	input_step(NULL),
	direction(DIR_NONE),
	solid(true),
	size(1, 1),
	destroy_sprite(true),
	layer(-1),
	visible(true),
	jumping(false),
	movement_type(MOVEMENT_DEFAULT),
	extra_offset(0, 0)
{
	name = util::JSON::trim_quotes(json->key);

	for (size_t i = 0; i < json->children.size(); i++) {
		util::JSON::Node *n = json->children[i];

		if (n->key == "\"position\"") {
			pos = json_to_integer_point(n);
		}
		else if (n->key == "\"speed\"") {
			speed = atof(n->value.c_str());
		}
		else if (n->key == "\"sprite\"") {
			sprite = json_to_sprite(n);
		}
		else if (n->key == "\"direction\"") {
			direction = (Direction)json_to_integer(n);
		}
		else if (n->key == "\"solid\"") {
			solid = json_to_bool(n);
		}
		else if (n->key == "\"size\"") {
			size = json_to_integer_size(n);
		}
		else if (n->key == "\"movement_type\"") {
			if (n->value == "\"wander\"") {
				movement_type = MOVEMENT_WANDER;
			}
			else if (n->value == "\"look_around\"") {
				movement_type = MOVEMENT_LOOK_AROUND;
			}
		}
		else if (n->key == "\"can_wander_l\"") {
			can_wander_l = json_to_bool(n);
		}
		else if (n->key == "\"can_wander_r\"") {
			can_wander_r = json_to_bool(n);
		}
		else if (n->key == "\"can_wander_u\"") {
			can_wander_u = json_to_bool(n);
		}
		else if (n->key == "\"can_wander_d\"") {
			can_wander_d = json_to_bool(n);
		}
		else if (n->key == "\"home_position\"") {
			home_position = json_to_integer_point(n);
		}
		else if (n->key == "\"max_dist_from_home\"") {
			max_dist_from_home = json_to_integer(n);
		}
		else if (n->key == "\"can_look_e\"") {
			if (json_to_bool(n) == false) {
				dont_look.push_back(DIR_E);
			}
		}
		else if (n->key == "\"can_look_w\"") {
			if (json_to_bool(n) == false) {
				dont_look.push_back(DIR_W);
			}
		}
		else if (n->key == "\"can_look_s\"") {
			if (json_to_bool(n) == false) {
				dont_look.push_back(DIR_S);
			}
		}
		else if (n->key == "\"can_look_n\"") {
			if (json_to_bool(n) == false) {
				dont_look.push_back(DIR_N);
			}
		}
		else if (n->key == "\"layer\"") {
			layer = json_to_integer(n);
		}
		else if (n->key == "\"visible\"") {
			visible = json_to_bool(n);
		}
		else if (n->key == "\"extra_offset\"") {
			extra_offset = json_to_integer_point(n);
		}
	}
}

Map_Entity::~Map_Entity()
{
	if (destroy_sprite) {
		delete sprite;
	}
	if (input_step) {
		input_step->die();
	}
}

void Map_Entity::set_wanders(bool can_wander_l, bool can_wander_r, bool can_wander_u, bool can_wander_d, util::Point<int> home_position, int max_dist_from_home)
{
	movement_type = MOVEMENT_WANDER;
	this->can_wander_l = can_wander_l;
	this->can_wander_r = can_wander_r;
	this->can_wander_u = can_wander_u;
	this->can_wander_d = can_wander_d;
	this->home_position = home_position;
	this->max_dist_from_home = max_dist_from_home;
}

void Map_Entity::set_looks_around(std::vector<Direction> dont_look)
{
	movement_type = MOVEMENT_LOOK_AROUND;
	this->dont_look = dont_look;
}

bool Map_Entity::start(Area *area)
{
	this->area = area;

	if (movement_type != MOVEMENT_NONE) {
		System *new_system = area->get_entity_movement_system();
		Task *new_task = new Task(new_system);

		switch (movement_type) {
			case MOVEMENT_DEFAULT:
				input_step = new Map_Entity_Input_Step(this, new_task);
				break;
			case MOVEMENT_WANDER:
				input_step = new Wander_Input_Step(this, can_wander_l, can_wander_r, can_wander_u, can_wander_d, home_position, max_dist_from_home, new_task);
				break;
			case MOVEMENT_LOOK_AROUND:
				input_step = new Look_Around_Input_Step(this, dont_look, new_task);
				break;
			default:
				break;
		}

		ADD_STEP(input_step)
		ADD_TASK(new_task)
	}

	if (layer == -1) {
		put_on_middle_layer();
	}

	return true;
}

std::string Map_Entity::get_name()
{
	return name;
}

void Map_Entity::set_name(std::string name)
{
	this->name = name;
}

util::Point<int> Map_Entity::get_position()
{
	return pos;
}

void Map_Entity::set_position(util::Point<int> position)
{
	pos = position;
}

util::Point<float> Map_Entity::get_offset()
{
	return offset;
}

void Map_Entity::set_offset(util::Point<float> offset)
{
	this->offset = offset;
}

float Map_Entity::get_speed()
{
	if (jumping) {
		std::vector<Uint32> delays = sprite->get_frame_times();
		std::string anim = sprite->get_animation();
		if (delays.size() < 3 || anim.substr(0, 4) != "jump") {
			return speed;
		}
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - jump_time;
		Uint32 motion = delays[0] + delays[1]; // first two frames are jump motion
		if (elapsed < motion) {
			return 0.0f;
		}
		Uint32 leap = 0; // length of all frames except first 2
		for (size_t i = 2; i < delays.size(); i++) {
			leap += delays[i];
		}
		return 16.0f/*16 ms per frame at 60fps*/ / leap;
	}
	return speed;
}

void Map_Entity::set_speed(float speed)
{
	this->speed = speed;
}

bool Map_Entity::is_moving()
{
	return moving;
}

void Map_Entity::set_moving(bool moving)
{
	this->moving = moving;
}

gfx::Sprite *Map_Entity::get_sprite()
{
	return sprite;
}

void Map_Entity::set_sprite(gfx::Sprite *sprite)
{
	this->sprite = sprite;
}

void Map_Entity::set_destroy_sprite(bool destroy_sprite)
{
	this->destroy_sprite = destroy_sprite;
}

Area *Map_Entity::get_area()
{
	return area;
}

void Map_Entity::draw(util::Point<float> draw_offset)
{
	if (visible == false || sprite == NULL) {
		return;
	}

	draw_offset += extra_offset;

	util::Point<float> p = (offset + pos) * shim::tile_size;
	gfx::Image *image = sprite->get_current_image();

	p.y -= (image->size.h - shim::tile_size);

	image->draw(draw_offset+p);

	int index = -1;
	std::vector<Map_Entity *> players = AREA->get_players();
	for (size_t i = 0; i < players.size(); i++) {
		if (players[i] == this) {
			index = (int)i;
			break;
		}
	}
	if (index >= 0 && INSTANCE->stats[index].base.status != STATUS_OK) {
		if (INSTANCE->stats[index].base.status == STATUS_POISONED) {
			gfx::Image *poison_image = globals->poison_sprite->get_current_image();
			util::Point<int> topleft, bottomright;
			sprite->get_bounds(topleft, bottomright);
			util::Point<int> sz = bottomright - topleft;
			poison_image->draw(draw_offset+p+topleft+sz/2-poison_image->size/2);
		}
		else {
			GLOBALS->draw_custom_status(this, INSTANCE->stats[index].base.status, draw_offset+p);
		}
	}
}

Map_Entity_Input_Step *Map_Entity::get_input_step()
{
	return input_step;
}

void Map_Entity::set_input_step(Map_Entity_Input_Step *input_step)
{
	this->input_step = input_step;
}

Direction Map_Entity::get_direction()
{
	return direction;
}

void Map_Entity::set_direction(Direction direction, bool update_sprite, bool moving)
{
	this->direction = direction;

	if (update_sprite) {
		if (moving) {
			switch (direction) {
				case DIR_N:
					sprite->set_animation("walk_n");
					break;
				case DIR_E:
					if (jumping) {
						sprite->set_animation("jump_e");
					}
					else {
						sprite->set_animation("walk_e");
					}
					break;
				case DIR_S:
					sprite->set_animation("walk_s");
					break;
				case DIR_W:
					if (jumping) {
						sprite->set_animation("jump_w");
					}
					else {
						sprite->set_animation("walk_w");
					}
					break;
				default:
					break;
			}
		}
		else {
			switch (direction) {
				case DIR_N:
					sprite->set_animation("stand_n");
					break;
				case DIR_E:
					sprite->set_animation("stand_e");
					break;
				case DIR_S:
					sprite->set_animation("stand_s");
					break;
				case DIR_W:
					sprite->set_animation("stand_w");
					break;
				default:
					break;
			}
		}
	}
}

void Map_Entity::set_area(Area *area)
{
	this->area = area;
}

bool Map_Entity::is_solid()
{
	return solid;
}

void Map_Entity::set_solid(bool solid)
{
	this->solid = solid;
}

util::Size<int> Map_Entity::get_size()
{
	return size;
}

void Map_Entity::set_size(util::Size<int> size)
{
	this->size = size;
}

void Map_Entity::activate(Map_Entity *activator)
{
}

std::string Map_Entity::save()
{
	std::string s;
	if (movement_type == MOVEMENT_WANDER) {
		s += util::string_printf("\"movement_type\": \"wander\",");
		s += util::string_printf("\"can_wander_l\": %s,", bool_to_string(can_wander_l).c_str());
		s += util::string_printf("\"can_wander_r\": %s,", bool_to_string(can_wander_r).c_str());
		s += util::string_printf("\"can_wander_u\": %s,", bool_to_string(can_wander_u).c_str());
		s += util::string_printf("\"can_wander_d\": %s,", bool_to_string(can_wander_d).c_str());
		s += util::string_printf("\"home_position\": %s,", integer_point_to_string(home_position).c_str());
		s += util::string_printf("\"max_dist_from_home\": %d,", max_dist_from_home);
	}
	else if (movement_type == MOVEMENT_LOOK_AROUND) {
		s += util::string_printf("\"movement_type\": \"look_around\",");
		for (size_t i = 0; i < dont_look.size(); i++) {
			std::string d;
			if (dont_look[i] == DIR_E) {
				d = "e";
			}
			else if (dont_look[i] == DIR_W) {
				d = "w";
			}
			else if (dont_look[i] == DIR_N) {
				d = "n";
			}
			else if (dont_look[i] == DIR_S) {
				d = "s";
			}
			if (d != "") {
				s += "\"can_look_" + d + "\": false,";
			}
		}
	}
	s += util::string_printf("\"position\": %s,", integer_point_to_string(pos).c_str());
	s += util::string_printf("\"speed\": %f,", speed); // FIXME: will this lose precision over time?
	if (sprite != NULL) {
		s += util::string_printf("\"sprite\": %s,", sprite_to_string(sprite).c_str());
	}
	s += util::string_printf("\"direction\": %d,", (int)direction);
	s += util::string_printf("\"solid\": %s,", bool_to_string(solid).c_str());
	s += util::string_printf("\"size\": %s,", integer_size_to_string(size).c_str());
	s += util::string_printf("\"layer\": %d,", layer);
	s += util::string_printf("\"visible\": %s,", bool_to_string(visible).c_str());
	s += util::string_printf("\"extra_offset\": %s", integer_point_to_string(extra_offset).c_str());
	return s;
}

int Map_Entity::get_layer()
{
	return layer;
}

void Map_Entity::set_layer(int layer)
{
	this->layer = layer;
}

void Map_Entity::put_on_middle_layer()
{
	layer = area->get_middle_layer();
}

bool Map_Entity::is_visible()
{
	return visible;
}

void Map_Entity::set_visible(bool visible)
{
	this->visible = visible;
}

bool Map_Entity::is_jumping()
{
	return jumping;
}

void Map_Entity::set_jumping(bool jumping)
{
	this->jumping = jumping;
	if (jumping) {
		jump_time = GET_TICKS();
	}
}

void Map_Entity::face(Map_Entity *entity, bool moving)
{
	util::Point<int> diff = entity->get_position() - get_position();
	Direction dir = direction_from_offset(diff);
	set_direction(dir, true, moving);
}

void Map_Entity::set_movement_type(Movement_Type type)
{
	movement_type = type;
}

void Map_Entity::set_extra_offset(util::Point<int> extra_offset)
{
	this->extra_offset = extra_offset;
}

}
