#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/map_entity.h"

#include "dialogue.h"
#include "globals.h"

Dialogue_Step::Dialogue_Step(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Task *task) :
	wedge::Step(task),
	tag(tag),
	text(text),
	type(type),
	position(position),
	done(false)
{
}

Dialogue_Step::~Dialogue_Step()
{
}

bool Dialogue_Step::run()
{
	return done == false;
}

void Dialogue_Step::handle_event(TGUI_Event *event)
{
	if (event->type == TGUI_KEY_DOWN || event->type == TGUI_MOUSE_DOWN || event->type == TGUI_JOY_DOWN) {
		send_done_signal();
		done = true;
	}
}

void Dialogue_Step::draw()
{
	int PAD = 3;
	int height = shim::screen_size.h/2;

	int y;

	if (position == wedge::DIALOGUE_TOP) {
		y = PAD;
	}

	else {
		y = shim::screen_size.h - height - PAD;
	}

	gfx::draw_filled_rectangle(shim::white, util::Point<int>(PAD, y), util::Size<int>(shim::screen_size.w-PAD*2, height));
	gfx::draw_rectangle(shim::black, util::Point<int>(PAD, y), util::Size<int>(shim::screen_size.w-PAD*2, height));

	shim::font->draw(shim::black, tag + (tag == "" ? "" : ": ") + text, util::Point<int>(PAD*2, y+PAD));
}
