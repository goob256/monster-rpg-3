#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/onscreen_controller.h"

using namespace wedge;

namespace wedge {

static bool use_onscreen_controller;
static bool b2_enabled;
static int b3_key;
static int b4_key;
static bool generate_repeats;

struct Finger {
	SDL_FingerID id;
	Onscreen_Button button;
	bool down;
	Uint32 down_time;
	Uint32 ignore_time;
};

std::list<Finger *> fingers;

static bool on_button(Onscreen_Button button, float x, float y)
{
	util::Size<int> sprite_size;

	if (button == ONSCREEN_B1 || button == ONSCREEN_B2 || button == ONSCREEN_B3 || button == ONSCREEN_B4) {
		globals->dpad->set_animation("button1");
		sprite_size = globals->dpad->get_current_image()->size;
	}
	else {
		globals->dpad->set_animation("dpad");
		sprite_size = globals->dpad->get_current_image()->size / 3;
	}

	util::Point<float> button_pos = GLOBALS->get_onscreen_button_position(button);
	util::Point<float> bottom_right = button_pos + sprite_size-1;

	return cd::box_box(button_pos, bottom_right, util::Point<int>(x, y), util::Point<int>(x+1, y+1));
}

static std::list<Finger *>::iterator find_finger(SDL_Event *event)
{
	for (std::list<Finger *>::iterator it = fingers.begin(); it != fingers.end(); it++) {
		Finger *f = *it;
		if (event->tfinger.fingerId == f->id) {
			return it;
		}
	}
	return fingers.end();
}

static Onscreen_Button which_button(float x, float y)
{
	if (on_button(ONSCREEN_UP, x, y)) {
		return ONSCREEN_UP;
	}
	else if (on_button(ONSCREEN_RIGHT, x, y)) {
		return ONSCREEN_RIGHT;
	}
	else if (on_button(ONSCREEN_DOWN, x, y)) {
		return ONSCREEN_DOWN;
	}
	else if (on_button(ONSCREEN_LEFT, x, y)) {
		return ONSCREEN_LEFT;
	}
	else if (on_button(ONSCREEN_B1, x, y)) {
		return ONSCREEN_B1;
	}
	else if (on_button(ONSCREEN_B2, x, y)) {
		return ONSCREEN_B2;
	}
	else if (on_button(ONSCREEN_B3, x, y)) {
		return ONSCREEN_B3;
	}
	else if (on_button(ONSCREEN_B4, x, y)) {
		return ONSCREEN_B4;
	}
	else {
		return ONSCREEN_NONE;
	}
}

static void activate_button(Onscreen_Button button, bool down, bool is_repeat)
{
	TGUI_Event e;

	TGUI_Event_Type type = down ? TGUI_KEY_DOWN : TGUI_KEY_UP;

	switch (button) {
		case ONSCREEN_UP:
			e.type = type;
			e.keyboard.is_repeat = is_repeat;
			e.keyboard.code = globals->key_u;
			break;
		case ONSCREEN_RIGHT:
			e.type = type;
			e.keyboard.is_repeat = is_repeat;
			e.keyboard.code = globals->key_r;
			break;
		case ONSCREEN_DOWN:
			e.type = type;
			e.keyboard.is_repeat = is_repeat;
			e.keyboard.code = globals->key_d;
			break;
		case ONSCREEN_LEFT:
			e.type = type;
			e.keyboard.is_repeat = is_repeat;
			e.keyboard.code = globals->key_l;
			break;
		case ONSCREEN_B1:
			e.type = type;
			e.keyboard.is_repeat = is_repeat;
			e.keyboard.code = globals->key_b1;
			break;
		case ONSCREEN_B2:
			if (b2_enabled) {
				e.type = type;
				e.keyboard.is_repeat = is_repeat;
				e.keyboard.code = globals->key_b2;
			}
			else {
				return;
			}
			break;
		case ONSCREEN_B3:
			if (b3_key >= 0) {
				e.type = type;
				e.keyboard.is_repeat = is_repeat;
				e.keyboard.code = b3_key;
			}
			else {
				return;
			}
			break;
		case ONSCREEN_B4:
			if (b4_key >= 0) {
				e.type = type;
				e.keyboard.is_repeat = is_repeat;
				e.keyboard.code = b4_key;
			}
			else {
				return;
			}
			break;
		default:
			return;
	}

	shim::push_event(e);
}

static Finger *add_finger(SDL_Event *event)
{
	Onscreen_Button button = which_button(event->tfinger.x, event->tfinger.y);

	bool exists = false;

	// Ignore fingers that land on buttons with fingers on them already
	for (std::list<Finger *>::iterator it = fingers.begin(); it != fingers.end(); it++) {
		Finger *existing = *it;
		if (existing->button == button && button != ONSCREEN_NONE && button != ONSCREEN_IGNORE) {
			exists = true;
			break;
		}
	}

	Finger *f = new Finger;
	f->id = event->tfinger.fingerId;
	if (exists) {
		f->button = ONSCREEN_IGNORE;
	}
	else {
		f->button = button;
	}
	f->down = true;
	f->down_time = GET_TICKS();
	f->ignore_time = GET_TICKS();
	fingers.push_back(f);
	return f;
}

static bool check_repeats(Finger *f)
{
	if (f->button == ONSCREEN_NONE || f->button == ONSCREEN_IGNORE) {
		return false;
	}

	// these define the repeat rate in thousands of a second
	int initial_delay = 500;
	int repeat_delay = 50;
	Uint32 diff = GET_TICKS() - f->down_time;
	if ((int)diff > initial_delay) {
		diff -= initial_delay;
		int elapsed = diff;
		int mod = elapsed % repeat_delay;
		if (mod < repeat_delay / 2) {
			// down
			if (f->down == false) {
				f->down = true;
				return true;
			}
		}
		else {
			// up
			f->down = false;
		}
	}

	return false;
}

bool handle_onscreen_controller(SDL_Event *event)
{
	if (event->type != SDL_FINGERDOWN && event->type != SDL_FINGERUP && event->type != SDL_FINGERMOTION) {
		return false;
	}

	event->tfinger.x *= shim::real_screen_size.w;
	event->tfinger.y *= shim::real_screen_size.h;
	event->tfinger.x -= shim::screen_offset.x;
	event->tfinger.y -= shim::screen_offset.y;
	event->tfinger.x /= shim::scale;
	event->tfinger.y /= shim::scale;

	if (shim::dpad_below) {
		if (event->tfinger.y >= shim::screen_size.h) {
			event->tfinger.y -= shim::screen_size.h/2;
		}
		else if (event->tfinger.y >= shim::screen_size.h/2) {
			return true;
		}
	}

	std::list<Finger *>::iterator it = find_finger(event);
	Finger *f = NULL;
	if (it != fingers.end()) {
		f = *it;
	}

	switch (event->type) {
		case SDL_FINGERDOWN: {
			f = add_finger(event);
			activate_button(f->button, true, false);
			break;
		}
		case SDL_FINGERUP: {
			if (f) {
				activate_button(f->button, false, false);
				delete f;
				fingers.erase(it);
			}
			break;
		}
		case SDL_FINGERMOTION: {
			if (f) {
				Onscreen_Button button = which_button(event->tfinger.x, event->tfinger.y);
				if (button != f->button) {
					bool exists = false;
					// Ignore fingers that land on buttons with fingers on them already
					for (std::list<Finger *>::iterator it = fingers.begin(); it != fingers.end(); it++) {
						Finger *existing = *it;
						if (existing != f && existing->button == button) {
							exists = true;
							break;
						}
					}
					activate_button(f->button, false, false);
					if (exists) {
						f->button = ONSCREEN_IGNORE;
					}
					else if (f->button != ONSCREEN_IGNORE) {
						activate_button(button, true, false);
						f->button = button;
					}
					f->down = true;
					f->down_time = GET_TICKS();
					f->ignore_time = GET_TICKS();
				}
			}
			break;
		}
		default:
			return false;
	}

	return true;
}

void update_onscreen_controller()
{
	std::list<Finger *>::iterator it;
	for (it = fingers.begin(); it != fingers.end();) {
		Finger *f = *it;
		if (generate_repeats) {
			if (check_repeats(f)) {
				activate_button(f->button, true, true);
				activate_button(f->button, false, true);
			}
		}
		if (f->button == ONSCREEN_IGNORE && GET_TICKS()-f->ignore_time > 5*60*1000) {
			// If it's an ignored finger, erase it after 5 minutes
			it = fingers.erase(it);
		}
		else {
			it++;
		}
	}
}

static void draw_dpad(Onscreen_Button dpad)
{
	int frame;

	switch (dpad) {
		case ONSCREEN_UP:
			frame = 1;
			break;
		case ONSCREEN_RIGHT:
			frame = 2;
			break;
		case ONSCREEN_DOWN:
			frame = 3;
			break;
		case ONSCREEN_LEFT:
			frame = 4;
			break;
		default:
			frame = 0;
			break;
	}

	globals->dpad->set_animation("dpad");
	gfx::Image *image = globals->dpad->get_image(frame);

	if (image) {
		util::Point<int> left = GLOBALS->get_onscreen_button_position(ONSCREEN_LEFT);
		util::Point<int> up = GLOBALS->get_onscreen_button_position(ONSCREEN_UP);
		util::Point<int> pos(left.x, up.y);
		if (shim::dpad_below) {
			pos.y += shim::screen_size.h/2;
		}
		SDL_Colour colour = shim::white;
		if (shim::dpad_below == false) {
			colour.r *= globals->onscreen_controls_alpha;
			colour.g *= globals->onscreen_controls_alpha;
			colour.b *= globals->onscreen_controls_alpha;
			colour.a *= globals->onscreen_controls_alpha;
		}
		image->draw_tinted(colour, pos);
	}
}

static void draw_button(Onscreen_Button button, bool down)
{
	switch (button) {
		case ONSCREEN_B4:
			globals->dpad->set_animation("button4");
			break;
		case ONSCREEN_B3:
			globals->dpad->set_animation("button3");
			break;
		case ONSCREEN_B2:
			globals->dpad->set_animation("button2");
			break;
		default:
			globals->dpad->set_animation("button1");
			break;
	}

	gfx::Image *image = globals->dpad->get_image(down ? 1 : 0);

	util::Point<float> pos = GLOBALS->get_onscreen_button_position(button);

	SDL_Colour colour = shim::white;
	if (shim::dpad_below == false) {
		colour.r *= globals->onscreen_controls_alpha;
		colour.g *= globals->onscreen_controls_alpha;
		colour.b *= globals->onscreen_controls_alpha;
		colour.a *= globals->onscreen_controls_alpha;
	}
	if (shim::dpad_below) {
		pos.y += shim::screen_size.h/2;
	}
	image->draw_tinted(colour, pos);
}

void draw_onscreen_controller()
{
	Onscreen_Button dpad = ONSCREEN_NONE;
	bool b1_down = false;
	bool b2_down = false;
	bool b3_down = false;
	bool b4_down = false;

	for (std::list<Finger *>::iterator it = fingers.begin(); it != fingers.end(); it++) {
		Finger *f = *it;
		if (f->button == ONSCREEN_NONE || f->button == ONSCREEN_IGNORE) {
			continue;
		}
		else if (f->button == ONSCREEN_B1) {
			b1_down = true;
		}
		else if (f->button == ONSCREEN_B2) {
			b2_down = true;
		}
		else if (f->button == ONSCREEN_B3) {
			b3_down = true;
		}
		else if (f->button == ONSCREEN_B4) {
			b4_down = true;
		}
		else {
			dpad = f->button;
		}
	}

	if (shim::dpad_below) {
		if (shim::opengl) {
			glScissor_ptr(0, 0, shim::real_screen_size.w, shim::real_screen_size.h);
		}
#ifdef _WIN32
		else {
			RECT scissor = { 0, 0, shim::real_screen_size.w, shim::real_screen_size.h };
			shim::d3d_device->SetScissorRect(&scissor);
		}
#endif
	}

	draw_dpad(dpad);
	draw_button(ONSCREEN_B1, b1_down);
	if (b2_enabled) {
		draw_button(ONSCREEN_B2, b2_down);
	}
	if (b3_key >= 0) {
		draw_button(ONSCREEN_B3, b3_down);
	}
	if (b4_key >= 0) {
		draw_button(ONSCREEN_B4, b4_down);
	}
	
	if (shim::dpad_below) {
		gfx::unset_scissor();
	}
}

void enable_onscreen_controller(bool enable)
{
	bool changed = enable != use_onscreen_controller;
	use_onscreen_controller = enable;
	if (enable == false) {
		for (std::list<Finger *>::iterator it = fingers.begin(); it != fingers.end();) {
			Finger *f = *it;
			activate_button(f->button, false, false);
			delete f;
			it = fingers.erase(it);
		}
	}
	if (enable == true && changed) {
		// If you're battling and you press your finger to close the last dialogue it'll not generate an up event
		// until the onscreen controller is re-enabled. When the onscreen controller is enabled, shim::handle_event
		// isn't called so it will generate continuous mouse down (repeat) events, this stops that.
		input::drop_repeats();
	}

	shim::dpad_enabled = use_onscreen_controller;
}

bool is_onscreen_controller_enabled()
{
	return use_onscreen_controller;
}

void set_onscreen_controller_b2_enabled(bool enabled)
{
	b2_enabled = enabled;
}

void set_onscreen_controller_b3_enabled(bool enabled, int key)
{
	if (enabled) {
		b3_key = key;
	}
	else {
		b3_key = -1;
	}
}

void set_onscreen_controller_b4_enabled(bool enabled, int key)
{
	if (enabled) {
		b4_key = key;
	}
	else {
		b4_key = -1;
	}
}

bool get_onscreen_controller_b3_enabled()
{
	return b3_key >= 0;
}

bool get_onscreen_controller_b4_enabled()
{
	return b4_key >= 0;
}

void start_onscreen_controller(bool generate_repeats)
{
	use_onscreen_controller = false;
	b2_enabled = true;
	b3_key = -1;
	b4_key = -1;
	fingers.clear();
	::generate_repeats = generate_repeats;
}

}
