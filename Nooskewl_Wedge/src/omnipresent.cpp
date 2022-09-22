#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/omnipresent.h"

using namespace wedge;

namespace wedge {

Omnipresent_Game::Omnipresent_Game() :
	fading(false),
	hide_red_triangle(false),
	quit_game(false)
{
}

Omnipresent_Game::~Omnipresent_Game()
{
}
	
void Omnipresent_Game::handle_event(TGUI_Event *event)
{
#ifndef TVOS
	// pump escape key events when top left triangle clicked
	if (hide_red_triangle == false && event->type == TGUI_MOUSE_DOWN && event->mouse.is_repeat == false) {
		if (event->mouse.x < shim::tile_size && event->mouse.y < shim::tile_size && event->mouse.x < shim::tile_size-event->mouse.y) {
			SDL_Event ev;
			ev.type = SDL_KEYDOWN;
			ev.key.keysym.sym = GLOBALS->key_b2;
			ev.key.repeat = 0;
			SDL_PushEvent(&ev);
		}
	}
#endif
}

void Omnipresent_Game::draw_fore()
{
	for (size_t i = 0; i < draw_fore_hooks.size(); i++) {
		draw_fore_hooks[i]->draw_fore();
	}
	draw_fore_hooks.clear();

	gfx::Font::end_batches();

	if (fading) {
		Uint32 now = GET_TICKS();
		Uint32 end = fade_start + fade_delay + fade_duration;
		Uint32 diff;
		if (now > end) {
			diff = 0;
		}
		else {
			diff = end-now;
		}
		if ((int)diff <= fade_duration) {
			float p = 1.0f - ((float)diff / fade_duration);
			SDL_Colour colour;
			colour.r = fade_colour.r * p;
			colour.g = fade_colour.g * p;
			colour.b = fade_colour.b * p;
			colour.a = fade_colour.a * p;
			gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
		}
	}

	for (size_t i = 0; i < draw_last_hooks.size(); i++) {
		draw_last_hooks[i]->draw_fore();
	}
	draw_last_hooks.clear();

#ifndef TVOS
	if (hide_red_triangle == false) {
		SDL_Colour colour = GLOBALS->red_triangle_colour;
		colour.r *= 0.5f;
		colour.g *= 0.5f;
		colour.b *= 0.5f;
		colour.a *= 0.5f;
		SDL_Colour colours[3] = { colour, colour, colour };
		gfx::draw_filled_triangle(colours, util::Point<float>(0.0f, 0.0f), util::Point<float>(shim::tile_size-1.0f, 0.0f), util::Point<float>(0.0f, shim::tile_size-1.0f));
	}
#endif
}

bool Omnipresent_Game::run()
{
	if (quit_game) {
		quit_game = false;
		GLOBALS->quit(true);
	}
	return true;
}

void Omnipresent_Game::start_fade(SDL_Colour colour, int delay, int duration)
{
	fade_colour = colour;
	fade_delay = delay;
	fade_duration = duration;
	fading = true;
	fade_start = GET_TICKS();
}

void Omnipresent_Game::end_fade()
{
	fading = false;
}

void Omnipresent_Game::hook_draw_fore(Step *step)
{
	draw_fore_hooks.push_back(step);
}

void Omnipresent_Game::hook_draw_last(Step *step)
{
	draw_last_hooks.push_back(step);
}

void Omnipresent_Game::set_hide_red_triangle(bool hide)
{
	hide_red_triangle = hide;
}

void Omnipresent_Game::set_quit_game(bool quit)
{
	quit_game = quit;
}

}
