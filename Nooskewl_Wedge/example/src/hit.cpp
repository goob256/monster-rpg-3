#include <Nooskewl_Wedge/battle_entity.h>
#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/globals.h>

#include "hit.h"

Hit_Step::Hit_Step(wedge::Battle_Entity *entity, int delay, wedge::Task *task) :
	wedge::Step(task),
	entity(entity),
	sprite(NULL),
	done(false),
	started(false)
{
	// don't put this in start
	start_time = SDL_GetTicks() + delay;
	sprite = new gfx::Sprite("hit");
	hit_size = sprite->get_current_image()->size;

	entity_sprite = entity->get_sprite();
	prev_anim = entity_sprite->get_animation();
	entity_sprite->set_animation("hit");
	util::Size<int> entity_size = entity_sprite->get_current_image()->size;
	draw_pos = entity->get_decoration_offset(hit_size.w, util::Point<int>(1, hit_size.h > entity_size.h ? entity_size.h-hit_size.h : 0), &flags);
}

Hit_Step::~Hit_Step()
{
	delete sprite;
}

bool Hit_Step::run()
{
	return !done;
}

void Hit_Step::draw()
{
	if (started == false) {
		Uint32 now = SDL_GetTicks();
		if (now >= start_time) {
			started = true;
			sprite->reset();
			sprite->start();
		}
	}

	if (started) {
		done = sprite->is_finished();
		
		if (done == false) {
			sprite->get_current_image()->draw(draw_pos, flags);
		}
		else {
			std::vector<wedge::Battle_Entity *> entities = BATTLE->get_all_entities();
			// don't change anim if dead
			if (std::find(entities.begin(), entities.end(), entity) != entities.end()) {
				entity_sprite->set_animation(prev_anim);
			}
			send_done_signal();
		}
	}
}
