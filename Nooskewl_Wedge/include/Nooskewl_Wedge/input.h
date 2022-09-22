#ifndef NOOSKEWL_WEDGE_INPUT_H
#define NOOSKEWL_WEDGE_INPUT_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class Map_Entity;
class Tile_Movement_Step;

class NOOSKEWL_WEDGE_EXPORT Map_Entity_Input_Step : public Step
{
public:
	Map_Entity_Input_Step(Map_Entity *entity, Task *task);
	virtual ~Map_Entity_Input_Step();

	void manual_handle_event(TGUI_Event *event);

	void done_signal(Step *step);

	bool is_moving();
	void end_movement();

	bool set_path(util::Point<int> goal, bool can_cancel = true, bool check_solids = true, bool allow_out_of_bounds = false);
	bool is_following_path();
	void add_to_path(util::Point<int> position);

	Tile_Movement_Step *get_movement_step();
	void set_movement_step(Tile_Movement_Step *movement_step);

	void repeat_pressed();
	void reset(System *new_system);
	void pause_presses(bool pause);
	bool are_presses_paused();

	void set_stash(bool l, bool r, bool u, bool d);

	void clear();
	
	bool party_all_dead();

	util::Point<int> get_path_goal();

protected:
	Direction path_next(util::Point<int> player_pos);
	void start_moving(Direction direction);
	void end_movement(bool at_next_tile, bool reset_directions = true, bool zero_party_offset = true);
	bool all_solid(gfx::Tilemap *tilemap, util::Point<int> entity_pos, util::Point<int> click);
	void handle_joy(TGUI_Event *event, int x, int y, int old_x, int old_y);
	Direction latest();
	void create_movement_step();
	void joy_axis_up();
	void mini_pause();

	Map_Entity *entity;
	bool l, r, u, d;
	Uint32 l_t, r_t, u_t, d_t; // times pressed
	bool stash_l, stash_r, stash_u, stash_d;
	Tile_Movement_Step *movement_step;
	std::list<util::A_Star::Node> path;
	bool following_path;
	bool stop_at_next_tile;
	float joy_axis0;
	float joy_axis1;
	int hat_x;
	int hat_y;
	bool pause_at_next_tile;
	bool can_cancel_path;
	bool presses_paused;
	bool locked;
	bool mini_pause_at_next_tile;
};

}

#endif // NOOSKEWL_WEDGE_INPUT_H
