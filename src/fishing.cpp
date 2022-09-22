#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/generic_callback.h>
#include <Nooskewl_Wedge/give_object.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/inventory.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/play_animation.h>
#include <Nooskewl_Wedge/pause_presses.h>

#include "achievements.h"
#include "fishing.h"
#include "fishing_prize.h"
#include "globals.h"
#include "inventory.h"
#include "question.h"

static void callback2(void *data)
{
	Fishing_Step *fs = static_cast<Fishing_Step *>(data);
	fs->set_done(true);
}

static void callback(void *data)
{
	AREA->get_player(ENY)->set_direction(wedge::DIR_S, true, false);
	
	NEW_SYSTEM_AND_TASK(AREA)
	wedge::Generic_Callback_Step *gcs = new wedge::Generic_Callback_Step(callback2, data, new_task);
	ADD_STEP(gcs)
	ADD_TASK(new_task)
	FINISH_SYSTEM(AREA)

	GLOBALS->add_next_dialogue_monitor(gcs);

	if (M3_INSTANCE->fish_caught < NUM_CATCHES) {
		M3_INSTANCE->fish_caught++;
		wedge::Object o;
		if (M3_INSTANCE->fish_caught == NUM_CATCHES) {
			o = OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_SECOND_CHANCE, 1);
			util::achieve((void *)ACHIEVE_LUNKER);
		}
		else {
			o = OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_FISH, 1);
		}

		NEW_SYSTEM_AND_TASK(AREA)
		ADD_STEP(new wedge::Give_Object_Step(o, wedge::DIALOGUE_AUTO, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)

		wedge::Area *area = AREA->get_current_area();
		area->remove_entity(area->find_entity("fishing_prize"), true);
	}
	else {
		GLOBALS->do_dialogue(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1431)/* Originally: Fish aren't biting... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
	}
}

Fishing_Step::Fishing_Step(wedge::Task *task) :
	wedge::Step(task),
	done(false)
{
}

Fishing_Step::~Fishing_Step()
{
}

bool Fishing_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return !done;
}

void Fishing_Step::done_signal(wedge::Step *step)
{
	Question_Step *q = static_cast<Question_Step *>(step);

	int choice = q->get_choice();

	if (choice == 0) {
		wedge::Object o = OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_BAIT, 1);
		int index;
		if ((index = INSTANCE->inventory.find(o)) < 0) {
			NEW_SYSTEM_AND_TASK(AREA)
			wedge::Generic_Callback_Step *gcs = new wedge::Generic_Callback_Step(callback2, this, new_task);
			ADD_STEP(gcs)
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			GLOBALS->do_dialogue("", GLOBALS->game_t->translate(1432)/* Originally: You have no bait. */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_AUTO, gcs);
		}
		else {
			INSTANCE->inventory.remove(index, 1);

			wedge::Map_Entity *eny = AREA->get_player(ENY);
			gfx::Sprite *sprite = eny->get_sprite();

			if (M3_INSTANCE->fish_caught >= NUM_CATCHES) {
				sprite->set_animation("fish_no_catch", callback, this);
			}
			else {
				sprite->set_animation("fish", callback, this);

				NEW_SYSTEM_AND_TASK(AREA)
				ADD_STEP(new Fishing_Prize_Step(new_task))
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)
			}

			M3_GLOBALS->cast_line->play(false);
			
			wedge::pause_presses(true);
		}
	}
	else {
		done = true;
	}
}

void Fishing_Step::set_done(bool done)
{
	this->done = done;
	wedge::pause_presses(false);

}
