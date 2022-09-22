#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/globals.h>

#include "ran_away.h"

Ran_Away_Step::Ran_Away_Step(wedge::Task *task) :
	wedge::Step(task),
	done(false)
{
}

Ran_Away_Step::~Ran_Away_Step()
{
}

bool Ran_Away_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return !done;
}

void Ran_Away_Step::done_signal(wedge::Step *step)
{
	done = true;
	BATTLE->set_done(true);
}
