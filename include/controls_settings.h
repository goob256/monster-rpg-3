#ifndef CONTROLS_SETTINGS_H
#define CONTROLS_SETTINGS_H

#include <Nooskewl_Wedge/main.h>

#include "widgets.h"

class Controls_Settings_GUI : public gui::GUI {
public:
	Controls_Settings_GUI(bool keyboard);
	virtual ~Controls_Settings_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	void draw();
	void draw_fore();

	void quit(bool apply);

	void transition_in_done();

private:
	void set_text();
	std::string get_key_name(int code);

	Widget_Controls_List *list;

	bool keyboard;

	enum Control {
		B1 = 0,
		B2,
		B3,
		B4,
		SWITCH,
		FS,
		L,
		R,
		U,
		D,
		CONTROL_SIZE
	};

	int controls[CONTROL_SIZE];

	bool assigning;
	Control which_assigning;
};

#endif // CONTROLS_SETTINGS_H
