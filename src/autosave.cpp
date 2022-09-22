#include "autosave.h"
#include "general.h"

Autosave_Step::Autosave_Step(wedge::Task *task) :
	Step(task)
{
}

Autosave_Step::~Autosave_Step()
{
}

bool Autosave_Step::run()
{
	if (can_autosave()) {
		autosave(false);
		return false;
	}
	else {
		return true;
	}
}
