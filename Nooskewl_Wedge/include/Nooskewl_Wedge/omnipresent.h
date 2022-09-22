#ifndef NOOSKEWL_WEDGE_OMNIPRESENT_GAME_H
#define NOOSKEWL_WEDGE_OMNIPRESENT_GAME_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Omnipresent_Game : public Game
{
public:
	Omnipresent_Game();
	virtual ~Omnipresent_Game();
	
	virtual void handle_event(TGUI_Event *event);
	virtual void draw_fore();
	virtual bool run();
	void draw_last();

	void start_fade(SDL_Colour colour, int delay, int duration);
	void end_fade();

	void hook_draw_fore(Step *step);
	void hook_draw_last(Step *step);

	void set_hide_red_triangle(bool hide);

	void set_quit_game(bool quit);

private:
	bool fading;
	Uint32 fade_start;
	int fade_delay;
	int fade_duration;
	SDL_Colour fade_colour;
	std::vector<Step *> draw_fore_hooks;
	std::vector<Step *> draw_last_hooks;
	bool hide_red_triangle;
	bool quit_game;
};

}

#endif // NOOSKEWL_WEDGE_OMNIPRESENT_GAME_H
