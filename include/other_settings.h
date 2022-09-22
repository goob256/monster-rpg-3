#ifndef OTHER_SETTINGS_H
#define OTHER_SETTINGS_H

#include <Nooskewl_Wedge/main.h>

#include "widgets.h"

class Other_Settings_GUI : public gui::GUI {
public:
	Other_Settings_GUI();
	virtual ~Other_Settings_GUI();

	void handle_event(TGUI_Event *event);
	void draw_fore();
	void draw();

	void transition_in_done();

	void exit();

private:
	Widget_Checkbox *safe_mode_checkbox;
	Widget_Checkbox *onscreen_controller_checkbox;
	Widget_Checkbox *rumble_enabled_checkbox;
	Widget_Checkbox *hide_onscreen_settings_button_checkbox;
	Widget_Checkbox *hires_font_checkbox;
};

#endif // OTHER_SETTINGS_H
