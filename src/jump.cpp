#include "globals.h"
#include "jump.h"

Jump_Step::Jump_Step(wedge::Map_Entity *entity, util::Point<float> run_offset, float jump_height, util::Point<int> dest_tile, wedge::Task *task) :
	wedge::Step(task),
	entity(entity),
	run_offset(run_offset),
	jump_height(jump_height),
	dest_tile(dest_tile),
	jumping(false)
{
}

Jump_Step::~Jump_Step()
{
}

void Jump_Step::start()
{
	start_tile = entity->get_position();
	jump_pixels = jump_height*2 + fabs(run_offset.y) *shim::tile_size + shim::tile_size * (dest_tile.y-start_tile.y);
	jump_time = JUMP_TIME_PER_PIXEL * jump_pixels;

	gfx::Sprite *entity_sprite = entity->get_sprite();

	if (start_tile.x < dest_tile.x) {
		entity_sprite->set_animation("walk_e");
	}
	else {
		entity_sprite->set_animation("walk_w");
	}
	
	was_solid = entity->is_solid();
	entity->set_solid(false);
}

bool Jump_Step::run()
{
	bool done;

	if (jumping) {
		bool go;
		if (jump_start == 0) {
			gfx::Sprite *entity_sprite = entity->get_sprite();
			if (entity_sprite->get_current_frame() > 1) {
				go = true;
				jump_start = GET_TICKS();
				entity_sprite->stop();
			}
			else {
				go = false;
			}
		}
		else {
			go = true;
		}
		if (go) {
			Uint32 now = GET_TICKS();
			Uint32 elapsed = now - jump_start;
			float p = elapsed / (float)jump_time;
			if (p >= 1.0f) {
				done = true;
				entity->set_position(dest_tile);
				entity->set_offset(util::Point<float>(0.0f, 0.0f));
				if (start_tile.x < dest_tile.x) {
					entity->set_direction(wedge::DIR_E, true, false);
				}
				else {
					entity->set_direction(wedge::DIR_W, true, false);
				}
				entity->set_solid(was_solid);
			}
			else {
				done = false;
				float up_p = jump_height / (float)jump_pixels;
				util::Point<float> o;
				float lateral = (dest_tile.x-start_tile.x) - run_offset.x;
				float half = lateral / 2.0f;
				if (p <= up_p) {
					p /= up_p;
					o = util::Point<float>(half * p, -(jump_height / (float)shim::tile_size * p));
				}
				else {
					float drop = (jump_pixels - jump_height) / shim::tile_size;
					p = (p - up_p) / (1.0f - up_p);
					o = util::Point<float>(half + half * p, drop * p - jump_height / (float)shim::tile_size);
				}
				entity->set_offset(run_offset+o);
			}
		}
		else {
			done = false;
		}
	}
	else {
		done = false;

		util::Point<float> entity_offset = entity->get_offset();
		float entity_speed = entity->get_speed();
		util::Point<float> diff = run_offset - entity_offset;
		util::Point<float> next_offset;
		if (diff.length() <= entity_speed) {
			next_offset = run_offset;
			jumping = true;
			jump_start = 0;
			gfx::Sprite *entity_sprite = entity->get_sprite();

			if (start_tile.x < dest_tile.x) {
				entity_sprite->set_animation("jump_e");
			}
			else {
				entity_sprite->set_animation("jump_w");
			}

			M3_GLOBALS->jump->play(false);
		}
		else {
			float angle = diff.angle();
			next_offset = util::Point<float>(entity_offset.x+cos(angle)*entity_speed, entity_offset.y+sin(angle)*entity_speed);
		}
		entity->set_offset(next_offset);
	}

	if (done) {
		send_done_signal();
	}

	return done == false;
}
