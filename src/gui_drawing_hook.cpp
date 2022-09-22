#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/omnipresent.h>

#include "gui_drawing_hook.h"

GUI_Drawing_Hook_Step::GUI_Drawing_Hook_Step(gui::GUI *gui, bool hook_draw_last) :
	wedge::Step(NULL),
	gui(gui),
	hook_draw_last(hook_draw_last)
{
}

GUI_Drawing_Hook_Step::~GUI_Drawing_Hook_Step()
{
}

void GUI_Drawing_Hook_Step::hook()
{
	if (hook_draw_last) {
		OMNIPRESENT->hook_draw_last(this);
	}
	else {
		OMNIPRESENT->hook_draw_fore(this);
	}
}

void GUI_Drawing_Hook_Step::draw_fore()
{
	gui->draw_fore();
}
