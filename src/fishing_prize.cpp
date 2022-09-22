#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/slide_entity.h>

#include "fishing_prize.h"
#include "globals.h"
#include "play_animation_and_delete.h"
#include "sailor.h"

Fishing_Prize_Step::Fishing_Prize_Step(wedge::Task *task) :
	wedge::Step(task),
	frame(0)
{
}

Fishing_Prize_Step::~Fishing_Prize_Step()
{
}

bool Fishing_Prize_Step::run()
{
	wedge::Map_Entity *eny = AREA->get_player(ENY);

	wedge::Area *area = AREA->get_current_area();

	gfx::Sprite *sprite = eny->get_sprite();
	std::string anim = sprite->get_animation();
	int curr_frame = sprite->get_current_frame();
	util::Point<int> pos = eny->get_position();

	if (anim != "fish") {
		curr_frame = 1000; // all done
	}

	if (curr_frame >= 9 && frame < 9) {
		frame = 9;

		if (M3_INSTANCE->fish_caught < NUM_CATCHES) {
			M3_GLOBALS->splash->play(false);

			util::Point<int> splash_pos = pos;
			splash_pos.y -= 2;
			
			spawn_splash(splash_pos);
		}
	}
	else if (curr_frame >= 12 && frame < 12) {
		frame = 12;

		if (M3_INSTANCE->fish_caught < NUM_CATCHES) {
			M3_GLOBALS->splash->play(false);

			gfx::Sprite *prize_sprite;
			if (M3_INSTANCE->fish_caught == NUM_CATCHES-1) {
				prize_sprite = new gfx::Sprite("second_chance");
			}
			else {
				prize_sprite = new gfx::Sprite("fish");
			}
			Sailor *fishing_prize = new Sailor("fishing_prize");
			util::Point<int> prize_pos = pos;
			prize_pos.y--;
			fishing_prize->start(area);
			fishing_prize->set_position(prize_pos);
			fishing_prize->set_sprite(prize_sprite);
			area->add_entity(fishing_prize);
		
			wedge::Map_Entity *sailship = area->find_entity("sailship_low");
			if (sailship != NULL) {
				gfx::Sprite *sprite = sailship->get_sprite();
				gfx::Image *img = sprite->get_current_image();
				util::Point<float> sailship_centre = sailship->get_position() * shim::tile_size - util::Point<int>(0, (img->size.h - shim::tile_size)) + util::Point<float>(img->size.w/2.0f, img->size.h*4.0f/5.0f);

				fishing_prize->set_use_pivot(true);
				fishing_prize->set_pivot(sailship_centre);
			}

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Slide_Entity_Step(fishing_prize, prize_pos+util::Point<int>(0, -1), 0.8f, new_task))
			ADD_STEP(new wedge::Slide_Entity_Step(fishing_prize, prize_pos+util::Point<int>(1, 2), 1.6f, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
		
			util::Point<int> splash_pos = pos;
			splash_pos.y -= 1;
			
			spawn_splash(splash_pos);
		}
	}
	else if (curr_frame >= 13 && frame < 13) {
		send_done_signal();
		return false;
	}

	return true;
}

void Fishing_Prize_Step::spawn_splash(util::Point<int> pos)
{
	wedge::Area *area = AREA->get_current_area();
	wedge::Map_Entity *splash = new wedge::Map_Entity("splash");
	splash->start(area);
	splash->set_position(pos);
	splash->set_sprite(new gfx::Sprite("splash"));
	area->add_entity(splash);

	NEW_SYSTEM_AND_TASK(AREA)
	ADD_STEP(new Play_Animation_And_Delete_Step(splash, "only", new_task))
	ADD_TASK(new_task)
	FINISH_SYSTEM(AREA)

	M3_GLOBALS->splash->play(false);
}
