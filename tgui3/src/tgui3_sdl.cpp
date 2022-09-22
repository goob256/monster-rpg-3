#include "tgui3/tgui3.h"
#include "tgui3/tgui3_sdl.h"

TGUI_Event tgui_sdl_convert_event(SDL_Event *sdl_event)
{
	TGUI_Event event;

	switch (sdl_event->type) {
		case SDL_QUIT:
			event.type = TGUI_QUIT;
			break;
		case SDL_KEYDOWN:
			event.type = TGUI_KEY_DOWN;
			event.keyboard.code = sdl_event->key.keysym.sym;
			event.keyboard.is_repeat = sdl_event->key.repeat != 0;
			break;
		case SDL_KEYUP:
			event.type = TGUI_KEY_UP;
			event.keyboard.code = sdl_event->key.keysym.sym;
			event.keyboard.is_repeat = sdl_event->key.repeat != 0;
			break;
		case SDL_JOYBUTTONDOWN:
			event.type = TGUI_JOY_DOWN;
			event.joystick.id = sdl_event->jbutton.which;
			event.joystick.button = sdl_event->jbutton.button;
			event.joystick.axis = -1;
			event.joystick.hat_x = 0.0f;
			event.joystick.hat_y = 0.0f;
			event.joystick.value = 0.0f;
			event.joystick.is_repeat = false;
			break;
		case SDL_JOYBUTTONUP:
			event.type = TGUI_JOY_UP;
			event.joystick.id = sdl_event->jbutton.which;
			event.joystick.button = sdl_event->jbutton.button;
			event.joystick.axis = -1;
			event.joystick.hat_x = 0.0f;
			event.joystick.hat_y = 0.0f;
			event.joystick.value = 0.0f;
			event.joystick.is_repeat = false;
			break;
		case SDL_JOYAXISMOTION:
			event.type = TGUI_JOY_AXIS;
			event.joystick.id = sdl_event->jaxis.which;
			event.joystick.button = -1;
			event.joystick.axis = sdl_event->jaxis.axis;
			event.joystick.hat_x = 0.0f;
			event.joystick.hat_y = 0.0f;
			event.joystick.value = TGUI3_NORMALISE_JOY_AXIS(sdl_event->jaxis.value);
			event.joystick.is_repeat = false;
			break;
		case SDL_JOYHATMOTION:
			event.type = TGUI_JOY_HAT;
			event.joystick.id = sdl_event->jhat.which;
			event.joystick.button = -1;
			event.joystick.axis = -1;
			switch (sdl_event->jhat.value) {
				case SDL_HAT_LEFTUP:
					event.joystick.hat_x = -1;
					event.joystick.hat_y = -1;
					break;
				case SDL_HAT_LEFT:
					event.joystick.hat_x = -1;
					event.joystick.hat_y = 0;
					break;
				case SDL_HAT_LEFTDOWN:
					event.joystick.hat_x = -1;
					event.joystick.hat_y = 1;
					break;
				case SDL_HAT_UP:
					event.joystick.hat_x = 0;
					event.joystick.hat_y = -1;
					break;
				case SDL_HAT_CENTERED:
					event.joystick.hat_x = 0;
					event.joystick.hat_y = 0;
					break;
				case SDL_HAT_DOWN:
					event.joystick.hat_x = 0;
					event.joystick.hat_y = 1;
					break;
					break;
				case SDL_HAT_RIGHTUP:
					event.joystick.hat_x = 1;
					event.joystick.hat_y = -1;
					break;
				case SDL_HAT_RIGHT:
					event.joystick.hat_x = 1;
					event.joystick.hat_y = 0;
					break;
				case SDL_HAT_RIGHTDOWN:
					event.joystick.hat_x = 1;
					event.joystick.hat_y = 1;
					break;
			}
			event.joystick.value = 0.0f;
			event.joystick.is_repeat = false;
			break;
#ifndef TVOS
		case SDL_MOUSEBUTTONDOWN:
			if (sdl_event->button.which != SDL_TOUCH_MOUSEID) {
				event.type = TGUI_MOUSE_DOWN;
				event.mouse.button = sdl_event->button.button;
				event.mouse.x = (float)sdl_event->button.x;
				event.mouse.y = (float)sdl_event->button.y;
				event.mouse.normalised = false;
				event.mouse.is_touch = false;
				event.mouse.is_repeat = false;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (sdl_event->button.which != SDL_TOUCH_MOUSEID) {
				event.type = TGUI_MOUSE_UP;
				event.mouse.button = sdl_event->button.button;
				event.mouse.x = (float)sdl_event->button.x;
				event.mouse.y = (float)sdl_event->button.y;
				event.mouse.normalised = false;
				event.mouse.is_touch = false;
				event.mouse.is_repeat = false;
			}
			break;
		case SDL_MOUSEMOTION:
			if (sdl_event->motion.which != SDL_TOUCH_MOUSEID) {
				event.type = TGUI_MOUSE_AXIS;
				event.mouse.button = -1;
				event.mouse.x = (float)sdl_event->motion.x;
				event.mouse.y = (float)sdl_event->motion.y;
				event.mouse.normalised = false;
				event.mouse.is_touch = false;
				event.mouse.is_repeat = false;
			}
			break;
		case SDL_MOUSEWHEEL:
			event.type = TGUI_MOUSE_WHEEL;
			event.mouse.button = -1;
			event.mouse.x = (float)sdl_event->wheel.x;
			event.mouse.y = (float)sdl_event->wheel.y;
			event.mouse.normalised = false;
			break;
		case SDL_FINGERDOWN:
			event.type = TGUI_MOUSE_DOWN;
			event.mouse.button = SDL_BUTTON_LEFT;
			event.mouse.x = (float)sdl_event->tfinger.x;
			event.mouse.y = (float)sdl_event->tfinger.y;
			event.mouse.normalised = true;
			event.mouse.is_touch = true;
			event.mouse.finger = sdl_event->tfinger.fingerId;
			event.mouse.is_repeat = false;
			break;
		case SDL_FINGERUP:
			event.type = TGUI_MOUSE_UP;
			event.mouse.button = SDL_BUTTON_LEFT;
			event.mouse.x = (float)sdl_event->tfinger.x;
			event.mouse.y = (float)sdl_event->tfinger.y;
			event.mouse.normalised = true;
			event.mouse.is_touch = true;
			event.mouse.finger = sdl_event->tfinger.fingerId;
			event.mouse.is_repeat = false;
			break;
		case SDL_FINGERMOTION:
			event.type = TGUI_MOUSE_AXIS;
			event.mouse.button = -1;
			event.mouse.x = (float)sdl_event->tfinger.x;
			event.mouse.y = (float)sdl_event->tfinger.y;
			event.mouse.normalised = true;
			event.mouse.is_touch = true;
			event.mouse.finger = sdl_event->tfinger.fingerId;
			event.mouse.is_repeat = false;
			break;
#endif
		default:
			event.type = TGUI_UNKNOWN;
			break;
	}

	return event;
}
