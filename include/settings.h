#ifndef SETTINGS_H
#define SETTINGS_H

#include <Nooskewl_Wedge/main.h>

#include "sliding_menu.h"
#include "widgets.h"

class Settings_GUI : public Sliding_Menu_GUI {
public:
	static void static_start();

	enum Setting {
		LANGUAGE = 0,
		VIDEO,
		AUDIO,
		OTHER,
		KEYBOARD,
		JOYSTICK,
#if defined GOOGLE_PLAY || defined AMAZON
		ACHIEVEMENTS,
		CLOUD_SAVES,
#endif
		THIRD_PARTY,
	};

	Settings_GUI(bool fade_in, Setting selected, int top = 0);
	virtual ~Settings_GUI();

	void handle_event(TGUI_Event *event);
	void update();

private:
	bool video_enabled;
	bool keyboard_enabled;
	bool joystick_enabled;

	static bool unset_b3_b4;
	static bool b3_enabled;
	static bool b4_enabled;
	bool changed_to_another_settings_gui;
};

#endif // SETTINGS_H
