#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/systems.h"
#include "Nooskewl_Wedge/tile_movement.h"

using namespace wedge;

namespace wedge {

Tile_Movement_Step::Tile_Movement_Step(Map_Entity *entity, Direction direction, Task *task) :
	Step(task),
	entity(entity),
	next_direction(DIR_NONE),
	waiting_for_next_direction(false),
	_hit_wall(false),
	movement_delay(0)
{
	go(direction);
}

Tile_Movement_Step::~Tile_Movement_Step()
{
}

void Tile_Movement_Step::go(Direction direction)
{
	moving = true;

	start_tile = entity->get_position();

	Area *area = entity->get_area();

	gfx::Tilemap *tilemap = area->get_tilemap();

	util::Point<int> inc;
	util::Point<float> new_offset;
	util::Point<float> new_increment;

	switch (direction) {
		case DIR_N:
			inc = util::Point<int>(0, -1);
			new_offset = util::Point<float>(0.0f, 1.0f);
			new_increment = util::Point<float>(0.0f, -1.0f);
			break;
		case DIR_E:
			inc = util::Point<int>(1, 0);
			new_offset = util::Point<float>(-1.0f, 0.0f);
			new_increment = util::Point<float>(1.0f, 0.0f);
			break;
		case DIR_S:
			inc = util::Point<int>(0, 1);
			new_offset = util::Point<float>(0.0f, -1.0f);
			new_increment = util::Point<float>(0.0f, 1.0f);
			break;
		case DIR_W:
			inc = util::Point<int>(-1, 0);
			new_offset = util::Point<float>(1.0f, 0.0f);
			new_increment = util::Point<float>(-1.0f, 0.0f);
			break;
		default:
			moving = false;
			break;
	}

	if (moving == false) {
		gfx::Sprite *sprite = entity->get_sprite();
		std::string anim = sprite ? sprite->get_animation() : "";
		if (sprite && (anim.substr(0, 4) == "walk" || anim.substr(0, 5) == "stand")) {
			entity->set_direction(entity->get_direction(), true, false);
		}
		entity->set_moving(false);
	}
	else {
		bool entity_is_solid = entity->is_solid();
		std::vector<Map_Entity *> players = AREA->get_players();
		std::vector<Map_Entity *>::iterator it = players.begin();
		it++;
		bool is_party_member = std::find(it, players.end(), entity) != players.end();
		bool in_bounds = tilemap->get_size().is_in_bounds(start_tile + inc);
		Map_Entity *entity_on_tile = area->entity_on_tile(start_tile + inc);
		bool both_party_members = entity_on_tile == NULL ? false : std::find(players.begin(), players.end(), entity) != players.end() && std::find(players.begin(), players.end(), entity_on_tile) != players.end();
		if ((entity_is_solid && tilemap->is_solid(-1, start_tile + inc)) || (is_party_member && in_bounds == false) || (both_party_members == false && (entity_on_tile != NULL && entity_on_tile->is_solid()))) {
			_hit_wall = true;
			moving = false;
			entity->set_direction(direction, true, false);
			entity->set_moving(false);
		}
		else {
			entity->set_position(start_tile + inc);
			entity->set_offset(new_offset);
			increment = new_increment;

			gfx::Sprite *sprite = entity->get_sprite();
			bool set_dir;
			if (sprite && sprite->get_animation() == std::string("walk_") + direction_to_string(direction)) {
				set_dir = false;
			}
			else {
				set_dir = true;
			}
			if (set_dir) {
				entity->set_direction(direction, true, true);
			}
			if (entity->is_moving() == false) {
				entity->set_moving(true);
			}

			if (INSTANCE->party_following_player) {
				std::vector<Map_Entity *> players = AREA->get_players();
				int index = -1;
				for (size_t i = 0; i < players.size()-1; i++) {
					if (players[i] == entity) {
						index = (int)i;
						break;
					}
				}
				if (index >= 0) {
					Map_Entity *follower = players[index+1];
					if (follower && follower->get_position() != start_tile) {
						Map_Entity_Input_Step *meis = follower->get_input_step();
						if (meis) {
							if (meis->is_following_path()) {
								meis->add_to_path(start_tile);
							}
							else {
								meis->set_path(start_tile);
								// If the lead player (the one 'follower' is following) is pathfinding andthus their
								// run() winds up in the done_signal (below) which keeps them moving along the path,
								// it will wind up here setting a path for their follower. Since players' are run()
								// first to last, that means the follower will then immediately run() after the player
								// giving them 1 extra run which means their movement will finish before the player,
								// resulting in them stopping which creates a jittery walk cycle. The line below
								// delays the follower by index+1 frames (for the first follower this would be 1, the
								// second 2, ...) which re-syncs movement between everyone.
								meis->get_movement_step()->delay_movement(index+1);
							}
						}
					}
				}
			}
		}
	}
}

bool Tile_Movement_Step::run()
{
	if (movement_delay > 0) {
		movement_delay--;
		return true;
	}

	if (_hit_wall) {
		send_done_signal();
		_hit_wall = false;
		return true;
	}

	if (moving == false) {
		return true;
	}

	util::Point<float> offset = entity->get_offset();

	float speed = entity->get_speed();
	util::Point<float> inc(speed * increment.x, speed * increment.y);
	offset += inc; // use inc here

	// use increment everywhere else
	if (increment.x < 0.0f) {
		if (offset.x <= 0.0f) {
			moving = false;
		}
	}
	else if (increment.x > 0.0f) {
		if (offset.x >= 0.0f) {
			moving = false;
		}
	}
	else if (increment.y < 0.0f) {
		if (offset.y <= 0.0f) {
			moving = false;
		}
	}
	else {
		if (offset.y >= 0.0f) {
			moving = false;
		}
	}

	if (moving == false) {
		offset = util::Point<float>(0.0f, 0.0f);
	}
	
	entity->set_offset(offset);

	if (moving == false) {
		waiting_for_next_direction = true;
		send_done_signal();
		waiting_for_next_direction = false;
		if (next_direction != DIR_NONE) {
			go(next_direction);
			next_direction = DIR_NONE;
		}
		else {
			Map_Entity_Input_Step *meis = entity->get_input_step();
			if (meis != NULL && meis->party_all_dead() == false && meis->is_following_path() == false) { // set_path was not called during send_done_signal above?
				entity->set_direction(entity->get_direction(), true, false);
				entity->set_moving(false);
			}

			moving = false;
		}
	}

	return true;
}

void Tile_Movement_Step::set_next_direction(Direction direction)
{
	if (moving) {
		return;
	}

	if (waiting_for_next_direction) {
		next_direction = direction;
		moving = true;
	}
	else {
		go(direction);
	}
}

bool Tile_Movement_Step::is_moving()
{
	return moving == true;
}

bool Tile_Movement_Step::hit_wall()
{
	return _hit_wall;
}

void Tile_Movement_Step::delay_movement(int frames)
{
	movement_delay += frames;
}

}
