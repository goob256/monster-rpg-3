#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/generic_immediate_callback.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/map_entity.h>

#include "captain.h"
#include "globals.h"
#include "question.h"

static void weigh_anchor(void *data)
{
	M3_INSTANCE->boatin = true;
	std::vector< util::Point<int> > positions;
	positions.push_back(util::Point<int>(0, 0));
	positions.push_back(util::Point<int>(0, 0));
	std::vector<wedge::Direction> directions;
	directions.push_back(wedge::DIR_S);
	directions.push_back(wedge::DIR_S);
	wedge::Area *area = AREA->get_current_area();
	area->find_entity("captain")->set_direction(wedge::DIR_E, true, false);
	area->set_next_area("boatin", positions, directions);
}

static void turn(void *data)
{
	M3_INSTANCE->boatin = true;
	M3_INSTANCE->boat_w = false;
	std::vector< util::Point<int> > positions;
	positions.push_back(util::Point<int>(0, 0));
	positions.push_back(util::Point<int>(0, 0));
	std::vector<wedge::Direction> directions;
	directions.push_back(wedge::DIR_S);
	directions.push_back(wedge::DIR_S);
	wedge::Area *area = AREA->get_current_area();
	area->find_entity("captain")->set_direction(wedge::DIR_E, true, false);
	area->set_next_area("boatin", positions, directions);
}

Captain_Step::Captain_Step(wedge::Task *task) :
	wedge::Step(task),
	done(false)
{
}

Captain_Step::~Captain_Step()
{
}

bool Captain_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return !done;
}

void Captain_Step::done_signal(wedge::Step *step)
{
	done = true;

	Question_Step *q = static_cast<Question_Step *>(step);

	int choice = q->get_choice();

	if (choice == 0) {
		NEW_SYSTEM_AND_TASK(AREA)
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1368)/* Originally: Weigh anchor, mates! Away we go! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Generic_Immediate_Callback_Step(weigh_anchor, NULL, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)
	}
	else {
		if (M3_INSTANCE->boatin && choice == 1) {
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1370)/* Originally: Turn her around, mates! We're headin' back! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(turn, NULL, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
		}
		else {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1372)/* Originally: Ready when you are, miss! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
		}
	}
}
