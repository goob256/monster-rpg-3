#ifndef AUDIO_SETTINGS_H
#define AUDIO_SETTINGS_H

#include <Nooskewl_Wedge/main.h>

#include "widgets.h"

class Audio_Settings_GUI : public gui::GUI {
public:
	Audio_Settings_GUI();
	virtual ~Audio_Settings_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	void draw_fore();

	void transition_in_done();

private:
	Widget_Slider *sfx_slider;
	Widget_Slider *music_slider;
};

#endif // AUDIO_SETTINGS_H
