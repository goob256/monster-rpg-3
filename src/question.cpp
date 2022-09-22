#include <Nooskewl_Wedge/general.h>

#include "dialogue.h"
#include "gui.h"
#include "question.h"

static void callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Question_Step *q = static_cast<Question_Step *>(d->userdata);
	q->set_choice(d->choice);
}

Question_Step::Question_Step(std::vector<std::string> choices, wedge::Task *task) :
	wedge::Step(task),
	choice(-1),
	choices(choices),
	dialogue_step(NULL),
	done(false)
{
}

Question_Step::~Question_Step()
{
}

bool Question_Step::run()
{
	return done == false;
}

void Question_Step::done_signal(wedge::Step *step)
{
	int HEIGHT, y;
	util::Size<int> PAD;
	util::Point<int> text_pos;
	int indicator_pos;
	Dialogue_Step::get_positions(&HEIGHT, &indicator_pos, &y, &PAD, &text_pos, wedge::DIALOGUE_BOTTOM);
	HEIGHT += indicator_pos - 1; // spacing
	Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(false, "", choices, -1, 1, -1, 0, 0, HEIGHT, 0, 0.03f, 0.03f, callback, this, 2, 50, true);
	gui->use_enlarge_transition(false);
	gui->use_shrink_transition(false);
	gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
	shim::guis.push_back(gui);

	dialogue_step = static_cast<Dialogue_Step *>(step); // save for later
}

int Question_Step::get_choice()
{
	return choice;
}

void Question_Step::set_choice(int choice)
{
	this->choice = choice;
	if (dialogue_step) {
		dialogue_step->dismiss();
	}
	send_done_signal();
	done = true;
}
