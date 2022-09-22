#ifndef LANGUAGE_SETTINGS_H
#define LANGUAGE_SETTINGS_H

#include <Nooskewl_Wedge/main.h>

#include "widgets.h"

class Language_Settings_GUI : public gui::GUI {
public:
	Language_Settings_GUI();
	virtual ~Language_Settings_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	void draw();
	void draw_fore();

	void transition_in_done();

private:
	void set_text();

	Widget_List *list;
	std::map<std::string, std::string> languages;
};

#endif // LANGUAGE_SETTINGS_H
