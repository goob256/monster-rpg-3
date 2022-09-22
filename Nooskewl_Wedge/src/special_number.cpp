#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/omnipresent.h"
#include "Nooskewl_Wedge/special_number.h"

using namespace wedge;

namespace wedge {

Special_Number_Step::Special_Number_Step(SDL_Colour colour, SDL_Colour shadow_colour, std::string text, util::Point<int> position, Movement_Type movement_type, Task *task, bool draw_last) :
	Step(task),
	colour(colour),
	shadow_colour(shadow_colour),
	text(text),
	position(position),
	movement_type(movement_type),
	count(0),
	draw_last(draw_last)
{
}

Special_Number_Step::~Special_Number_Step()
{
}

bool Special_Number_Step::run()
{
	bool done = (GET_TICKS() >= start_time + DISPLAY_TIME);
	if (done) {
		send_done_signal();
	}
	return !done;
}

void Special_Number_Step::draw_fore()
{
	// this Step hooks OMNIPRESENT::draw_fore
	int mod = count % 2;
	count++;
	if (mod == 0) {
		if (draw_last) {
			OMNIPRESENT->hook_draw_last(this);
		}
		else {
			OMNIPRESENT->hook_draw_fore(this);
		}
		return;
	}

	util::Point<float> offset(0.0f, 0.0f);

	if (movement_type == SHAKE) {
		offset.x = util::rand(0, 1000) / 1000.0f;
		offset.y = util::rand(0, 1000) / 1000.0f;
	}
	else if (movement_type == RISE) {
		float p = MIN(1.0f, (GET_TICKS() - start_time) / (float)DISPLAY_TIME);
		offset.y = p * RISE_MAX_OFFSET;
	}

	shim::font->enable_shadow(shadow_colour, gfx::Font::FULL_SHADOW);
	shim::font->draw(colour, text, position + offset);
	shim::font->disable_shadow();
}

void Special_Number_Step::start()
{
	start_time = GET_TICKS();
}

}
