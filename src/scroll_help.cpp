#include <Nooskewl_Wedge/globals.h>

#include "dialogue.h"
#include "general.h"
#include "question.h"
#include "milestones.h"
#include "scroll_help.h"

Scroll_Help_Step::Scroll_Help_Step(std::string tag, wedge::Task *task) :
	wedge::Step(task),
	done(false),
	tag(tag),
	count(0)
{
}

Scroll_Help_Step::~Scroll_Help_Step()
{
}

void Scroll_Help_Step::start()
{
	if (INSTANCE->is_milestone_complete(MS_GOT_SCROLL_HELP)) {
		// Only offer scroll help if it's never been seen before
		done = true;
		return;
	}
	INSTANCE->set_milestone_complete(MS_GOT_SCROLL_HELP, true);
	std::vector<std::string> v;
	v.push_back(GLOBALS->game_t->translate(1729)/* Originally: Yes */);
	v.push_back(GLOBALS->game_t->translate(1730)/* Originally: No */);
	do_question(tag + TAG_END, GLOBALS->game_t->translate(1728)/* Originally: Do you want to learn about Scrolls? */, wedge::DIALOGUE_SPEECH, v, this);
}

bool Scroll_Help_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return done == false;
}

void Scroll_Help_Step::done_signal(wedge::Step *step)
{
	if (count == 0) {
		Question_Step *q = dynamic_cast<Question_Step *>(step);
		if (q && q->get_choice() == 0) {
			wedge::globals->do_dialogue(tag + TAG_END, GLOBALS->game_t->translate(1514)/* Originally: Whoever reads a Scroll learns its spell. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1515)/* Originally: Scrolls disappear after they've been read once. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, this);
		}
		else {
			wedge::globals->do_dialogue(tag + TAG_END, GLOBALS->game_t->translate(1516)/* Originally: OK, suit yourself. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, this);
		}
	}
	else {
		done = true;
	}
	count++;
}
