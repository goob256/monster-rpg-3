#ifndef VIDEO_SETTINGS_H
#define VIDEO_SETTINGS_H

#include <Nooskewl_Wedge/main.h>

#include "widgets.h"

class Video_Settings_GUI : public gui::GUI {
public:
	Video_Settings_GUI();
	virtual ~Video_Settings_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	void draw();
	void draw_fore();
	void found_device();

	void transition_in_done();

private:
	void set_text();
	void set_selected();

	Widget_List *list;

	std::vector< util::Size<int> > windowed_modes;
	std::vector< util::Size<int> > fullscreen_modes;
};

#endif // VIDEO_SETTINGS_H
