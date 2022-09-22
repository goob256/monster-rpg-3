#include <Nooskewl_Wedge/general.h>

#include "general.h"
#include "heal.h"

Heal_Step::Heal_Step(util::Point<int> draw_pos, util::Size<int> size, int pulse, wedge::Task *task) :
	wedge::Step(task),
	draw_pos(draw_pos),
	size(size),
	pulse(pulse)
{
	SLOPE = MIN(shim::tile_size, size.w);
	SEGMENTS = MIN(SLOPE, size.h);
}

Heal_Step::~Heal_Step()
{
}

bool Heal_Step::run()
{
	Uint32 elapsed = GET_TICKS() - start_time;
	bool ret = elapsed < DURATION;
	if (ret == false) {
		send_done_signal();
	}
	return ret;
}

void Heal_Step::draw_fore()
{
	Uint32 elapsed = GET_TICKS() - start_time;
	float p = elapsed / (float)DURATION;
	if (p > 1.0f) {
		p = 1.0f;
	}

	bool reverse = p >= 0.5f;

	if (reverse) {
		p = 1.0f - ((p - 0.5f) / 0.5f);
	}
	else {
		p = p / 0.5f;
	}

	float alpha;
	const int alpha_phase = 1000;
	const int half_alpha_phase = alpha_phase / 2;
	Uint32 ticks = GET_TICKS() % alpha_phase;
	if (ticks < half_alpha_phase) {
		alpha = ticks/(float)half_alpha_phase * 0.5f;
	}
	else {
		alpha = (half_alpha_phase-(ticks-half_alpha_phase))/(float)half_alpha_phase * 0.5f;
	}

	alpha += 0.25f;

	start_pulse_brighten(1.5f, true, false, pulse);
	
	gfx::draw_primitives_start();

	int extra = shim::tile_size/2;
	int bars = p * (size.w + SLOPE + extra);
	int full = bars - SLOPE;

	int i;

	for (i = 0; i < bars && i < size.w; i++) {
		int length;
		
		length = i > full ? SEGMENTS - (i-full) : SEGMENTS;

		int x = draw_pos.x + (reverse ? size.w-i-1 : i);

		float l = length / (float)SEGMENTS;

		l = l * l;

		SDL_Colour c1 = shim::palette[17];
		SDL_Colour c2 = shim::white;

		SDL_Colour colour;

		colour.r = (c1.r + l * (c2.r - c1.r)) * alpha;
		colour.g = (c1.g + l * (c2.g - c1.g)) * alpha;
		colour.b = (c1.b + l * (c2.b - c1.b)) * alpha;
		colour.a = 255 * alpha;

		SDL_Colour colours[4];

		colours[0] = shim::palette[17];
		colours[1] = shim::palette[17];
		colours[2] = colour;
		colours[3] = colour;

		gfx::draw_filled_rectangle(colours, util::Point<int>(x, draw_pos.y), util::Size<int>(1, l * size.h));
	}

	i--;

	int x = draw_pos.x + (reverse ? size.w-i-1 : 0);

	SDL_Colour colour = shim::palette[18];
	colour.r *= alpha;
	colour.g *= alpha;
	colour.b *= alpha;
	colour.a = alpha * 255;

	gfx::draw_filled_rectangle(colour, util::Point<int>(x, draw_pos.y), util::Size<int>(i+1, 1));

	gfx::draw_primitives_end();

	end_pulse_brighten();
}

void Heal_Step::start()
{
	start_time = GET_TICKS();
}
