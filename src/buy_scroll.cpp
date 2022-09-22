#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/globals.h>

#include "buy_scroll.h"
#include "globals.h"
#include "question.h"
#include "scroll_help.h"

Buy_Scroll_Step::Buy_Scroll_Step(int item_id, int cost, wedge::Task *task) :
	wedge::Step(task),
	item_id(item_id),
	cost(cost),
	done(false)
{
}

Buy_Scroll_Step::~Buy_Scroll_Step()
{
}

bool Buy_Scroll_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return !done;
}

void Buy_Scroll_Step::done_signal(wedge::Step *step)
{
	done = true;

	Question_Step *q = static_cast<Question_Step *>(step);

	int choice = q->get_choice();

	if (choice == 0) {
		if (INSTANCE->get_gold() < cost) {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, util::string_printf(GLOBALS->game_t->translate(1363)/* Originally: Did I say %d kisses? I meant GOLD! */.c_str(), cost), wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
		}
		else {
			M3_GLOBALS->buysell->play(false);
			INSTANCE->add_gold(-cost);
			wedge::Object o = OBJECT->make_object(wedge::OBJECT_ITEM, item_id, 1);
			INSTANCE->inventory.add(o);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new Scroll_Help_Step(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
		}
	}
	else {
		GLOBALS->do_dialogue(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1366)/* Originally: Your loss! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
	}
}
