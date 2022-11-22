#include <Nooskewl_Shim/savetool.h>

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/onscreen_controller.h>

#include "bolt.h"
#include "cure.h"
#include "enemies.h"
#include "general.h"
#include "globals.h"
#include "monster_rpg_3.h"
#include "settings.h"
#include "vice.h"
#include "widgets.h"

#if defined __linux__ && !defined ANDROID && !defined STEAMWORKS
#include <unistd.h>

std::string desktop_contents = 
	"[Desktop Entry]\n"
	"Encoding=UTF-8\n"
	"Value=1.0\n"
	"Type=Application\n"
	"Name=Monster RPG 3\n"
	"GenericName=Monster RPG 3\n"
	"Comment=Monster RPG 3\n"
	"Icon=FIXME/MonsterRPG3.png\n"
	"Exec=\"FIXME/MonsterRPG3\"\n"
	"Categories=Game;\n"
	"Path=FIXME";
#endif

#ifdef _WIN32
#define strdup _strdup
#endif

#ifdef IOS
#include <Nooskewl_Shim/ios.h>
#endif

#if defined IOS || defined MAS
#include <Nooskewl_Shim/apple.h>
#include <Nooskewl_Shim/gamecenter.h>
#endif

util::Size<int> desktop_resolution;
bool force_windowed;
bool force_fullscreen;
util::Size<int> force_screen_size;
int tmp_key_b1;
int tmp_key_b2;
int tmp_key_b3;
int tmp_key_b4;
int tmp_key_switch;
int tmp_key_fs;
int tmp_key_l;
int tmp_key_r;
int tmp_key_u;
int tmp_key_d;
int tmp_joy_b1;
int tmp_joy_b2;
int tmp_joy_b3;
int tmp_joy_b4;
#ifdef ANDROID
int tmp_joy_switch;
#else
int tmp_joy_switch;
#endif
bool tmp_use_onscreen_controller;
bool tmp_tv_safe_mode;
bool tmp_hide_onscreen_settings_button;
bool tmp_rumble_enabled;
std::string tmp_language;
#if defined __linux__ && !defined ANDROID && !defined STEAMWORKS
bool prompt_to_install_desktop;
#endif
int orig_argc;
char **orig_argv;
char **my_argv;
int my_argc;
float sfx_volume;
float music_volume;
std::vector<std::string> cfg_args;
std::vector<std::string> bak_args;

void delete_shim_args()
{
	for (int i = 0; i < my_argc; i++) {
		free(my_argv[i]);
	}
	free(my_argv);
	
	shim::argc = orig_argc;
	shim::argv = orig_argv;
}

void set_shim_args(bool keep_initial)
{
	// certain arguments are not allowed/only allowed the first time
	for (std::vector<std::string>::iterator it = cfg_args.begin(); it != cfg_args.end();) {
		std::string arg = *it;
		if (arg == "+windowed") {
			it = cfg_args.erase(it);
			force_windowed = true;
			bak_args.push_back("+windowed");
		}
		else if (arg == "+fullscreen") {
			it = cfg_args.erase(it);
			force_fullscreen = true;
			bak_args.push_back("+fullscreen");
		}
		else if (arg == "+width") {
			it = cfg_args.erase(it);
			std::string w = *it;
			force_screen_size.w = atoi(w.c_str());
			it = cfg_args.erase(it);
			bak_args.push_back("+width");
			bak_args.push_back(w);
		}
		else if (arg == "+height") {
			it = cfg_args.erase(it);
			std::string h = *it;
			force_screen_size.h = atoi(h.c_str());
			it = cfg_args.erase(it);
			bak_args.push_back("+height");
			bak_args.push_back(h);
		}
		else if (arg == "+scale") {
			it = cfg_args.erase(it);
			bak_args.push_back("+scale");
			bak_args.push_back(*it);
			it = cfg_args.erase(it);
		}
		else {
			it++;
		}
	}

	int count = 0;
	for (int i = 0; i < orig_argc;) {
		if (!strcmp(orig_argv[i], "+windowed")) {
			i++;
			count++;
		}
		else if (!strcmp(orig_argv[i], "+fullscreen")) {
			i++;
			count++;
		}
		else if (!strcmp(orig_argv[i], "+width")) {
			i += 2;
			count += 2;
		}
		else if (!strcmp(orig_argv[i], "+height")) {
			i += 2;
			count += 2;
		}
		else if (!strcmp(orig_argv[i], "+scale")) {
			i += 2;
			count += 2;
		}
		else {
			i++;
		}
	}
	bool user_mode = count > 0;
	int extra = (int)cfg_args.size();
	if (keep_initial == false || user_mode == false) {
		extra++; // for windowed/fullscreen
	}
	if (keep_initial == false || user_mode == false) {
		extra += 4; // for size
	}
	my_argc = (orig_argc - (keep_initial ? 0 : count)) + extra;
	my_argv = (char **)malloc(my_argc * sizeof(char *));
	count = 0;
	for (int i = 0; i < orig_argc;) {
		if (keep_initial == false) {
			if (!strcmp(orig_argv[i], "+windowed")) {
				i++;
				continue;
			}
			else if (!strcmp(orig_argv[i], "+fullscreen")) {
				i++;
				continue;
			}
			else if (!strcmp(orig_argv[i], "+width")) {
				i += 2;
				continue;
			}
			else if (!strcmp(orig_argv[i], "+height")) {
				i += 2;
				continue;
			}
			else if (!strcmp(orig_argv[i], "+scale")) {
				i += 2;
				continue;
			}
		}
		my_argv[count++] = strdup(orig_argv[i]);
		i++;
	}
	if (keep_initial == false || user_mode == false) {
		if (force_windowed) {
			my_argv[count++] = strdup("+windowed");
		}
		else {
			my_argv[count++] = strdup("+fullscreen");
		}
		my_argv[count++] = strdup("+width");
		my_argv[count++] = strdup(util::itos(force_screen_size.w).c_str());
		my_argv[count++] = strdup("+height");
		my_argv[count++] = strdup(util::itos(force_screen_size.h).c_str());
	}

	for (size_t i = 0; i < cfg_args.size(); i++) {
		my_argv[count++] = strdup(cfg_args[i].c_str());
	}

	shim::argc = my_argc;
	shim::argv = my_argv;
}

#ifdef GOOGLE_PLAY
#include <jni.h>

void start_google_play_games_services()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "start_google_play_games_services", "()V");

	env->CallVoidMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);
}
#endif

int main(int argc, char **argv)
{
	// Save game compression/decompression utility
	for (int i = 0; i < argc; i++) {
		if (std::string(argv[i]) == "+compress-save") {
			if (i+2 < argc) {
				do_compress(argv[i+1], argv[i+2], true);
				return 0;
			}
		}
		if (std::string(argv[i]) == "+decompress-save") {
			if (i+2 < argc) {
				do_decompress(argv[i+1], argv[i+2]);
				return 0;
			}
		}
	}

	try {
#ifdef _WIN32
		SDL_RegisterApp("MonsterRPG3", 0, 0);
#endif

#if defined IOS || defined ANDROID
		SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown");
#endif

#if defined IOS || defined MAS
		util::apple_set_cloud_info("iCloud.ca.nooskewl.monsterrpg3", "MoRPG3");
#endif

		// this must be before static_start which inits SDL
#ifdef _WIN32
		bool directsound = util::bool_arg(true, argc, argv, "directsound");
		bool wasapi = util::bool_arg(false, argc, argv, "wasapi");
		bool winmm = util::bool_arg(false, argc, argv, "winmm");

		if (directsound) {
			_putenv_s("SDL_AUDIODRIVER", "directsound");
		}
		else if (wasapi) {
			_putenv_s("SDL_AUDIODRIVER", "wasapi");
		}
		else if (winmm) {
			_putenv_s("SDL_AUDIODRIVER", "winmm");
		}
#endif

		shim::static_start_all();

#if defined IOS || defined MAS
		util::static_init_gamecenter();
#endif

		start_autosave_thread();
		
		Settings_GUI::static_start();

		Bolt_Step::static_start();
		Cure_Step::static_start();
		vIce_Step::static_start();

		Enemy_Bloated::static_start();
		Enemy_Sludge::static_start();
		Enemy_Ghastly::static_start();
		Enemy_Werewolf::static_start();
		Enemy_Knightly::static_start();
		Enemy_Shocker::static_start();
		Enemy_Cyclone::static_start();
		Enemy_Bones::static_start();
		Enemy_Shadow::static_start();

		force_windowed = false;
		force_fullscreen = true;
		force_screen_size = util::Size<int>(-1, -1);
		tmp_key_b1 = TGUIK_RETURN;
		tmp_key_b2 = TGUIK_ESCAPE;
		tmp_key_b3 = TGUIK_e;
		tmp_key_b4 = TGUIK_F1;
		tmp_key_switch = TGUIK_COMMA;
		tmp_key_fs = TGUIK_f;
		tmp_key_l = TGUIK_LEFT;
		tmp_key_r = TGUIK_RIGHT;
		tmp_key_u = TGUIK_UP;
		tmp_key_d = TGUIK_DOWN;
		tmp_joy_b1 = 0;
		tmp_joy_b2 = 1;
		tmp_joy_b3 = 2;
		tmp_joy_b4 = 3;
#ifdef ANDROID
		tmp_joy_switch = 10;
#else
		tmp_joy_switch = 5;
#endif
		tmp_use_onscreen_controller = false;
		tmp_tv_safe_mode = false;
		tmp_hide_onscreen_settings_button = false;
#if defined ANDROID || defined IOS
		tmp_rumble_enabled = false;
#else
		tmp_rumble_enabled = true;
#endif
		tmp_language = "English";
#if defined __linux__ && !defined ANDROID && !defined STEAMWORKS
		prompt_to_install_desktop = true;
#endif
		sfx_volume = 1.0f;
		music_volume = 1.0f;

		shim::window_title = "Monster RPG 3";
		shim::organization_name = "Nooskewl";
		shim::game_name = "Monster RPG 3";
		shim::convert_xbox_dpad_to_arrows = true;
		shim::tile_size = 12;
		shim::notification_duration = 2500;
		shim::notification_fade_duration = 250;
		shim::create_depth_buffer = true;
		// for the initial window paint
		//--
		shim::black.r = 35;
		shim::black.g = 30;
		shim::black.b = 60;
		shim::black.a = 255;
		//--
		shim::convert_directions_to_focus_events = true;
		gfx::Image::ignore_palette = true;
		shim::logging = true;
		shim::allow_dpad_below = true;

		desktop_resolution = gfx::get_desktop_resolution(); // need to get this before any modes are set

#if defined IOS || defined ANDROID
		wedge::set_onscreen_controller_b2_enabled(false); // so b2 doesn't flash on at the beginning
#endif

		bool settings_loaded = load_settings(false);

		if (settings_loaded == false) {
			std::map<std::string, std::string> languages;

			languages["german"] = "German";
			languages["french"] = "French";
			languages["dutch"] = "Dutch";
			languages["greek"] = "Greek";
			languages["italian"] = "Italian";
			languages["polish"] = "Polish";
			languages["portuguese"] = "Portuguese";
			languages["russian"] = "Russian";
			languages["spanish"] = "Spanish";
			languages["korean"] = "Korean";
			languages["english"] = "English";
			languages["brazilian"] = "Brazilian";

			std::string syslang = util::get_system_language();

			tmp_language = languages[syslang];

			if (tmp_language != "English" && tmp_language != "French" && tmp_language != "Spanish" && tmp_language != "Brazilian") {
				tmp_language = "English";
			}
			
			shim::music_volume = music_volume * music_amp;
		}

		orig_argc = argc;
		orig_argv = argv;

		set_shim_args(true);

		if (wedge::start(util::Size<int>(SCR_W, SCR_H)) == false) {
			util::errormsg("wedge::start failed.\n");
			return 1;
		}

		// Colour settings must come after palette loaded

		Widget::static_start();

		Widget_List::set_default_focussed_bar_colour(shim::palette[14]);
		Widget_List::set_default_bar_colour(shim::palette[14]);
		Widget_List::set_default_normal_text_colour(shim::palette[20]);
		Widget_List::set_default_selected_text_colour(shim::palette[20]);
		Widget_List::set_default_text_shadow_colour(shim::palette[24]);
		Widget_List::set_default_selected_text_shadow_colour(shim::palette[15]);
		Widget_List::set_default_unfocussed_selected_text_shadow_colour(shim::palette[15]);
		Widget_List::set_default_highlight_text_colour(shim::palette[19]);
		Widget_List::set_default_highlight_text_shadow_colour(shim::palette[24]);
		Widget_List::set_default_disabled_text_colour(shim::black);
		Widget_Label::set_default_text_colour(shim::palette[20]);
		Widget_Label::set_default_shadow_colour(shim::palette[24]);

		shim::white = shim::palette[20]; // use our own white, it's actual white but could change
		shim::black = shim::palette[27]; // black doesn't exist in our palette

		wedge::globals = new Monster_RPG_3_Globals();

		// FIXME: use this if using TTFs
		/*
		shim::font->end_batch();
		shim::font->set_vertex_cache_id(0);
		GLOBALS->bold_font->end_batch();
		GLOBALS->bold_font->set_vertex_cache_id(0);
		*/

		if (settings_loaded) {
			GLOBALS->key_b1 = tmp_key_b1;
			GLOBALS->key_b2 = tmp_key_b2;
			GLOBALS->key_b3 = tmp_key_b3;
			M3_GLOBALS->key_b4 = tmp_key_b4;
			M3_GLOBALS->key_switch = tmp_key_switch;
			shim::fullscreen_key = tmp_key_fs;
			shim::key_l = GLOBALS->key_l = tmp_key_l;
			shim::key_r = GLOBALS->key_r = tmp_key_r;
			shim::key_u = GLOBALS->key_u = tmp_key_u;
			shim::key_d = GLOBALS->key_d = tmp_key_d;
			GLOBALS->joy_b1 = tmp_joy_b1;
			GLOBALS->joy_b2 = tmp_joy_b2;
			GLOBALS->joy_b3 = tmp_joy_b3;
			GLOBALS->joy_b4 = tmp_joy_b4;
			GLOBALS->joy_switch = tmp_joy_switch;
			M3_GLOBALS->tv_safe_mode = tmp_tv_safe_mode;
			M3_GLOBALS->hide_onscreen_settings_button = tmp_hide_onscreen_settings_button;
			GLOBALS->rumble_enabled = tmp_rumble_enabled;
			GLOBALS->onscreen_controller_was_enabled = tmp_use_onscreen_controller;
		}

		// Even if settings not loaded...
		GLOBALS->language = tmp_language;

		GLOBALS->load_translation();

#if defined __linux__ && !defined ANDROID && !defined STEAMWORKS
		// Install a .desktop if the user wants
		if (util::bool_arg(false, shim::argc, shim::argv, "install")) {
			prompt_to_install_desktop = true;
		}
		if (prompt_to_install_desktop && false) { // NOTE the false... this is disabled
			const char *home = getenv("HOME");
			if (home) {
				std::string path = std::string(home) + "/.local/share/applications/MonsterRPG3.desktop";
				FILE *f = fopen(path.c_str(), "r");
				if (f == NULL) {
					int popup_result = gui::popup(GLOBALS->game_t->translate(1594)/* Originally: Install .desktop? */, GLOBALS->game_t->translate(1595)/* Originally: Install .desktop file to ~/.local/share/applications?\nThis will give you a shiny icon. */, gui::YESNO);
					// -1 == error, best thing we can do in case of error is just go ahead and instlal the .desktop
					if (popup_result == 0 || popup_result == -1) {
						char buf[1000];
						if (getcwd(buf, 1000)) {
							size_t pos;
							while ((pos = desktop_contents.find("FIXME")) != std::string::npos) {
								desktop_contents.replace(pos, 5, buf);
							}
							f = fopen(path.c_str(), "w");
							if (f) {
								fwrite(desktop_contents.c_str(), desktop_contents.length(), 1, f);
								fclose(f);
								chmod(path.c_str(), S_IRWXU);
							}
						}
					}
					else {
						prompt_to_install_desktop = false;
					}
				}
				else {
					fclose(f);
				}
			}
		}
#endif

#ifdef GOOGLE_PLAY
		start_google_play_games_services();
#elif defined IOS || defined MAS
		util::init_gamecenter();
#endif

		if (wedge::go() == false) {
			util::debugmsg("wedge::go return false.\n");
			return 1;
		}

		GLOBALS->max_battle_steps = M3_GLOBALS->saved_max_battle_steps;
		GLOBALS->min_battle_steps = M3_GLOBALS->saved_min_battle_steps;
		
		save_settings(); // settings could be changed then alt-f4 pressed

		wedge::end();

		delete_shim_args();

		end_autosave_thread();
	
		shim::static_end();
	}
	catch (util::Error e) {
		util::errormsg("Fatal error: %s\n", e.error_message.c_str());
		gui::popup("Fatal Error", e.error_message, gui::OK);
		return 1;
	}

#if defined IOS || defined ANDROID
	// iOS: This should only happen if the app is terminated by the OS, exiting by the user is not enabled on iOS (not allowed by Apple)
	exit(0);
#endif

	return 0;
}
