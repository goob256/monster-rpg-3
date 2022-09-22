#include "Nooskewl_Shim/audio.h"
#include "Nooskewl_Shim/cpa.h"
#include "Nooskewl_Shim/error.h"
#include "Nooskewl_Shim/gfx.h"
#include "Nooskewl_Shim/gui.h"
#include "Nooskewl_Shim/image.h"
#include "Nooskewl_Shim/input.h"
#include "Nooskewl_Shim/mml.h"
#include "Nooskewl_Shim/mt.h"
#include "Nooskewl_Shim/primitives.h"
#include "Nooskewl_Shim/shim.h"
#include "Nooskewl_Shim/sprite.h"
#include "Nooskewl_Shim/translation.h"
#include "Nooskewl_Shim/util.h"
#include "Nooskewl_Shim/vertex_cache.h"

#include "Nooskewl_Shim/internal/audio.h"
#include "Nooskewl_Shim/internal/gfx.h"
#include "Nooskewl_Shim/internal/shim.h"
#include "Nooskewl_Shim/internal/util.h"

#ifdef STEAMWORKS
#include "Nooskewl_Shim/steamworks.h"
#endif

using namespace noo;

#if defined IOS || defined ANDROID
static bool app_in_background;
#endif

#if defined IOS
static bool adjust_screen_size;
#endif

namespace noo {

namespace shim {

static TGUI_Event *tgui_event;
static input::Focus_Event *focus_event;
static std::vector<TGUI_Event> pushed_events;
static bool waiting_for_fullscreen_change;
static bool waiting_for_resize;
static bool quitting;
static util::Size<int> switch_out_screen_size;

bool opengl;
SDL_Colour palette[256];
SDL_Colour black;
SDL_Colour white;
SDL_Colour magenta;
SDL_Colour transparent;
std::vector<gui::GUI *> guis;
float scale;
std::string window_title;
std::string organization_name;
std::string game_name;
gfx::Shader *current_shader;
gfx::Shader *default_shader;
gfx::Shader *model_shader;
int tile_size;
util::Size<int> screen_size;
util::Size<int> real_screen_size;
gfx::Font *font;
bool create_depth_buffer;
bool create_stencil_buffer;
util::Size<int> depth_buffer_size;
util::Point<int> screen_offset;
float black_bar_percent;
float z_add;
void (*user_render)();
int refresh_rate;
bool hide_window;
int adapter;
bool use_hires_font;
#ifdef _WIN32
IDirect3DDevice9 *d3d_device;
#endif
audio::MML *music;
audio::MML *widget_mml;
float music_volume;
float sfx_volume;
int samplerate;
bool convert_xbox_dpad_to_arrows;
int xbox_l;
int xbox_r;
int xbox_u;
int xbox_d;
int key_l;
int key_r;
int key_u;
int key_d;
int fullscreen_key;
bool convert_directions_to_focus_events;
bool ignore_hat_diagonals;
float joystick_activate_threshold;
float joystick_deactivate_threshold;
bool mouse_button_repeats;
int mouse_button_repeat_max_movement;
bool dpad_below;
bool allow_dpad_below;
bool dpad_enabled;
int joy_index;
util::CPA *cpa;
int cpa_extra_bytes_after_exe_data;
Uint8 *cpa_pointer_to_data;
int cpa_data_size;
int notification_duration;
int notification_fade_duration;
Uint32 timer_event_id;
int argc;
char **argv;
bool logging;
int logic_rate;
bool use_cwd;
bool log_tags;
int error_level;
#ifdef TVOS
bool pass_menu_to_os;
#endif

static void handle_resize(SDL_Event *event)
{
	// Right after recreating the window on Windows we can get window events from the old window
	if (event->window.windowID != gfx::internal::gfx_context.windowid) {
		return;
	}

#if !defined IOS && !defined ANDROID
	if (gfx::internal::gfx_context.fullscreen == false) {
#endif
		gfx::resize_window(event->window.data1, event->window.data2);
#if !defined IOS && !defined ANDROID
	}
#endif
}

// this may run in a different thread :/
static int event_filter(void *userdata, SDL_Event *event)
{
	switch (event->type)
	{
#ifdef IOS
		case SDL_APP_TERMINATING:
			event->type = SDL_QUIT;
			return 1;
		case SDL_APP_LOWMEMORY:
			return 0;
		case SDL_APP_WILLENTERBACKGROUND:
			return 0;
		case SDL_APP_DIDENTERBACKGROUND:
			app_in_background = true;
			return 0;
		case SDL_APP_WILLENTERFOREGROUND:
			adjust_screen_size = true;
			return 0;
		case SDL_APP_DIDENTERFOREGROUND:
			app_in_background = false;
			SDL_SetHint(SDL_HINT_APPLE_TV_CONTROLLER_UI_EVENTS, "0");
			return 0;
#ifdef TVOS
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (event->key.keysym.sym == SDLK_MENU && pass_menu_to_os) {
				SDL_SetHint(SDL_HINT_APPLE_TV_CONTROLLER_UI_EVENTS, "1");
			}
			return 1;
#endif
#elif defined ANDROID
		case SDL_APP_WILLENTERBACKGROUND:
			util::internal::flush_log_file();
			return 0;
		case SDL_APP_WILLENTERFOREGROUND:
			return 0;
		case SDL_APP_DIDENTERBACKGROUND:
			app_in_background = true;
			return 0;
		case SDL_APP_DIDENTERFOREGROUND:
			app_in_background = false;
			return 0;
#elif !defined __linux__ // works fine as-is on Linux
		case SDL_WINDOWEVENT: {
				if (gfx::internal::gfx_context.fullscreen == false) {
					if (event->window.event == SDL_WINDOWEVENT_RESIZED && gfx::internal::gfx_context.inited == true && gfx::internal::gfx_context.restarting == false && quitting == false) {
						SDL_LockMutex(gfx::internal::gfx_context.draw_mutex);
						if (gfx::get_target_image() == 0) {
							if (user_render) {
								handle_resize(event);
								user_render();
							}
							else {
								waiting_for_resize = true;
								gfx::clear(shim::black);
								gfx::flip();
							}
						}
						SDL_UnlockMutex(gfx::internal::gfx_context.draw_mutex);
					}
				}
			}
			return 1;
#endif
		default:
			return 1;
	}
}

static bool init_sdl(int sdl_init_flags)
{
	if (SDL_Init(sdl_init_flags) != 0) {
		throw util::Error(util::string_printf("SDL_Init failed: %s.", SDL_GetError()));
		return false;
	}

	SDL_SetEventFilter(event_filter, NULL);

	return true;
}

static void load_mml()
{
	try {
		widget_mml = new audio::MML("sfx/widget.mml");
		widget_mml->set_pause_with_sfx(false);
	}
	catch (util::Error &e) {
		util::infomsg(e.error_message + "\n");
	}
}

static void destroy_mml()
{
	delete widget_mml;
}

bool static_start(int sdl_init_flags)
{
	if (sdl_init_flags == 0) {
#ifdef ANDROID
		sdl_init_flags = SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO; // we need to be able to shutdown/bring up the joystick system on Android
#else
		sdl_init_flags = SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC;
#endif
	}


#if defined IOS || defined ANDROID
	app_in_background = false;
#endif

#if defined IOS
	adjust_screen_size = false;
#endif

	argc = 0;
	argv = 0;
	tgui_event = 0;
	focus_event = 0;
	pushed_events.clear();
	waiting_for_fullscreen_change = false;
	waiting_for_resize = false;
	opengl = false;
	black.r = 0;
	black.g = 0;
	black.b = 0;
	black.a = 255;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 255;
	magenta.r = 255;
	magenta.g = 0;
	magenta.b = 255;
	magenta.a = 255;
	transparent.r = 0;
	transparent.g = 0;
	transparent.b = 0;
	transparent.a = 0;
	guis.clear();
	window_title = "";
	organization_name = "";
	game_name = "";
	create_depth_buffer = false;
	create_stencil_buffer = false;
	depth_buffer_size = util::Size<int>(-1, -1);
	black_bar_percent = 0.0f;
	z_add = 0.0f;
	user_render = 0;
	refresh_rate = 0;
	hide_window = false;
	use_hires_font = false;
	music = 0;
	widget_mml = 0;
	music_volume = 1.0f;
	sfx_volume = 1.0f;
	samplerate = 0;
	convert_xbox_dpad_to_arrows = false;
	xbox_l = 11;
	xbox_r = 12;
	xbox_u = 13;
	xbox_d = 14;
	key_l = TGUIK_LEFT;
	key_r = TGUIK_RIGHT;
	key_u = TGUIK_UP;
	key_d = TGUIK_DOWN;
	fullscreen_key = 0;
	convert_directions_to_focus_events = false;
	ignore_hat_diagonals = false;
	mouse_button_repeats = true;
	mouse_button_repeat_max_movement = 1; // * shim::scale
	dpad_below = false;
	allow_dpad_below = false;
	joy_index = 0;
	cpa = 0;
	cpa_extra_bytes_after_exe_data = 0;
	cpa_pointer_to_data = 0;
	cpa_data_size = 0;
	notification_duration = 3000;
	notification_fade_duration = 500;
	timer_event_id = (Uint32)-1;
	logging = false;
	logic_rate = 60;
	use_cwd = false;
	log_tags = true;
#ifdef DEBUG
	error_level = 9999;
#elif defined IOS
	error_level = 3; // let debugmsg hit Xcode console
#else
	error_level = 1;
#endif
#ifdef TVOS
	pass_menu_to_os = false;
#endif
	quitting = false;
	font = 0;
	adapter = 0;
	
	switch_out_screen_size = util::Size<int>(-1, -1);

	if (init_sdl(sdl_init_flags) == false) {
		return false;
	}

#ifdef STEAMWORKS
	util::start_steamworks();
#endif

	return true;
}

bool static_start_all(int sdl_init_flags)
{
	audio::static_start();
	gfx::static_start();
	return static_start(sdl_init_flags);
}

bool start()
{
	if (timer_event_id == (Uint32)-1) {
		timer_event_id = SDL_RegisterEvents(1);
	}

#if !defined ANDROID
	try {
		if (cpa_pointer_to_data != 0) {
			cpa = new util::CPA(cpa_pointer_to_data, cpa_data_size);
		}
		else if (argv != 0) {
			cpa = new util::CPA(argv[0]);
		}
		else {
			throw util::Error("Catch me!");
		}
	}
	catch (util::Error &e) {
#endif
		try {
			cpa = new util::CPA();
		}
		catch (util::Error &e) {
			util::infomsg(e.error_message + "\n");
		}
#if !defined ANDROID
	}
#endif

	// Load resources

	load_mml();

	tgui_event = new TGUI_Event;
	focus_event = new input::Focus_Event;

	return true;
}

bool start_all(int scaled_gfx_w, int scaled_gfx_h, bool force_integer_scaling, int gfx_window_w, int gfx_window_h)
{
	util::start();

	if (audio::start() == false) {
		//delete cpa;
		//return false;
		// NOTE: mute was set in audio::start... continue on muted
	}
	
	start();

	if (gfx::start(scaled_gfx_w, scaled_gfx_h, force_integer_scaling, gfx_window_w, gfx_window_h) == false) {
		audio::end();
		delete cpa;
		throw util::Error(util::string_printf("gfx::start failed."));
		return false;
	}

	if (input::start() == false) {
		gfx::end();
		audio::end();
		delete cpa;
		throw util::Error(util::string_printf("input::start failed."));
		return false;
	}

	return true;
}

void static_end()
{
	gfx::static_end();
	SDL_Quit();
}

void end()
{
	for (size_t i = 0; i < guis.size(); i++) {
		delete guis[i];
	}

	destroy_mml();

	delete cpa;

	delete tgui_event;
	delete focus_event;

	util::infomsg("%d unfreed images.\n", gfx::Image::get_unfreed_count());
}

void end_all()
{
	input::end();

	gfx::end();

	end();

	audio::end();

	util::end();
}

static TGUI_Event *real_handle_tgui_event(TGUI_Event *tgui_event)
{
	if (tgui_event->type == TGUI_UNKNOWN) {
		return tgui_event;
	}

	// change Xbox 360 dpad buttons to keys
#if defined __APPLE__ || defined ANDROID
	const int xb_l = 13;
	const int xb_r = 14;
	const int xb_u = 11;
	const int xb_d = 12;
#else
	const int xb_l = 11;
	const int xb_r = 12;
	const int xb_u = 13;
	const int xb_d = 14;
#endif
	if (convert_xbox_dpad_to_arrows) {
		if (tgui_event->type == TGUI_JOY_DOWN) {
			if (tgui_event->joystick.button == xb_l) {
				tgui_event->type = TGUI_JOY_AXIS;
				tgui_event->joystick.axis = 0;
				tgui_event->joystick.value = -1.0f;
			}
			else if (tgui_event->joystick.button == xb_r) {
				tgui_event->type = TGUI_JOY_AXIS;
				tgui_event->joystick.axis = 0;
				tgui_event->joystick.value = 1.0f;
			}
			else if (tgui_event->joystick.button == xb_u) {
				tgui_event->type = TGUI_JOY_AXIS;
				tgui_event->joystick.axis = 1;
				tgui_event->joystick.value = -1.0f;
			}
			else if (tgui_event->joystick.button == xb_d) {
				tgui_event->type = TGUI_JOY_AXIS;
				tgui_event->joystick.axis = 1;
				tgui_event->joystick.value = 1.0f;
			}
		}
		else if (tgui_event->type == TGUI_JOY_UP) {
			if (tgui_event->joystick.button == xb_l) {
				tgui_event->type = TGUI_JOY_AXIS;
				tgui_event->joystick.axis = 0;
				tgui_event->joystick.value = 0.0f;
			}
			else if (tgui_event->joystick.button == xb_r) {
				tgui_event->type = TGUI_JOY_AXIS;
				tgui_event->joystick.axis = 0;
				tgui_event->joystick.value = 0.0f;
			}
			else if (tgui_event->joystick.button == xb_u) {
				tgui_event->type = TGUI_JOY_AXIS;
				tgui_event->joystick.axis = 1;
				tgui_event->joystick.value = 0.0f;
			}
			else if (tgui_event->joystick.button == xb_d) {
				tgui_event->type = TGUI_JOY_AXIS;
				tgui_event->joystick.axis = 1;
				tgui_event->joystick.value = 0.0f;
			}
		}
	}

	bool is_focus;

	if (convert_directions_to_focus_events) {
		is_focus = input::convert_to_focus_event(tgui_event, focus_event);
	}
	else {
		is_focus = false;
	}

	TGUI_Event *event;
	if (is_focus) {
		event = focus_event;
	}
	else {
		event = tgui_event;
	}

	if (event->type == TGUI_MOUSE_AXIS || event->type == TGUI_MOUSE_DOWN || event->type == TGUI_MOUSE_UP) {
		gfx::set_custom_mouse_cursor();
	}

	bool ret = gfx::internal::scale_mouse_event(event);

	input::handle_event(event);

	if (ret) {
		return event;
	}

	if (guis.size() > 0) {
		gui::GUI *noo_gui = guis[guis.size()-1];
		if (noo_gui->is_transitioning_out() == false) {
			noo_gui->handle_event(event);
		}
	}

	if (!gfx::internal::gfx_context.fullscreen && event->type == TGUI_KEY_DOWN && event->keyboard.code == fullscreen_key && event->keyboard.is_repeat == false) {
		waiting_for_fullscreen_change = true;
		gfx::clear(shim::black);
		gfx::flip();
		gfx::internal::gfx_context.fullscreen_window = !gfx::internal::gfx_context.fullscreen_window;
		SDL_SetWindowFullscreen(gfx::internal::gfx_context.window, gfx::internal::gfx_context.fullscreen_window ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
		gfx::clear(shim::black);
		gfx::flip();
	}

	return event;
}

TGUI_Event *handle_event(SDL_Event *sdl_event)
{
	if (sdl_event->type == SDL_WINDOWEVENT && sdl_event->window.event == SDL_WINDOWEVENT_RESIZED) {
		waiting_for_fullscreen_change = false;
		handle_resize(sdl_event);
	}
	else if (sdl_event->type == SDL_WINDOWEVENT && sdl_event->window.event == SDL_WINDOWEVENT_ENTER) {
		gfx::internal::gfx_context.mouse_in_window = true;
	}
	else if (sdl_event->type == SDL_WINDOWEVENT && sdl_event->window.event == SDL_WINDOWEVENT_LEAVE) {
		gfx::internal::gfx_context.mouse_in_window = false;
	}
	else if (sdl_event->type == SDL_QUIT) {
		quitting = true;
	}

	if (sdl_event->type == timer_event_id) {
		tgui_event->type = TGUI_TICK;
	}
	else {
		*tgui_event = tgui_sdl_convert_event(sdl_event);
	}

	return real_handle_tgui_event(tgui_event);
}

TGUI_Event *pop_pushed_event()
{
	if (pushed_events.size() == 0) {
		return 0;
	}
	else {
		*tgui_event = pushed_events[0];
		pushed_events.erase(pushed_events.begin());
		return real_handle_tgui_event(tgui_event);
	}
}

bool event_in_queue(TGUI_Event e)
{
	for (size_t i = 0; i < pushed_events.size(); i++) {
		TGUI_Event &e2 = pushed_events[i];
		if (e.type == e2.type) {
			switch (e.type) {
				case TGUI_UNKNOWN:
				case TGUI_QUIT:
					return true;
				case TGUI_TICK:
					return true;
				case TGUI_KEY_DOWN:
				case TGUI_KEY_UP:
					if (memcmp(&e.keyboard, &e2.keyboard, sizeof(e.keyboard)) == 0) {
						return true;
					}
					break;
				case TGUI_MOUSE_DOWN:
				case TGUI_MOUSE_UP:
				case TGUI_MOUSE_AXIS:
				case TGUI_MOUSE_WHEEL:
					if (memcmp(&e.mouse, &e2.mouse, sizeof(e.mouse)) == 0) {
						return true;
					}
					break;
				case TGUI_JOY_DOWN:
				case TGUI_JOY_UP:
				case TGUI_JOY_AXIS:
				case TGUI_JOY_HAT:
					if (memcmp(&e.joystick, &e2.joystick, sizeof(e.joystick)) == 0) {
						return true;
					}
					break;
				case TGUI_FOCUS:
					if (memcmp(&e.focus, &e2.focus, sizeof(e.focus)) == 0) {
						return true;
					}
					break;
			}
		}
	}
	
	SDL_Event events[100];
	int n;
	n = SDL_PeepEvents(&events[0], 100, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
	for (int i = 0; i < n; i++) {
		SDL_Event &e2 = events[i];
		if (e2.type == SDL_KEYDOWN && e.type == TGUI_KEY_DOWN) {
			if (e.keyboard.code == e2.key.keysym.sym && e.keyboard.is_repeat == (e2.key.repeat != 0)) {
				return true;
			}
		}
		if (e2.type == SDL_KEYUP && e.type == TGUI_KEY_UP) {
			if (e.keyboard.code == e2.key.keysym.sym && e.keyboard.is_repeat == (e2.key.repeat != 0)) {
				return true;
			}
		}
		if (e2.type == SDL_MOUSEBUTTONDOWN && e.type == TGUI_MOUSE_DOWN) {
			if (e.mouse.button == e2.button.button && e.mouse.x == e2.button.x && e.mouse.y == e2.button.y) {
				return true;
			}
		}
		if (e2.type == SDL_MOUSEBUTTONUP && e.type == TGUI_MOUSE_UP) {
			if (e.mouse.button == e2.button.button && e.mouse.x == e2.button.x && e.mouse.y == e2.button.y) {
				return true;
			}
		}
		if (e2.type == SDL_MOUSEMOTION && e.type == TGUI_MOUSE_AXIS) {
			if (e.mouse.x == e2.motion.x && e.mouse.y == e2.motion.y) {
				return true;
			}
		}
		if (e2.type == SDL_MOUSEWHEEL && e.type == TGUI_MOUSE_WHEEL) {
			if (e.mouse.x == e2.wheel.x && e.mouse.y == e2.wheel.y) {
				return true;
			}
		}
		if (e2.type == SDL_JOYBUTTONDOWN && e.type == TGUI_JOY_DOWN) {
			if (e.joystick.id == e2.jbutton.which && e.joystick.button == e2.jbutton.button) {
				return true;
			}
		}
		if (e2.type == SDL_JOYBUTTONUP && e.type == TGUI_JOY_UP) {
			if (e.joystick.id == e2.jbutton.which && e.joystick.button == e2.jbutton.button) {
				return true;
			}
		}
		if (e2.type == SDL_JOYAXISMOTION && e.type == TGUI_JOY_AXIS) {
			float v = TGUI3_NORMALISE_JOY_AXIS(e2.jaxis.value);
			if (e2.jaxis.which == e.joystick.id && e2.jaxis.axis == e.joystick.axis && v == e.joystick.value) {
				return true;
			}
		}
		if (e2.type == SDL_JOYHATMOTION && e.type == TGUI_JOY_HAT) {
			int hat_x = -2, hat_y = -2;
			switch (e2.jhat.value) {
				case SDL_HAT_LEFTUP:
					hat_x = -1;
					hat_y = -1;
					break;
				case SDL_HAT_LEFT:
					hat_x = -1;
					hat_y = 0;
					break;
				case SDL_HAT_LEFTDOWN:
					hat_x = -1;
					hat_y = 1;
					break;
				case SDL_HAT_UP:
					hat_x = 0;
					hat_y = -1;
					break;
				case SDL_HAT_CENTERED:
					hat_x = 0;
					hat_y = 0;
					break;
				case SDL_HAT_DOWN:
					hat_x = 0;
					hat_y = 1;
					break;
					break;
				case SDL_HAT_RIGHTUP:
					hat_x = 1;
					hat_y = -1;
					break;
				case SDL_HAT_RIGHT:
					hat_x = 1;
					hat_y = 0;
					break;
				case SDL_HAT_RIGHTDOWN:
					hat_x = 1;
					hat_y = 1;
					break;
			}
			if (e2.jhat.which == e.joystick.id && hat_x == e.joystick.hat_x && hat_y == e.joystick.hat_y) {
				return true;
			}
		}
		if (e2.type == shim::timer_event_id && e.type == TGUI_TICK) {
			return true;
		}
	}

	return false;
}

bool update()
{
#if defined STEAMWORKS
	SteamAPI_RunCallbacks();
#endif

#if defined IOS || defined ANDROID
	if (app_in_background) {
		return false;
	}
#endif

	if (waiting_for_fullscreen_change) {
		return false;
	}

	if (waiting_for_resize) {
		return false;
	}

	if (quitting) {
		return false;
	}

#ifdef IOS
	if (adjust_screen_size) {
		adjust_screen_size = false;
		int width, height;
		SDL_GL_GetDrawableSize(gfx::internal::gfx_context.window, &width, &height);
		shim::real_screen_size.w = width;
		shim::real_screen_size.h = height;
		gfx::set_screen_size(shim::real_screen_size);
	}
#endif

    if (gfx::internal::gfx_context.work_image != 0 && gfx::internal::gfx_context.work_image->size != shim::real_screen_size) {
        gfx::internal::recreate_work_image();
	}

	input::update();

	if (guis.size() > 0) {
		gui::GUI *noo_gui = guis[guis.size()-1];
		std::vector<gui::GUI *> other_guis;
		other_guis.insert(other_guis.begin(), guis.begin(), guis.end()-1);
		if (noo_gui->gui && noo_gui->gui->get_focus() == 0) {
			noo_gui->gui->set_focus(noo_gui->focus);
		}
		if (noo_gui->is_transitioning_out() == false) {
			noo_gui->update();
		}
		// Not else if here, the state could change in update()
		if (noo_gui->is_transitioning_out() && noo_gui->is_transition_out_finished()) {
			// update may have pushed other GUIs on the stack, so we can't just erase the last one
			for (size_t i = 0; i < guis.size(); i++) {
				if (guis[i] == noo_gui) {
					guis.erase(guis.begin() + i);
					delete noo_gui;
					break;
				}
			}
		}
		for (size_t i = 0; i < other_guis.size(); i++) {
			noo_gui = other_guis[i];
			if (noo_gui->gui && noo_gui->gui->get_focus() != 0) {
				noo_gui->focus = noo_gui->gui->get_focus();
				noo_gui->gui->set_focus(0);
			}
			if (noo_gui->is_transitioning_out() == false) {
				noo_gui->update_background();
			}
			else if (noo_gui->is_transition_out_finished()) {
				// update may have pushed other GUIs on the stack, so we can't just erase the last one
				for (size_t j = 0; j < guis.size(); j++) {
					if (guis[j] == noo_gui) {
						guis.erase(guis.begin() + j);
						delete noo_gui;
						break;
					}
				}
			}
		}
	}

	return true;
}

void push_event(TGUI_Event event)
{
	pushed_events.push_back(event);
}

namespace internal {

TGUI_Event *handle_tgui_event(TGUI_Event *event)
{
	*tgui_event = *event;
	return real_handle_tgui_event(tgui_event);
}

}

} // End namespace shim

} // End namespace noo
