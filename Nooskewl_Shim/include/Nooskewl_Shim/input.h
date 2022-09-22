#ifndef NOO_INPUT_H
#define NOO_INPUT_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace input {

struct NOOSKEWL_SHIM_EXPORT Focus_Event : public TGUI_Event {
	TGUI_Event_Type orig_type;
	union {
		TGUI_Event::TGUI_Event_Keyboard orig_keyboard;
		TGUI_Event::TGUI_Event_Joystick orig_joystick;
	} u;

	virtual ~Focus_Event();
};

bool start();
void reset();
void end();
void update();
void handle_event(TGUI_Event *event);

bool NOOSKEWL_SHIM_EXPORT convert_to_focus_event(TGUI_Event *event, Focus_Event *focus);
void NOOSKEWL_SHIM_EXPORT convert_focus_to_original(TGUI_Event *event);
void NOOSKEWL_SHIM_EXPORT rumble(float strength, Uint32 length);
bool NOOSKEWL_SHIM_EXPORT is_joystick_connected();
std::string NOOSKEWL_SHIM_EXPORT joystick_button_to_name(int button);
void NOOSKEWL_SHIM_EXPORT drop_repeats(bool joystick = true, bool mouse = true);

} // End namespace input

} // End namespace noo

#endif // NOO_INPUT_H
