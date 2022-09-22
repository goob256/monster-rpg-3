#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/fade.h"
#include "Nooskewl_Wedge/globals.h"

using namespace wedge;

namespace wedge {

Fade_Step::Fade_Step(SDL_Colour colour, bool out, int length, Task *task) :
	Step(task),
	colour(colour),
	out(out),
	length(length)
{
}

Fade_Step::~Fade_Step()
{
}

bool Fade_Step::run()
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - start_time;

	bool done = elapsed >= (Uint32)length;

	if (done) {
		elapsed = length;
	}
	float p = elapsed / (float)length;
	if (out == false) {
		p = 1.0f - p;
	}
	SDL_Colour c;
	c.r = colour.r * p;
	c.g = colour.g * p;
	c.b = colour.b * p;
	c.a = 255.0f * p;

	AREA->get_current_area()->set_overlay_colour(c);

	if (done) {
		send_done_signal();
	}

	return done == false;
}

void Fade_Step::start()
{
	start_time = GET_TICKS();
}

}
