#include "Nooskewl_Shim/gfx.h"
#include "Nooskewl_Shim/input.h"
#include "Nooskewl_Shim/shim.h"
#include "Nooskewl_Shim/util.h"

#ifdef __APPLE__
#define USE_CONSTANT_RUMBLE 1
#else
#define USE_CONSTANT_RUMBLE 0
#endif

#ifdef ANDROID
#include <jni.h>
#endif

#define REPEAT_VEC std::vector<Joy_Repeat>

#ifdef ANDROID
const Uint32 JOYSTICK_SUBSYSTEMS = SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC;
#endif

#ifdef IOS
#import <AudioToolbox/AudioToolbox.h>
#endif

using namespace noo;

#include "Nooskewl_Shim/internal/gfx.h"
#include "Nooskewl_Shim/internal/shim.h"

struct Joy_Repeat {
	bool is_button;
	bool is_hat;
	int button;
	int axis;
	int hat_x;
	int hat_y;
	int value;
	Uint32 initial_press_time;
	bool down;
	bool repeated;
};

struct Mouse_Button_Repeat {
	bool is_touch;
	int button;
	SDL_FingerID finger;
	util::Point<float> down_pos;
	Uint32 initial_press_time;
	bool down;
};

struct Joystick {
	SDL_JoystickID id;
	SDL_Joystick *joy;
	SDL_Haptic *haptic;
#if USE_CONSTANT_RUMBLE
	SDL_HapticEffect haptic_effect;
	int haptic_effect_id;
#endif
	SDL_GameController *gc;
	REPEAT_VEC joystick_repeats;
};

static std::vector<Joystick> joysticks;
static int num_joysticks;
static std::vector<Mouse_Button_Repeat> mouse_button_repeats;

static Joystick *find_joystick(SDL_JoystickID id)
{
	for (size_t i = 0; i < joysticks.size(); i++) {
		if (joysticks[i].id == id) {
			return &joysticks[i];
		}
	}

	return 0;
}

static int find_joy_repeat(bool is_button, bool is_hat, int n, Joystick *js)
{
	REPEAT_VEC &joystick_repeats = js->joystick_repeats;

	for (size_t i = 0; i < joystick_repeats.size(); i++) {
		if (is_button && joystick_repeats[i].is_button && joystick_repeats[i].button == n) {
			return (int)i;
		}
		if (!is_button && !is_hat && !joystick_repeats[i].is_button && !joystick_repeats[i].is_hat && joystick_repeats[i].axis == n) {
			return (int)i;
		}
		if (is_hat && joystick_repeats[i].is_hat) {
			return (int)i;
		}
	}

	return -1;
}

static int find_mouse_button_repeat(bool is_touch, int button, SDL_FingerID finger)
{
	for (size_t i = 0; i < mouse_button_repeats.size(); i++) {
		if (mouse_button_repeats[i].is_touch == is_touch) {
			if (is_touch && mouse_button_repeats[i].finger == finger) {
				return (int)i;
			}
			else if (is_touch == false && mouse_button_repeats[i].button == button) {
				return (int)i;
			}
		}
	}

	return -1;
}

static bool check_joystick_repeat(Joy_Repeat &jr)
{
	// these define the repeat rate in thousands of a second
	int initial_delay = 500;
	int repeat_delay = 50;
	Uint32 diff = SDL_GetTicks() - jr.initial_press_time;
	if ((int)diff > initial_delay) {
		diff -= initial_delay;
		int elapsed = diff;
		int mod = elapsed % repeat_delay;
		if (mod < repeat_delay / 2) {
			// down
			if (jr.down == false) {
				jr.down = true;
				return true;
			}
		}
		else {
			// up
			jr.down = false;
		}
	}

	return false;
}

static bool check_mouse_button_repeat(Mouse_Button_Repeat &mr)
{
	// these define the repeat rate in thousands of a second
	int initial_delay = 500;
	int repeat_delay = 50;
	Uint32 diff = SDL_GetTicks() - mr.initial_press_time;
	if ((int)diff > initial_delay) {
		diff -= initial_delay;
		int elapsed = diff;
		int mod = elapsed % repeat_delay;
		if (mod < repeat_delay / 2) {
			// down
			if (mr.down == false) {
				mr.down = true;
				return true;
			}
		}
		else {
			// up
			mr.down = false;
		}
	}

	return false;
}

static void check_joysticks()
{
	int nj = SDL_NumJoysticks();
	int curr_index = 0;
	if (nj != num_joysticks) {
		num_joysticks = nj;
		for (size_t i = 0; i < joysticks.size(); i++) {
			if (joysticks[i].haptic) {
				SDL_HapticClose(joysticks[i].haptic);
			}
			SDL_GameControllerClose(joysticks[i].gc);
		}
		joysticks.clear();
		for (int i = 0; i < num_joysticks; i++) {
			Joystick j;
			j.gc = SDL_GameControllerOpen(i);
			j.joy = SDL_GameControllerGetJoystick(j.gc);
#ifdef TVOS
			// Ignore joystick input from Siri Remote (still get keys)
			if (SDL_JoystickNumButtons(j.joy) <= 3) { // Siri Remote has 3 buttons
#else
			if (SDL_JoystickNumButtons(j.joy) <= 4) { // Fix for Moonlight doubling inputs
#endif
				SDL_JoystickClose(j.joy);
				continue;
			}
			/*
			if (curr_index != shim::joy_index) {
				SDL_JoystickClose(j.joy);
				curr_index++;
				continue;
			}
			*/
			j.id = SDL_JoystickInstanceID(j.joy);
			if (SDL_JoystickIsHaptic(j.joy)) {
				util::infomsg("Joystick has haptics.\n");
				j.haptic = SDL_HapticOpenFromJoystick(j.joy);
				if (j.haptic == 0) {
					util::infomsg("Haptic init failed: %s.\n", SDL_GetError());
				}
				else {
#if USE_CONSTANT_RUMBLE
					memset(&j.haptic_effect, 0, sizeof(j.haptic_effect));
					j.haptic_effect.type = SDL_HAPTIC_CONSTANT;
					j.haptic_effect.constant.level = 0x7fff;
					j.haptic_effect.constant.length = 1000;
					j.haptic_effect_id = SDL_HapticNewEffect(j.haptic, &j.haptic_effect);
					if (j.haptic_effect_id < 0) {
						util::infomsg("Couldn't create constant haptic effect.\n");
					}
#else
					if (SDL_HapticRumbleInit(j.haptic) != 0) {
						util::infomsg("Can't init rumble effect: %s\n", SDL_GetError());
					}
#endif
					if (SDL_HapticSetGain(j.haptic, 100)) {
						util::infomsg("Can't set haptic gain: %s\n", SDL_GetError());
					}
				}
			}
			else {
				util::infomsg("Joystick does not have haptics\n");
				j.haptic = 0;
			}
			joysticks.push_back(j);
			// FIXME: support multiple joysticks
			//break;
		}
		gfx::show_mouse_cursor(joysticks.size() == 0);
	}
}

namespace noo {

namespace input {

bool start()
{
	int index;
	if ((index = util::check_args(shim::argc, shim::argv, "+joystick-activate-threshold") > 0)) {
		shim::joystick_activate_threshold = atof(shim::argv[index+1]);
	}
	else {
		shim::joystick_activate_threshold = 0.8f;
	}
	if ((index = util::check_args(shim::argc, shim::argv, "+joystick-deactivate-threshold") > 0)) {
		shim::joystick_deactivate_threshold = atof(shim::argv[index+1]);
	}
	else {
		shim::joystick_deactivate_threshold = 0.75f;
	}
	if ((index = util::check_args(shim::argc, shim::argv, "+joy-index") > 0)) {
		shim::joy_index = atoi(shim::argv[index+1]);
	}

	joysticks.clear();
	num_joysticks = 0;

#ifdef ANDROID
	SDL_InitSubSystem(JOYSTICK_SUBSYSTEMS);
#endif

 	check_joysticks();

	return true;
}

void reset()
{
	for (size_t i = 0; i < joysticks.size(); i++) {
		Joystick &j = joysticks[i];
		if (j.haptic) {
		       SDL_HapticClose(j.haptic);
		}
		if (j.gc) {
			SDL_GameControllerClose(j.gc);
		}
	}

	joysticks.clear();

	num_joysticks = 0;
}

void end()
{
	reset();
#ifdef ANDROID
	SDL_QuitSubSystem(JOYSTICK_SUBSYSTEMS);
#endif
}

void update()
{
	check_joysticks();

	// joystick repeat
	for (size_t j = 0; j < joysticks.size(); j++) {
		REPEAT_VEC &joystick_repeats = joysticks[j].joystick_repeats;
		for (size_t i = 0; i < joystick_repeats.size(); i++) {
			Joy_Repeat &jr = joystick_repeats[i];
			if (check_joystick_repeat(jr)) {
				jr.repeated = true;
				TGUI_Event event;
				if (jr.is_button) {
					event.type = TGUI_JOY_DOWN;
					event.joystick.is_repeat = true;
					event.joystick.button = jr.button;
					event.joystick.id = joysticks[j].id;
					shim::push_event(event);
				}
				else if (jr.is_hat) {
					event.type = TGUI_JOY_HAT;
					event.joystick.is_repeat = true;
					event.joystick.hat_x = jr.hat_x;
					event.joystick.hat_y = jr.hat_y;
					event.joystick.id = joysticks[j].id;
					shim::push_event(event);
				}
				else {
					event.type = TGUI_JOY_AXIS;
					event.joystick.is_repeat = true;
					event.joystick.axis = jr.axis;
					event.joystick.value = jr.value;
					event.joystick.id = joysticks[j].id;
					shim::push_event(event);
				}
			}
		}
	}

	// mouse button repeat
	if (shim::mouse_button_repeats) {
		for (size_t i = 0; i < mouse_button_repeats.size(); i++) {
			Mouse_Button_Repeat &mr = mouse_button_repeats[i];
			if (check_mouse_button_repeat(mr)) {
				TGUI_Event event;
				event.type = TGUI_MOUSE_DOWN;
				event.mouse.is_touch = mr.is_touch;
				event.mouse.is_repeat = true;
				event.mouse.button = mr.button;
				event.mouse.finger = mr.finger;
				event.mouse.x = mr.down_pos.x * shim::scale + shim::screen_offset.x;
				event.mouse.y = mr.down_pos.y * shim::scale + shim::screen_offset.y;
				event.mouse.normalised = false;
				shim::push_event(event);
			}
		}
	}
}

void handle_event(TGUI_Event *event)
{
	// FIXME: support multiple joysticks
	if (event->type == TGUI_JOY_DOWN || event->type == TGUI_JOY_UP || event->type == TGUI_JOY_AXIS || event->type == TGUI_JOY_HAT) {
		Joystick *js = find_joystick(event->joystick.id);

		if (js == 0) {
			event->type = TGUI_UNKNOWN;
			return;
		}
	}

	// joystick button repeat
	if ((event->type == TGUI_JOY_DOWN || event->type == TGUI_JOY_UP)) {
		Joystick *js = find_joystick(event->joystick.id);

		if (js == 0) {
			return;
		}

		REPEAT_VEC &joystick_repeats = js->joystick_repeats;

		if (event->type == TGUI_JOY_DOWN) {
			if (find_joy_repeat(true, false, event->joystick.button, js) < 0 && event->joystick.is_repeat == false) {
				Joy_Repeat jr;
				jr.is_button = true;
				jr.is_hat = false;
				jr.button = event->joystick.button;
				jr.initial_press_time = SDL_GetTicks();
				jr.down = true;
				joystick_repeats.push_back(jr);
			}
		}
		else {
			int index = find_joy_repeat(true, false, event->joystick.button, js);
			if (index >= 0) {
				joystick_repeats.erase(joystick_repeats.begin() + index);
			}
		}
	}
	else if (event->type == TGUI_MOUSE_DOWN || event->type == TGUI_MOUSE_UP) {
		if (event->type == TGUI_MOUSE_DOWN) {
			if (find_mouse_button_repeat(event->mouse.is_touch, event->mouse.button, event->mouse.finger) < 0 && event->mouse.is_repeat == false) {
				Mouse_Button_Repeat mr;
				mr.is_touch = event->mouse.is_touch;
				mr.button = event->mouse.button;
				mr.finger = event->mouse.finger;
				mr.down_pos = util::Point<float>(event->mouse.x, event->mouse.y);
				mr.initial_press_time = SDL_GetTicks();
				mr.down = true;
				mouse_button_repeats.push_back(mr);
			}
		}
		else {
			int index = find_mouse_button_repeat(event->mouse.is_touch, event->mouse.button, event->mouse.finger);
			if (index >= 0) {
				mouse_button_repeats.erase(mouse_button_repeats.begin() + index);
			}
		}
	}
	else if (event->type == TGUI_MOUSE_AXIS) {
		int index = find_mouse_button_repeat(event->mouse.is_touch, event->mouse.button, event->mouse.finger);
		if (index >= 0) {
			Mouse_Button_Repeat &mr = mouse_button_repeats[index];
			util::Point<float> diff = mr.down_pos - util::Point<float>(event->mouse.x, event->mouse.y);
			if (fabsf(diff.x) > shim::scale*shim::mouse_button_repeat_max_movement || fabsf(diff.y) > shim::scale*shim::mouse_button_repeat_max_movement) {
				mouse_button_repeats.erase(mouse_button_repeats.begin() + index);
			}
		}
	}
}

bool convert_to_focus_event(TGUI_Event *event, Focus_Event *focus)
{
	static int hat_x = 0;
	static int hat_y = 0;
	int x = 0;
	int y = 0;

	if (is_joystick_connected() && event->type == TGUI_JOY_AXIS && (event->joystick.axis == 0 || event->joystick.axis == 1)) {
		Joystick *js = find_joystick(event->joystick.id);

		if (js == 0) {
			return false;
		}
		
		REPEAT_VEC &joystick_repeats = js->joystick_repeats;

		int axis = event->joystick.axis;
		int index = find_joy_repeat(false, false, axis, js);
		Sint16 other_s = SDL_JoystickGetAxis(js->joy, 1-axis);
		float other = TGUI3_NORMALISE_JOY_AXIS(other_s);

		if (fabsf(event->joystick.value) > shim::joystick_activate_threshold && fabsf(other) < shim::joystick_deactivate_threshold) {
			bool go = true;
			if (index < 0) {
				if (event->joystick.is_repeat == false) {
					Joy_Repeat jr;
					jr.is_button = false;
					jr.is_hat = false;
					jr.axis = axis;
					jr.value = event->joystick.value < 0 ? -1 : 1;
					jr.initial_press_time = SDL_GetTicks();
					jr.down = true;
					jr.repeated = false;
					joystick_repeats.push_back(jr);
					go = true;
				}
			}
			else {
				Joy_Repeat &jr = joystick_repeats[index];
				if (jr.repeated) {
					jr.repeated = false;
					go = true;
				}
				else {
					go = false;
				}
			}
			if (go) {
				if (axis == 0) {
					if (event->joystick.value < 0) {
						x = -1;
					}
					else {
						x = 1;
					}
				}
				else if (axis == 1) {
					if (event->joystick.value < 0) {
						y = -1;
					}
					else {
						y = 1;
					}
				}
			}
		}
		else if (index >= 0 && fabsf(event->joystick.value) < shim::joystick_deactivate_threshold) {
			joystick_repeats.erase(joystick_repeats.begin() + index);
		}
	}
	if (event->type == TGUI_KEY_DOWN) {
		if (event->keyboard.code == shim::key_l) {
			x = -1;
		}
		else if (event->keyboard.code == shim::key_r) {
			x = 1;
		}
		else if (event->keyboard.code == shim::key_u) {
			y = -1;
		}
		else if (event->keyboard.code == shim::key_d) {
			y = 1;
		}
	}
	else if (event->type == TGUI_JOY_HAT) {
		Joystick *js = find_joystick(event->joystick.id);

		if (js == 0) {
			return false;
		}
		
		REPEAT_VEC &joystick_repeats = js->joystick_repeats;

		int index = find_joy_repeat(false, true, 0, js);
		if (index < 0) {
			if (event->joystick.hat_x != 0 || event->joystick.hat_y != 0) {
				if (event->joystick.is_repeat == false) {
					Joy_Repeat jr;
					jr.is_button = false;
					jr.is_hat = true;
					jr.hat_x = event->joystick.hat_x;
					jr.hat_y = event->joystick.hat_y;
					jr.initial_press_time = SDL_GetTicks();
					jr.down = true;
					joystick_repeats.push_back(jr);
				}
			}
		}
		else {
			if (event->joystick.hat_x == 0 && event->joystick.hat_y == 0) {
				joystick_repeats.erase(joystick_repeats.begin() + index);
			}
			else {
				if (event->joystick.hat_x != joystick_repeats[index].hat_x || event->joystick.hat_y != joystick_repeats[index].hat_y) {
					joystick_repeats[index].hat_x = event->joystick.hat_x;
					joystick_repeats[index].hat_y = event->joystick.hat_y;
					joystick_repeats[index].initial_press_time = SDL_GetTicks();
				}
			}
		}

		x = event->joystick.hat_x;
		y = event->joystick.hat_y;
		if (x != 0 && y != 0) {
			if (shim::ignore_hat_diagonals) {
				x = y = 0;
			}
			else {
				if (hat_x != 0) {
					hat_x = x = 0;
					hat_y = y;
				}
				else if (hat_y != 0) {
					hat_x = x;
					hat_y = y = 0;
				}
				else {
					x = y = 0;
				}
			}
		}
		else {
			hat_x = event->joystick.hat_x;
			hat_y = event->joystick.hat_y;
		}
	}

	if (x != 0 || y != 0) {
		focus->orig_type = event->type;
		if (event->type == TGUI_KEY_DOWN) {
			focus->u.orig_keyboard.code = event->keyboard.code;
			focus->u.orig_keyboard.is_repeat = event->keyboard.is_repeat;
		}
		else {
			focus->u.orig_joystick.id = event->joystick.id;
			focus->u.orig_joystick.button = event->joystick.button;
			focus->u.orig_joystick.axis = event->joystick.axis;
			focus->u.orig_joystick.hat_x = event->joystick.hat_x;
			focus->u.orig_joystick.hat_y = event->joystick.hat_y;
			focus->u.orig_joystick.value = event->joystick.value;
			focus->u.orig_joystick.is_repeat = event->joystick.is_repeat;
		}
		focus->type = TGUI_FOCUS;
		if  (x < 0) {
			focus->focus.type = TGUI_FOCUS_LEFT;
		}
		else if (x > 0) {
			focus->focus.type = TGUI_FOCUS_RIGHT;
		}
		else if (y < 0) {
			focus->focus.type = TGUI_FOCUS_UP;
		}
		else {
			focus->focus.type = TGUI_FOCUS_DOWN;
		}
		return true;
	}
	else {
		return false;
	}
}

void convert_focus_to_original(TGUI_Event *event)
{
	if (event->type == TGUI_FOCUS) {
		// grab the original...
		input::Focus_Event *focus = dynamic_cast<input::Focus_Event *>(event);
		if (focus) {
			event->type = focus->orig_type;
			if (focus->orig_type == TGUI_KEY_DOWN) {
				event->keyboard.code = focus->u.orig_keyboard.code;
				event->keyboard.is_repeat = focus->u.orig_keyboard.is_repeat;
			}
			else {
				event->joystick.id = focus->u.orig_joystick.id;
				event->joystick.button = focus->u.orig_joystick.button;
				event->joystick.axis = focus->u.orig_joystick.axis;
				event->joystick.hat_x = focus->u.orig_joystick.hat_x;
				event->joystick.hat_y = focus->u.orig_joystick.hat_y;
				event->joystick.value = focus->u.orig_joystick.value;
				event->joystick.is_repeat = focus->u.orig_joystick.is_repeat;
			}
		}
	}
}

// FIXME: this rumbles ALL joysticks
void rumble(float strength, Uint32 length)
{
#ifdef ANDROID
	if (is_joystick_connected() == false) {
		JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
		jobject activity = (jobject)SDL_AndroidGetActivity();
		jclass clazz(env->GetObjectClass(activity));

		jmethodID method_id = env->GetMethodID(clazz, "rumble", "(I)V");

		if (method_id != 0) {
			env->CallVoidMethod(activity, method_id, length);
		}

		env->DeleteLocalRef(activity);
		env->DeleteLocalRef(clazz);
	}
	else
#elif defined IOS && !defined TVOS
	if (length >= 1000) {
		AudioServicesPlaySystemSound(/*0x00000FFF*/kSystemSoundID_Vibrate);
	}
	else
#endif
	{
		for (size_t i = 0; i < joysticks.size(); i++) {
			Joystick &j = joysticks[i];

			//SDL_GameControllerRumble(j.gc, strength*0xffff, strength*0xffff, length);
			if (j.gc) {
				SDL_GameControllerRumble(j.gc, 0x7777, 0x7777, length);
			}
		}
	}
}

bool is_joystick_connected()
{
	return joysticks.size() != 0;
}

std::string joystick_button_to_name(int button)
{
	switch (button) {
		case 0:
			return "A";
		case 1:
			return "B";
		case 2:
			return "X";
		case 3:
			return "Y";
#ifdef ANDROID
		case 9:
			return "LB";
		case 10:
			return "RB";
#else
		case 4:
			return "LB";
		case 5:
			return "RB";
#endif
		default:
			return std::string("BUTTON ") + util::itos(button);
	}
}

void drop_repeats(bool joystick, bool mouse)
{
	if (joystick) {
		for (size_t i = 0; i < joysticks.size(); i++) {
			joysticks[i].joystick_repeats.clear();
		}
	}
	if (mouse) {
		mouse_button_repeats.clear();
	}
}

Focus_Event::~Focus_Event()
{
}

} // End namespace input

} // End namespace noo
