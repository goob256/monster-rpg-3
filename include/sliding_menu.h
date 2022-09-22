#ifndef SLIDING_MENU_H
#define SLIDING_MENU_H

#include <Nooskewl_Wedge/main.h>

#include "widgets.h"

class Sliding_Menu_GUI : public gui::GUI {
public:
	static const int FADE_TIME = 250;

	Sliding_Menu_GUI();
	virtual ~Sliding_Menu_GUI();

	void update();
	void draw();
	void draw_fore();

protected:
	void set_caption(std::string caption);
	void set_fading(bool in, bool out);

	Widget_List *list;

	bool fading_in;
	bool fading_out;
	Uint32 fade_start;
	SDL_Colour clear_colour;
	
	glm::mat4 modelview, proj;

	std::string caption;
};

#endif // SLIDING_MENU_H
