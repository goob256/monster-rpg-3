#include "transition.h"

Transition_Step::Transition_Step(bool out, wedge::Task *task) :
	wedge::Step(task),
	out(out)
{
}

Transition_Step::~Transition_Step()
{
}

bool Transition_Step::run()
{
	Uint32 now = SDL_GetTicks();
	Uint32 elapsed = now - start_time;

	return elapsed < LENGTH;
}

void Transition_Step::draw()
{
	SDL_Colour colours1[4];
	SDL_Colour colours2[4];

	int xblack1, xblack2;
	int xquad1;

	Uint32 now = SDL_GetTicks();
	Uint32 elapsed = now - start_time;
	float p = MIN(1.0f, elapsed / (float)LENGTH);

	p = p * p;

	const int quad_size = 32;
	
	xquad1 = p * (shim::screen_size.w + quad_size) - quad_size;

	if (out) {
		colours1[0] = colours1[3] = shim::black;
		colours1[1] = colours1[2] = shim::transparent;
		xblack1 = 0;
		xblack2 = xquad1 - 1;
	}
	else {
		colours1[0] = colours1[3] = shim::transparent;
		colours1[1] = colours1[2] = shim::black;
		xblack1 = xquad1 + quad_size;
		xblack2 = shim::screen_size.w;
	}

	colours2[0] = colours2[1] = colours2[2] = colours2[3] = shim::black;

	gfx::Vertex_Cache::instance()->start();
	gfx::Vertex_Cache::instance()->cache(colours1, util::Point<float>(0, 0), util::Size<float>(1, 1), util::Point<float>(xquad1, 0), util::Size<float>(quad_size, shim::screen_size.h), 0);
	gfx::Vertex_Cache::instance()->cache(colours2, util::Point<float>(0, 0), util::Size<float>(1, 1), util::Point<float>(xblack1, 0), util::Size<float>(xblack2-xblack1+1, shim::screen_size.h), 0);
	gfx::Vertex_Cache::instance()->end();
}

void Transition_Step::start()
{
	start_time = SDL_GetTicks();
}
