#ifndef NOOSKEWL_WEDGE_ONSCREEN_CONTROLLER
#define NOOSKEWL_WEDGE_ONSCREEN_CONTROLLER

#include "Nooskewl_Wedge/main.h"

namespace wedge {

enum Onscreen_Button {
	ONSCREEN_IGNORE = 0,
	ONSCREEN_NONE,
	ONSCREEN_UP,
	ONSCREEN_RIGHT,
	ONSCREEN_DOWN,
	ONSCREEN_LEFT,
	ONSCREEN_B1,
	ONSCREEN_B2,
	ONSCREEN_B3,
	ONSCREEN_B4
};

void NOOSKEWL_WEDGE_EXPORT enable_onscreen_controller(bool enabled);
bool NOOSKEWL_WEDGE_EXPORT is_onscreen_controller_enabled();
void NOOSKEWL_WEDGE_EXPORT set_onscreen_controller_b2_enabled(bool enabled);
void NOOSKEWL_WEDGE_EXPORT set_onscreen_controller_b3_enabled(bool enabled, int key);
void NOOSKEWL_WEDGE_EXPORT set_onscreen_controller_b4_enabled(bool enabled, int key);
bool NOOSKEWL_WEDGE_EXPORT get_onscreen_controller_b3_enabled();
bool NOOSKEWL_WEDGE_EXPORT get_onscreen_controller_b4_enabled();

bool handle_onscreen_controller(SDL_Event *event);
void update_onscreen_controller();
void draw_onscreen_controller();

void start_onscreen_controller(bool generate_repeats);

}

#endif // NOOSKEWL_WEDGE_ONSCREEN_CONTROLLER
