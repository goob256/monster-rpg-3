#include "Nooskewl_Wedge/main.h"
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/internal.h>
#include <Nooskewl_Wedge/omnipresent.h>
#include <Nooskewl_Wedge/onscreen_controller.h>

#ifdef ANDROID
#include <jni.h>
#endif

using namespace wedge;

namespace wedge {

static util::Size<int> last_screen_size;

static void lost_device()
{
	if (OMNIPRESENT != NULL) {
		OMNIPRESENT->lost_device();
	}
	if (BATTLE != NULL) {
		BATTLE->lost_device();
	}
	if (MENU != NULL) {
		MENU->lost_device();
	}
	if (SHOP != NULL) {
		SHOP->lost_device();
	}
	if (AREA != NULL) {
		AREA->lost_device();
	}
	delete globals->work_image;
	globals->work_image = NULL;
	delete globals->noise;
	globals->noise = NULL;
	
	gfx::destroy_fonts();
	delete GLOBALS->bold_font;
	GLOBALS->bold_font = NULL;
}

static void found_device()
{
	gfx::load_fonts();
	shim::font->set_vertex_cache_id(1);
	shim::font->start_batch();

	if (shim::use_hires_font) {
#ifdef USE_TTF
		GLOBALS->bold_font = new gfx::TTF("hbold.ttf", 8*int(shim::scale));
#else
		GLOBALS->bold_font = new gfx::Pixel_Font("hbold");
#endif
	}
	else {
		GLOBALS->bold_font = new gfx::Pixel_Font("bold");
	}
	GLOBALS->bold_font->set_vertex_cache_id(2);
	GLOBALS->bold_font->start_batch();

	globals->create_work_image();
	globals->create_noise();

	if (OMNIPRESENT != NULL) {
		OMNIPRESENT->found_device();
	}
	if (BATTLE != NULL) {
		BATTLE->found_device();
	}
	if (MENU != NULL) {
		MENU->found_device();
	}
	if (SHOP != NULL) {
		SHOP->found_device();
	}
	if (AREA != NULL) {
		AREA->found_device();
	}
}

static void draw_all()
{
#ifdef ANDROID
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "start_draw", "()V");

	if (method_id != NULL) {
		env->CallVoidMethod(activity, method_id);
	}

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);
#endif

#ifdef VERBOSE
	util::debugmsg("draw_all------------------------------------------------------------------\n");
#endif

#ifdef _WIN32
	if (shim::opengl == false && gfx::is_d3d_lost() == true) {
		gfx::flip(); // this is where lost devices are handled...
		return;
	}
#endif

	gfx::clear_buffers();

	if (BATTLE != NULL) {
		BATTLE->draw();
	}
	else if (MENU != NULL) {
		MENU->draw();
	}
	else if (SHOP != NULL) {
		SHOP->draw();
	}
	else if (AREA != NULL) {
		AREA->draw();
	}

	OMNIPRESENT->draw();

	if (BATTLE != NULL) {
		BATTLE->draw_fore();
	}
	else if (MENU != NULL) {
		MENU->draw_fore();
	}
	else if (SHOP != NULL) {
		SHOP->draw_fore();
	}
	else if (AREA != NULL) {
		AREA->draw_fore();
	}

	gfx::draw_guis();

	OMNIPRESENT->draw_fore();

	gfx::draw_notifications();

	if (is_onscreen_controller_enabled()) {
		draw_onscreen_controller();
	}

	gfx::flip();
}

bool start(util::Size<int> base_screen_size)
{
	gfx::register_lost_device_callbacks(lost_device, found_device);

	gfx::set_min_aspect_ratio(1.32f); // eg, iPad 4:3
	gfx::set_max_aspect_ratio(1.78f); // some Androids are higher, they'll get black bars (this is approx. 16:9)

	if (util::bool_arg(false, shim::argc, shim::argv, "logging")) {
		shim::logging = true;
	}

	if (util::bool_arg(false, shim::argc, shim::argv, "dump-images")) {
		gfx::Image::keep_data = true;
		gfx::Image::save_palettes = util::bool_arg(false, shim::argc, shim::argv, "save-palettes");
		gfx::Image::save_rle = true;
		gfx::Image::save_rgba = false;
		gfx::Image::premultiply_alpha = false;
	}

	gfx::set_minimum_window_size(base_screen_size * 4);
	util::Size<int> desktop_resolution = gfx::get_desktop_resolution();
	gfx::set_maximum_window_size(desktop_resolution);

	if (shim::start_all(base_screen_size.w, base_screen_size.h, true) == false) {
		gui::popup("shim::start failed", "Initialization failed.", gui::OK);
		return false;
	}

#ifdef _WIN32
	gfx::enable_press_and_hold(false);
#endif

	if (shim::font == 0) {
		gui::popup("Fatal Error", "data/gfx/fonts/font.ttf not found! Aborting.", gui::OK);
		return false;
	}

	shim::font->set_vertex_cache_id(1);
	shim::font->start_batch();

	if (util::bool_arg(false, shim::argc, shim::argv, "dump-images")) {
		std::vector<std::string> filenames = shim::cpa->get_all_filenames();
		for (size_t i = 0; i < filenames.size(); i++) {
			std::string filename =  filenames[i];
			if (filename.find(".tga") != std::string::npos) {
				gfx::Image *image = new gfx::Image(filename, true);
				std::string path = "out/" + filename;
				std::string dir;
				size_t i;
				while ((i = path.find("/")) != std::string::npos) {
					dir += path.substr(0, i);
					util::mkdir(dir.c_str());
					path = path.substr(i+1);
					dir += "/";
				}
				image->save("out/" + filename);
				delete image;
			}
		}
		exit(0);
	}

	shim::fullscreen_key = TGUIK_f;

	TGUI::set_focus_sloppiness(0);

	shim::user_render = draw_all;

	start_onscreen_controller(false);

	return true;
}

void handle_event(TGUI_Event *event)
{
	if (event->type == TGUI_UNKNOWN) {
		return;
	}

	if (event->type == TGUI_QUIT) {
		quit_all();
		GLOBALS->terminate = true;
	}

	if (event->type == TGUI_MOUSE_AXIS) {
		mouse_position = util::Point<int>((int)event->mouse.x, (int)event->mouse.y);
	}

	if (BATTLE != NULL) {
		BATTLE->handle_event(event);
	}
	else if (MENU != NULL) {
		MENU->handle_event(event);
	}
	else if (SHOP != NULL) {
		SHOP->handle_event(event);
	}
	else if (AREA != NULL) {
		AREA->handle_event(event);
	}
	
	OMNIPRESENT->handle_event(event);

	if (shim::screen_size != last_screen_size) {
		if (BATTLE != NULL) {
			BATTLE->resize(shim::screen_size);
		}
		else if (MENU != NULL) {
			MENU->resize(shim::screen_size);
		}
		else if (SHOP != NULL) {
			SHOP->resize(shim::screen_size);
		}
		else if (AREA != NULL) {
			AREA->resize(shim::screen_size);
		}
		
		OMNIPRESENT->resize(shim::screen_size);

		last_screen_size = shim::screen_size;
	}
}

static void loop()
{
	bool quit = false;

	// These keep the logic running at 60Hz and drawing at refresh rate is possible
	// NOTE: screen refresh rate has to be 60Hz or higher for this to work.
	const float target_fps = shim::refresh_rate <= 0 ? 60.0f : shim::refresh_rate;
	Uint32 start = SDL_GetTicks();
	int logic_frames = 0;
	int drawing_frames = 0;
	bool can_draw = true;
	bool can_logic = true;
	std::string old_music_name = "";
#if defined IOS || defined ANDROID
	float old_volume = 1.0f;
#endif

	last_screen_size = shim::screen_size;

	while (quit == false) {
		GLOBALS->loop();

		if (GLOBALS->work_image != NULL && GLOBALS->work_image->size != shim::real_screen_size) {
			delete GLOBALS->work_image;
			GLOBALS->create_work_image();
		}

		if (GLOBALS->onscreen_controller_was_enabled) {
			if (GLOBALS->can_walk() == false) {
				enable_onscreen_controller(false);
			}
			else {
				enable_onscreen_controller(true);
			}
		}
		else {
			enable_onscreen_controller(false);
		}

		// EVENTS
		SDL_Event sdl_event;

		while (SDL_PollEvent(&sdl_event)) {
			if (sdl_event.type == SDL_QUIT) {
				if (can_logic == false || (AREA == NULL && BATTLE == NULL)) {
					shim::handle_event(&sdl_event);
					quit = true;
					break;
				}
			}
			// right mouse clicks are transformed to escape keys
			else if (can_logic && sdl_event.type == SDL_MOUSEBUTTONDOWN && sdl_event.button.button == SDL_BUTTON_RIGHT && GLOBALS->title_gui_is_top() == false) {
				sdl_event.type = SDL_KEYDOWN;
				sdl_event.key.keysym.sym = SDLK_ESCAPE;
				sdl_event.key.repeat = 0;
			}
			else if ((sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP) && sdl_event.key.keysym.sym == SDLK_SELECT) {
				sdl_event.key.keysym.sym = globals->key_b1;
			}
			else if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_F12) {
				GLOBALS->load_translation();
			}
#ifdef TVOS
			else if ((sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP) && sdl_event.key.keysym.sym == SDLK_MENU) {
				sdl_event.key.keysym.sym = globals->key_b2;
			}
			else if ((sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP) && sdl_event.key.keysym.sym == SDLK_PAUSE) {
				sdl_event.key.keysym.sym = globals->key_switch;
			}
#endif
#ifdef ANDROID
			else if ((sdl_event.type == SDL_JOYBUTTONDOWN || sdl_event.type == SDL_JOYBUTTONUP) && sdl_event.jbutton.button == SDL_CONTROLLER_BUTTON_MAX+4) {
				// This is the "center" button found on Android TV remotes (the button in the center of the arrows)
				sdl_event.type = (sdl_event.type == SDL_JOYBUTTONDOWN) ? SDL_KEYDOWN : SDL_KEYUP;
				sdl_event.key.keysym.sym = GLOBALS->key_b1;
				sdl_event.key.repeat = 0;
			}
			// for some reason the back button doesn't work on android tv remote unless another keyboard/gamepad
			// is connected. this fixes it
			else if ((sdl_event.type == SDL_JOYBUTTONDOWN || sdl_event.type == SDL_JOYBUTTONUP) && sdl_event.jbutton.button == 4) {
				// first check '4' isn't used already as that would cause problems
				if (4 != GLOBALS->joy_b1 && 4 != GLOBALS->joy_b2 && 4 != GLOBALS->joy_b3 && 4 != GLOBALS->joy_b4 && 4 != GLOBALS->joy_switch) {
					// This is the "center" button found on Android TV remotes (the button in the center of the arrows)
					sdl_event.type = (sdl_event.type == SDL_JOYBUTTONDOWN) ? SDL_KEYDOWN : SDL_KEYUP;
					sdl_event.key.keysym.sym = GLOBALS->key_b2;
					sdl_event.key.repeat = 0;
				}
			}
			else if ((sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP) && sdl_event.key.keysym.sym == SDLK_AUDIOPLAY) {
				sdl_event.key.keysym.sym = globals->key_switch;
			}
			else if ((sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP) && sdl_event.key.keysym.sym == SDLK_AC_SEARCH) {
				sdl_event.key.keysym.sym = globals->key_b3;
			}
#endif

			if (can_logic) {
				// Android back key gets turned to key_b2
				if ((sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP) && sdl_event.key.keysym.sym == SDLK_AC_BACK) {
					sdl_event.key.keysym.sym = globals->key_b2;
				}

				bool is_red_triangle;
				if (sdl_event.type == SDL_FINGERDOWN) {
					float fx = sdl_event.tfinger.x;
					float fy = sdl_event.tfinger.y;
					fx *= shim::real_screen_size.w;
					fy *= shim::real_screen_size.h;
					fx -= shim::screen_offset.x;
					fy -= shim::screen_offset.y;
					fx /= shim::scale;
					fy /= shim::scale;
					is_red_triangle = fx < shim::tile_size && fy < shim::tile_size && fx < shim::tile_size-fy;
				}
				else {
					is_red_triangle = false;
				}

				if (is_red_triangle == false && is_onscreen_controller_enabled() && (sdl_event.type == SDL_MOUSEBUTTONDOWN || sdl_event.type == SDL_MOUSEBUTTONUP || sdl_event.type == SDL_MOUSEMOTION || handle_onscreen_controller(&sdl_event))) {
					continue;
				}

				TGUI_Event *event;
				
				if (sdl_event.type == SDL_QUIT) {
					static TGUI_Event quit_event;
					quit_event.type = TGUI_QUIT;
					event = &quit_event;
				}
				else {
					event = shim::handle_event(&sdl_event);
				}

				if (GLOBALS->is_mini_paused() == false) {
					handle_event(event);
				}
			}
		}

		if (quit) {
			break;
		}

		TGUI_Event *e;
		while ((e = shim::pop_pushed_event()) != NULL) {
			handle_event(e);
		}

		// TIMING
		int diff = SDL_GetTicks() - start;
		bool skip_drawing;
		int logic_reps;

		if (diff > 0) {
			float average;
			// Skip logic if running fast
			average = logic_frames / (diff / 1000.0f);
			if (average > shim::logic_rate-1) { // a little leeway
				logic_reps = 0;
			}
			else {
				logic_reps = shim::logic_rate - average;
			}
			// Skip drawing if running fast
			average = drawing_frames / (diff / 1000.0f);
			if (average < (target_fps-2.0f)) { // allow a little bit of fluctuation, i.e., not exactly target_fps here
				skip_drawing = true;
			}
			else {
				skip_drawing = false;
			}
		}
		else {
			skip_drawing = false;
			logic_reps = 1;
		}

		for (int logic = 0; logic < logic_reps; logic++) {
#if defined IOS || defined ANDROID
			bool could_draw = can_draw;
#endif
			can_draw = shim::update();
#if defined IOS || defined ANDROID
			can_logic = can_draw;
			if (could_draw != can_draw) {
				if (can_draw == false) {
					if (AREA != NULL && MENU == NULL && SHOP == NULL) {
						Uint32 pause_start_time = GET_TICKS();
						Uint32 played_time = pause_start_time - INSTANCE->play_start;
						INSTANCE->play_time += (played_time / 1000);
					}
					old_volume = shim::music->get_master_volume(); 
					shim::music->pause();
				}
				else {
					if (AREA != NULL && MENU == NULL && SHOP == NULL) {
						INSTANCE->play_start = GET_TICKS();
					}
					start = SDL_GetTicks();
					logic_frames = 0;
					drawing_frames = 0;
					shim::music->play(old_volume, true);
				}
			}
#endif

			// Generate a timer tick event (TGUI_TICK)
			SDL_Event sdl_event;
			sdl_event.type = shim::timer_event_id;
			TGUI_Event *event = shim::handle_event(&sdl_event);
			handle_event(event);

			if (is_onscreen_controller_enabled()) {
				update_onscreen_controller();
			}

			logic_frames++;
		}

		// LOGIC
		if (can_logic && GLOBALS->is_mini_paused() == false) {
			for (int logic = 0; logic < logic_reps; logic++) {
				gfx::update_animations();

				if (BATTLE != NULL) {
					if (BATTLE->run() == false) {
						delete BATTLE;
						BATTLE = NULL;
						OMNIPRESENT->end_fade();
					}
				}
				else if (MENU != NULL) {
					if (MENU->run() == false) {
						delete MENU;
						MENU = NULL;
					}
				}
				else if (SHOP != NULL) {
					if (SHOP->run() == false) {
						delete SHOP;
						SHOP = NULL;
					}
				}
				else if (AREA != NULL) {
					if (AREA->run() == false) {
						delete AREA;
						AREA = NULL;
					}
				}

				OMNIPRESENT->run();
			}
		}

		if (shim::guis.size() == 0 && AREA == NULL && BATTLE == NULL) {
			if (GLOBALS->terminate) {
				break;
			}
			if (globals->add_title_gui() == false) {
				break;
			}
		}

		// DRAWING
		if (skip_drawing == false && can_draw) {
			draw_all();
		}

		drawing_frames++;
	}
}

bool go()
{
	OMNIPRESENT = new Omnipresent_Game();
	OMNIPRESENT->start();

	globals->add_title_gui();

	loop();

	return true;
}

void end()
{
	shim::user_render = NULL;

	delete BATTLE;
	delete MENU;
	delete SHOP;
	delete AREA;
	delete OMNIPRESENT;

	// If Alt-F4 is pressed the title gui can remain in shim::guis. Leaving it to shim to destruct won't work, because ~Title_GUI accesses Globals which is destroyed below
	for (std::vector<gui::GUI *>::iterator it = shim::guis.begin(); it != shim::guis.end();) {
		gui::GUI *gui = *it;
		delete gui;
		it = shim::guis.erase(it);
	}

	delete globals;

	shim::end_all();
}

}
