#include <zlib.h>

#ifdef TVOS
#include <Nooskewl_Shim/ios.h>
#endif

#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/input.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/onscreen_controller.h>
#include <Nooskewl_Wedge/systems.h>
#include <Nooskewl_Wedge/tile_movement.h>

#include "achievements.h"
#include "audio_settings.h"
#include "controls_settings.h"
#include "dialogue.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "inn.h"
#include "inventory.h"
#include "language_settings.h"
#include "monster_rpg_3.h"
#include "other_settings.h"
#include "question.h"
#include "save_slot.h"
#include "settings.h"
#include "stats.h"
#include "video_settings.h"

#ifdef __APPLE__
#include "apple.h"
#endif

static bool bander_enabled = false;

#ifdef ANDROID
#include <jni.h>

static bool osc_enabled = false;

extern "C" {
	JNIEXPORT void JNICALL Java_com_nooskewl_m3_Monster_1RPG_13_1Activity_pause
	  (JNIEnv *env, jobject obj)
	{
		if (GLOBALS) {
			osc_enabled = GLOBALS->onscreen_controller_was_enabled;
			if (osc_enabled) {
				GLOBALS->onscreen_controller_was_enabled = false;
				GLOBALS->onscreen_controller_temporarily_disabled = true;
			}

			while (shim::pop_pushed_event() != NULL) {
			}
			
			input::end();
		}
	}

	JNIEXPORT void JNICALL Java_com_nooskewl_m3_Monster_1RPG_13_1Activity_resume
	  (JNIEnv *env, jobject obj)
	{
		if (osc_enabled) {
			GLOBALS->onscreen_controller_was_enabled = true;
			GLOBALS->onscreen_controller_temporarily_disabled = false;
		}
		
		input::start();
	}

	JNIEXPORT jstring JNICALL Java_com_nooskewl_m3_Monster_1RPG_13_1Activity_translate
	  (JNIEnv *env, jobject obj, jint id)
	{
		jstring result;
		char translation[5000];
		strcpy(translation, GLOBALS->game_t->translate(id).c_str());
		result = env->NewStringUTF(translation); 
		return result; 
	}
}
#endif

static volatile bool do_autosave;
static volatile bool _end_autosave_thread;
static SDL_mutex *autosave_mutex;
static SDL_cond *autosave_cond;
static SDL_Thread *autosave_thread;

int autosave_func(void *data)
{
	while (true) {
		SDL_LockMutex(autosave_mutex);
		while (!_end_autosave_thread && !do_autosave) {
			SDL_CondWait(autosave_cond, autosave_mutex);
		}
		SDL_UnlockMutex(autosave_mutex);

		if (_end_autosave_thread) {
			break;
		}

#ifdef TVOS
		std::string fn = "auto5.dat";
		util::tvos_delete_file(fn);
#else
		std::string fn = save_dir() + "/" + "auto5.dat";
#ifdef _WIN32
		DeleteFile(fn.c_str());
#elif defined __APPLE__
		apple_delete_file(fn);
#else
		remove(fn.c_str());
#endif
#endif

		for (int i = 4; i >= 1; i--) {
#ifdef TVOS
			util::tvos_rename("auto" + util::itos(i) + ".dat", "auto" + util::itos(i+1) + ".dat");
#else
			rename(save_filename(i-1, "auto").c_str(), save_filename(i, "auto").c_str());
#endif
		}

		Uint32 now = GET_TICKS();
		Uint32 played_time = now - INSTANCE->play_start;
		INSTANCE->play_time += (played_time / 1000);
		INSTANCE->play_start = now;

		wedge::save(save_filename(0, "auto"));

		// files must be touched so syncing works correctly
		for (int i = 0; i < 5; i++) {
			std::string fn;
#ifdef TVOS
			fn = "auto" + util::itos(i+1) + ".dat";
			util::tvos_touch(fn);
#else
			fn = save_filename(i, "auto");
			utime(fn.c_str(), NULL);
#endif
		}

		do_autosave = false;
	}

	_end_autosave_thread = false;

	return 0;
}

void autosave(bool wait)
{
	SDL_LockMutex(autosave_mutex);
	do_autosave = true;
	SDL_CondSignal(autosave_cond);
	SDL_UnlockMutex(autosave_mutex);

	if (wait) {
		while (do_autosave == true) {
			SDL_Delay(1);
		}
	}
}

bool can_autosave()
{
	return can_show_settings(true, true) && INSTANCE->stats.size() == 2 && M3_INSTANCE->boatin == false && AREA->get_current_area()->get_next_area_name() == "";
}

void start_autosave_thread()
{
	do_autosave = false;
	_end_autosave_thread = false;
	autosave_mutex = SDL_CreateMutex();
	autosave_cond = SDL_CreateCond();
	autosave_thread = SDL_CreateThread(autosave_func, "autosave_thread", NULL);
	SDL_DetachThread(autosave_thread);
}

void end_autosave_thread()
{
	SDL_LockMutex(autosave_mutex);
	_end_autosave_thread = true;
	SDL_CondSignal(autosave_cond);
	SDL_UnlockMutex(autosave_mutex);

	while (_end_autosave_thread == true) {
		SDL_Delay(1);
	}

	SDL_DestroyCond(autosave_cond);
	SDL_DestroyMutex(autosave_mutex);
}

void do_question(std::string tag, std::string text, wedge::Dialogue_Type type, std::vector<std::string> choices, wedge::Step *monitor)
{
	wedge::Game *g;
	if (BATTLE) {
		g = BATTLE;
	}
	else {
		g = AREA;
	}

	NEW_SYSTEM_AND_TASK(g)
	Question_Step *q = new Question_Step(choices, new_task);
	if (monitor) {
		q->add_monitor(monitor);
	}
	ADD_STEP(q)
	ADD_TASK(new_task)
	ANOTHER_TASK
	Dialogue_Step *d = new Dialogue_Step(tag, text, type, wedge::DIALOGUE_TOP, new_task);
	d->set_dismissable(false);
	d->add_monitor(q);
	ADD_STEP(d)
	ADD_TASK(new_task)
	FINISH_SYSTEM(g)
}

int num_bands(int height)
{
	return MAX(4, height / (shim::screen_size.h / 16));
}

std::string get_status_name(int hp, int status)
{
	std::string s;

	if (hp <= 0) {
		s += GLOBALS->game_t->translate(1433)/* Originally: KO */;
	}
	else {
		switch (status) {
			case wedge::STATUS_OK:
				s += GLOBALS->game_t->translate(1434)/* Originally: OK */;
				break;
			case wedge::STATUS_POISONED:
				s += GLOBALS->game_t->translate(1435)/* Originally: Poison */;
				break;
			case STATUS_BLIND:
				s += GLOBALS->game_t->translate(1641)/* Originally: Blind */;
				break;
		}
	}

	return s;
}

SDL_Colour get_status_colour(int hp, int status)
{
	if (hp <= 0) {
		return shim::palette[9];
	}

	switch (status) {
		case wedge::STATUS_OK:
			return shim::palette[20];
		case wedge::STATUS_POISONED:
			return shim::palette[29];
		case STATUS_BLIND:
			return shim::black;
	}

	return shim::palette[20];
}

void start_pulse_brighten(float amount, bool need_untextured, bool need_textured, int cycle)
{
	const int half_cycle = cycle / 2;

	float brightness;
	Uint32 now = SDL_GetTicks();
	int mod = now % cycle;
	if (mod < half_cycle) {
		float p = mod/(float)half_cycle;
		p = p * p;
		brightness = 1.0f + amount * p;
	}
	else {
		float p = (mod-half_cycle)/(float)half_cycle;
		p = p * p;
		brightness = 1.0f + (amount - (amount * p));
	}

	if (need_textured) {
		if (need_untextured) {
			shim::current_shader = M3_GLOBALS->darken_both_shader; // can be used to brighten also
		}
		else {
			shim::current_shader = M3_GLOBALS->darken_textured_shader;
		}
	}
	else {
		shim::current_shader = M3_GLOBALS->darken_shader;
	}
	shim::current_shader->use();
	gfx::update_projection();
	shim::current_shader->set_float("brightness", brightness);
}

void end_pulse_brighten()
{
	shim::current_shader = shim::default_shader;
	shim::current_shader->use();
	gfx::update_projection();
}

// start_band and end_band: if band is not between, use start or end colour depending on use_start value
SDL_Colour *start_bander(int bands, SDL_Colour start, SDL_Colour end, float alpha)
{
	static SDL_Colour colours[4];

	colours[0] = start;
	colours[1] = start;
	colours[2] = end;
	colours[3] = end;

	float start_r = start.r / 255.0f;
	float start_g = start.g / 255.0f;
	float start_b = start.b / 255.0f;
	float end_r = end.r / 255.0f;
	float end_g = end.g / 255.0f;
	float end_b = end.b / 255.0f;

	float range[3];
	range[0] = fabsf(end_r - start_r) / bands;
	range[1] = fabsf(end_g - start_g) / bands;
	range[2] = fabsf(end_b - start_b) / bands;

	if (range[0] == 0 || range[1] == 0 || range[2] == 0) {
		return colours;
	}

	float m[3];
	m[0] = fmodf(start_r, range[0]);
	m[1] = fmodf(start_g, range[1]);
	m[2] = fmodf(start_b, range[2]);

	shim::current_shader = M3_GLOBALS->bander_shader;
	shim::current_shader->use();
	gfx::update_projection();

	shim::current_shader->set_float_vector("m", 3, m, 1);
	shim::current_shader->set_float_vector("range", 3, range, 1);
	shim::current_shader->set_float("alpha", alpha);
	
	if (start.r < end.r) { // won't work unless rgb all go up or all go down
		shim::current_shader->set_colour("cmin", start);
		shim::current_shader->set_colour("cmax", end);
	}
	else {
		shim::current_shader->set_colour("cmax", start);
		shim::current_shader->set_colour("cmin", end);
	}

	bander_enabled = true;

	return colours;
}

void end_bander()
{
	if (bander_enabled == false) {
		return;
	}

	bander_enabled = false;

	shim::current_shader = shim::default_shader;
	shim::current_shader->use();
	gfx::update_projection();
}

// A wee adjustment because sometimes the lines don't meet up exactly (I think because shim::scale is an imprecise number) (this is 1 REAL screen pixel)
float get_little_bander_offset()
{
	// trying to go without this...
	/*
	if (shim::scale > 2.0f) {
		return 1.25f / shim::scale;
	}
	*/
	return 0.0f;
}

SDL_Colour brighten(SDL_Colour colour, float amount)
{
	int r = colour.r;
	int g = colour.g;
	int b = colour.b;
	r *= 1.0f + amount;
	g *= 1.0f + amount;
	b *= 1.0f + amount;
	r = MIN(255, r);
	g = MIN(255, g);
	b = MIN(255, b);
	colour.r = r;
	colour.g = g;
	colour.b = b;
	colour.a = colour.a;
	return colour;
}

std::string save_dir()
{
	std::string path;

#ifdef ANDROID
	path = util::get_standard_path(util::SAVED_GAMES, true);
#elif defined _WIN32
	path = util::get_standard_path(util::SAVED_GAMES, true);
	path += "/" + shim::game_name;
	util::mkdir(path);
#else
	path = util::get_appdata_dir();
#endif

	return path;
}

std::string save_filename(int slot, std::string prefix)
{
#ifdef TVOS
	return prefix + util::itos(slot+1) + ".dat";
#else
	return save_dir() + "/" + prefix + util::itos(slot+1) + ".dat";
#endif
}

bool save()
{
	util::achieve((void *)ACHIEVE_SAVE);
	std::string filename = save_filename(M3_GLOBALS->save_slot);
	return wedge::save(filename);
}

bool save_play_time()
{
	if (M3_GLOBALS->save_slot < 0) {
		return false;
	}
	std::string filename = save_filename(M3_GLOBALS->save_slot);
	return wedge::save_play_time(filename);
}

std::string play_time_to_string(int time)
{
	int minutes = (time / 60) % 60;
	int hours = (time / 60 / 60) % 24;
	int days = time / 60 / 60 / 24;

	std::string s;

	if (days > 0) {
		s += util::string_printf("%d:", days);
		s += util::string_printf("%02d:", hours);
		s += util::string_printf("%02d", minutes);

	}
	else if (hours > 0) {
		s += util::string_printf("%d:", hours);
		s += util::string_printf("%02d", minutes);
	}
	else {
		s = util::string_printf("%dm", minutes);
	}

	return s;
}

void draw_tinted_shadowed_image(SDL_Colour tint, SDL_Colour shadow_colour, gfx::Image *image, util::Point<float> dest_pos, gfx::Font::Shadow_Type shadow_type)
{
	if (shadow_type != gfx::Font::NO_SHADOW) {
		shim::current_shader = M3_GLOBALS->solid_colour_shader;
		shim::current_shader->use();
		gfx::update_projection();
		shim::current_shader->set_colour("solid_colour", shadow_colour);
		image->start_batch();
		if (shadow_type == gfx::Font::DROP_SHADOW) {
			image->draw(dest_pos+util::Point<int>(1, 0));
			image->draw(dest_pos+util::Point<int>(0, 1));
			image->draw(dest_pos+util::Point<int>(1, 1));
		}
		else if (shadow_type == gfx::Font::FULL_SHADOW) {
			image->draw(dest_pos+util::Point<int>(1, 0));
			image->draw(dest_pos+util::Point<int>(-1, 0));
			image->draw(dest_pos+util::Point<int>(0, 1));
			image->draw(dest_pos+util::Point<int>(0, -1));
			image->draw(dest_pos+util::Point<int>(1, 1));
			image->draw(dest_pos+util::Point<int>(-1, 1));
			image->draw(dest_pos+util::Point<int>(1, -1));
			image->draw(dest_pos+util::Point<int>(-1, -1));
		}
		image->end_batch();
		shim::current_shader = shim::default_shader;
		shim::current_shader->use();
		gfx::update_projection();
	}
	image->draw_tinted(tint, dest_pos);
}

void draw_shadowed_image(SDL_Colour shadow_colour, gfx::Image *image, util::Point<float> dest_pos, gfx::Font::Shadow_Type shadow_type)
{
	draw_tinted_shadowed_image(shim::white, shadow_colour, image, dest_pos, shadow_type);
}

#ifndef TVOS
static std::string cfg_filename()
{
#ifdef ANDROID
	return save_dir() + "/config.ini";
#else
	return util::get_appdata_dir() + "/config.ini";
#endif
}
#endif

bool save_settings()
{
	std::string s;

	s += util::string_printf("windowed=%d\n", force_windowed);
	s += util::string_printf("fullscreen=%d\n", force_fullscreen);
	s += util::string_printf("screen_w=%d\n", force_screen_size.w);
	s += util::string_printf("screen_h=%d\n", force_screen_size.h);
	s += util::string_printf("sfx_volume=%f\n", sfx_volume);
	s += util::string_printf("music_volume=%f\n", music_volume);
	s += util::string_printf("key_b1=%d\n", GLOBALS->key_b1);
	s += util::string_printf("key_b2=%d\n", GLOBALS->key_b2);
	s += util::string_printf("key_b3=%d\n", GLOBALS->key_b3);
	s += util::string_printf("key_b4=%d\n", M3_GLOBALS->key_b4);
	s += util::string_printf("key_switch=%d\n", M3_GLOBALS->key_switch);
	s += util::string_printf("key_fs=%d\n", shim::fullscreen_key);
	s += util::string_printf("key_l=%d\n", GLOBALS->key_l);
	s += util::string_printf("key_r=%d\n", GLOBALS->key_r);
	s += util::string_printf("key_u=%d\n", GLOBALS->key_u);
	s += util::string_printf("key_d=%d\n", GLOBALS->key_d);
	s += util::string_printf("joy_b1=%d\n", GLOBALS->joy_b1);
	s += util::string_printf("joy_b2=%d\n", GLOBALS->joy_b2);
	s += util::string_printf("joy_b3=%d\n", GLOBALS->joy_b3);
	s += util::string_printf("joy_b4=%d\n", GLOBALS->joy_b4);
	s += util::string_printf("joy_switch=%d\n", GLOBALS->joy_switch);
	s += util::string_printf("tv_safe_mode=%d\n", M3_GLOBALS->tv_safe_mode ? 1 : 0);
	s += util::string_printf("hide_onscreen_settings_button=%d\n", M3_GLOBALS->hide_onscreen_settings_button ? 1 : 0);
	s += util::string_printf("rumble_enabled=%d\n", GLOBALS->rumble_enabled ? 1 : 0);
	s += util::string_printf("use_onscreen_controller=%d\n", (GLOBALS->onscreen_controller_was_enabled || GLOBALS->onscreen_controller_temporarily_disabled) ? 1 : 0);
	s += util::string_printf("use_hires_font=%d\n", shim::use_hires_font ? 1 : 0);

	std::string extra_args;
	std::vector<std::string> args;
	args.insert(args.begin(), bak_args.begin(), bak_args.end());
	args.insert(args.end(), cfg_args.begin(), cfg_args.end());
	for (size_t i = 0; i < args.size(); i++) {
		extra_args += args[i];
		if (i < args.size()-1) {
			extra_args += ",";
		}
	}
	s += util::string_printf("language=%s\n", GLOBALS->language.c_str());
	//s += util::string_printf("joy_index=%d\n", shim::joy_index);
	s += util::string_printf("extra_args=%s\n", extra_args.c_str());
#if defined __linux__ && !defined ANDROID && !defined STEAMWORKS
	s += util::string_printf("prompt_to_install_desktop=%d\n", prompt_to_install_desktop ? 1 : 0);
#endif

#ifdef TVOS
	util::tvos_save_file("config.ini", s);
#else
	FILE *f = fopen(cfg_filename().c_str(), "w");

	if (f == NULL) {
		return false;
	}

	fprintf(f, "%s", s.c_str());

	fclose(f);
#endif

	return true;
}

bool load_settings(bool globals_created)
{
#ifdef TVOS
	std::string s;
	if (util::tvos_read_file("config.ini", s) == false) {
		return false;
	}
#else
	int sz;
	char *bytes;
	try {
		bytes = util::slurp_file_from_filesystem(cfg_filename(), &sz);
	}
	catch (util::Error e) {
		return false;
	}
	std::string s = std::string(bytes, sz);
	delete[] bytes;
#endif

	std::string line;
	util::Tokenizer t(s, '\n');

	while ((line = t.next()) != "") {
		size_t equals = line.find('=');
		if (equals == std::string::npos) {
			continue;
		}

		std::string key = line.substr(0, equals);
		util::trim(key);
		std::string value = line.substr(equals+1);
		util::trim(value);

		if (key == "windowed") {
			force_windowed = atoi(value.c_str()) != 0;
		}
		else if (key == "fullscreen") {
			force_fullscreen = atoi(value.c_str()) != 0;
		}
		else if (key == "screen_w") {
			force_screen_size.w = atoi(value.c_str());
		}
		else if (key == "screen_h") {
			force_screen_size.h = atoi(value.c_str());
		}
		else if (key == "sfx_volume") {
			sfx_volume = atof(value.c_str());
			shim::sfx_volume = sfx_volume * sfx_amp;
		}
		else if (key == "music_volume") {
			music_volume = atof(value.c_str());
			shim::music_volume = music_volume * music_amp;
		}
		else if (key == "joy_index") {
			shim::joy_index = atoi(value.c_str());
		}
		else if (key == "extra_args") {
			util::Tokenizer t(value, ',');
			std::string arg;
			while ((arg = t.next()) != "") {
				cfg_args.push_back(arg);
			}
		}
		else if (key == "tv_safe_mode") {
			tmp_tv_safe_mode = atoi(value.c_str()) == 1 ? true : false;
			if (tmp_tv_safe_mode) {
				shim::black_bar_percent = 0.05f;
			}
		}
		else if (key == "use_hires_font") {
			shim::use_hires_font = atoi(value.c_str()) == 1 ? true : false;
		}
#if defined __linux__ && !defined ANDROID && !defined STEAMWORKS
		else if (key == "prompt_to_install_desktop") {
			prompt_to_install_desktop = atoi(value.c_str()) != 0;
		}
#endif
		else {
			if (globals_created) {
				if (key == "key_b1") {
					GLOBALS->key_b1 = atoi(value.c_str());
				}
				else if (key == "key_b2") {
					GLOBALS->key_b2 = atoi(value.c_str());
				}
				else if (key == "key_b3") {
					GLOBALS->key_b3 = atoi(value.c_str());
				}
				else if (key == "key_b4") {
					M3_GLOBALS->key_b4 = atoi(value.c_str());
				}
				else if (key == "key_switch") {
					M3_GLOBALS->key_switch = atoi(value.c_str());
				}
				else if (key == "key_fs") {
					shim::fullscreen_key = atoi(value.c_str());
				}
				else if (key == "key_l") {
					GLOBALS->key_l = atoi(value.c_str());
				}
				else if (key == "key_r") {
					GLOBALS->key_r = atoi(value.c_str());
				}
				else if (key == "key_u") {
					GLOBALS->key_u = atoi(value.c_str());
				}
				else if (key == "key_d") {
					GLOBALS->key_d = atoi(value.c_str());
				}
				else if (key == "joy_b1") {
					GLOBALS->joy_b1 = atoi(value.c_str());
				}
				else if (key == "joy_b2") {
					GLOBALS->joy_b2 = atoi(value.c_str());
				}
				else if (key == "joy_b3") {
					GLOBALS->joy_b3 = atoi(value.c_str());
				}
				else if (key == "joy_b4") {
					GLOBALS->joy_b4 = atoi(value.c_str());
				}
				else if (key == "joy_switch") {
					GLOBALS->joy_switch = atoi(value.c_str());
				}
				else if (key == "hide_onscreen_settings_button") {
					M3_GLOBALS->hide_onscreen_settings_button = atoi(value.c_str()) == 1 ? true : false;
				}
				else if (key == "rumble_enabled") {
					GLOBALS->rumble_enabled = atoi(value.c_str()) == 1 ? true : false;
				}
				else if (key == "use_onscreen_controller") {
					GLOBALS->onscreen_controller_was_enabled = (atoi(value.c_str()) == 1 ? true : false);
				}
				else if (key == "language") {
					GLOBALS->language = value;
				}
			}
			else {
				if (key == "key_b1") {
					tmp_key_b1 = atoi(value.c_str());
				}
				else if (key == "key_b2") {
					tmp_key_b2 = atoi(value.c_str());
				}
				else if (key == "key_b3") {
					tmp_key_b3 = atoi(value.c_str());
				}
				else if (key == "key_b4") {
					tmp_key_b4 = atoi(value.c_str());
				}
				else if (key == "key_switch") {
					tmp_key_switch = atoi(value.c_str());
				}
				else if (key == "key_fs") {
					tmp_key_fs = atoi(value.c_str());
				}
				else if (key == "key_l") {
					tmp_key_l = atoi(value.c_str());
				}
				else if (key == "key_r") {
					tmp_key_r = atoi(value.c_str());
				}
				else if (key == "key_u") {
					tmp_key_u = atoi(value.c_str());
				}
				else if (key == "key_d") {
					tmp_key_d = atoi(value.c_str());
				}
				else if (key == "joy_b1") {
					tmp_joy_b1 = atoi(value.c_str());
				}
				else if (key == "joy_b2") {
					tmp_joy_b2 = atoi(value.c_str());
				}
				else if (key == "joy_b3") {
					tmp_joy_b3 = atoi(value.c_str());
				}
				else if (key == "joy_b4") {
					tmp_joy_b4 = atoi(value.c_str());
				}
				else if (key == "joy_switch") {
					tmp_joy_switch = atoi(value.c_str());
				}
				else if (key == "hide_onscreen_settings_button") {
					tmp_hide_onscreen_settings_button = atoi(value.c_str()) == 1 ? true : false;
				}
				else if (key == "rumble_enabled") {
					tmp_rumble_enabled = atoi(value.c_str()) == 1 ? true : false;
				}
				else if (key == "use_onscreen_controller") {
					tmp_use_onscreen_controller = atoi(value.c_str()) == 1 ? true : false;
				}
				else if (key == "language") {
					tmp_language = value;
				}
			}
		}
	}

	return true;
}

std::string get_key_name(int code)
{
	std::string s = SDL_GetKeyName(code);
	s = util::uppercase(s);
	int id = GLOBALS->english_game_t->get_id(s);
	if (id == -1) {
		return s;
	}
	return GLOBALS->game_t->translate(id);
}

std::string get_joystick_button_name(int button)
{
	std::string s = input::joystick_button_to_name(button);
	int id = GLOBALS->english_game_t->get_id(s);
	if (id == -1) {
		return s;
	}
	return GLOBALS->game_t->translate(id);
}

static void save_callback2(void *data)
{
	Notification_GUI::Callback_Data *d = static_cast<Notification_GUI::Callback_Data *>(data);
	wedge::Step *step = static_cast<wedge::Step *>(d->userdata);
	if (step) {
		step->done_signal(NULL);
	}
}

static void save_callback(void *data)
{
	Save_Slot_GUI::Callback_Data *d = static_cast<Save_Slot_GUI::Callback_Data *>(data);
	wedge::Step *step = static_cast<wedge::Step *>(d->userdata);
	if (d->slot >= 0) {
		M3_GLOBALS->save_slot = d->slot;
		std::string message;
		if (save()) {
			message = GLOBALS->game_t->translate(1437)/* Originally: Game saved! */;
		}
		else {
			message = GLOBALS->game_t->translate(1438)/* Originally: An error occurred! */;
		}
		Notification_GUI *notification_gui = new Notification_GUI(message, save_callback2, step);
		shim::guis.push_back(notification_gui);
	}
	else {
		if (step) {
			step->done_signal(NULL);
		}
	}
}

void show_save_screen(wedge::Step *monitor)
{
	int slot = M3_GLOBALS->save_slot < 0 ? 0 : M3_GLOBALS->save_slot;
	Save_Slot_GUI *gui = new Save_Slot_GUI(true, slot, save_callback, monitor);
	shim::guis.push_back(gui);
}

bool inn_active()
{
	wedge::System_List systems = AREA->get_systems();
	for (wedge::System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		wedge::System *system = *it;
		wedge::Task_List tasks = system->get_tasks();
		for (wedge::Task_List::iterator it2 = tasks.begin(); it2 != tasks.end(); it2++) {
			wedge::Task *task = *it2;
			wedge::Step_List steps = task->get_steps();
			for (wedge::Step_List::iterator it3 = steps.begin(); it3 != steps.end(); it3++) {
				if (dynamic_cast<Inn_Step *>(*it3) != NULL) {
					return true;
				}
			}
		}
	}

	return false;
}

bool settings_active()
{
	for (size_t i = 0; i < shim::guis.size(); i++) {
		gui::GUI *g = shim::guis[i];
		if (
			dynamic_cast<Settings_GUI *>(g) ||
			dynamic_cast<Language_Settings_GUI *>(g) ||
			dynamic_cast<Video_Settings_GUI *>(g) ||
			dynamic_cast<Audio_Settings_GUI *>(g) ||
			dynamic_cast<Controls_Settings_GUI *>(g) ||
			dynamic_cast<Other_Settings_GUI *>(g)
		) {
			return true;
		}
	}
	return false;
}

bool can_show_settings(bool check_events, bool can_be_moving, bool only_initialised_dialogues_block, bool allow_paused_presses_if_changing_areas)
{
	if (check_events) {
		// if there are other key/joystick button/mouse button events in the queue, don't open settings
		// this is to prevent settings/dialogue or settings/battle action taking place at once
		SDL_Event events[100];
		int n;
		n = SDL_PeepEvents(&events[0], 100, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
		for (int i = 0; i < n; i++) {
			SDL_Event *e = &events[i];
			if (e->type == SDL_KEYDOWN || e->type == SDL_JOYBUTTONDOWN || e->type == SDL_MOUSEBUTTONDOWN) {
				return false;
			}
		}
	}

	if (AREA == NULL || settings_active() || inn_active() || (SHOP == NULL && GLOBALS->dialogue_active(AREA, only_initialised_dialogues_block)) || (BATTLE && GLOBALS->dialogue_active(BATTLE)) || (BATTLE == NULL && (wedge::are_presses_paused() && (allow_paused_presses_if_changing_areas == false || (AREA && AREA->changing_areas() == false)))) || AREA->is_pausing()) {
		return false;
	}

	wedge::Map_Entity *eny = AREA->get_player(ENY);
	wedge::Map_Entity_Input_Step *eny_input_step = eny->get_input_step();
	wedge::Map_Entity *tiggy = INSTANCE->party_following_player ? AREA->get_player(TIGGY) : NULL;

	if (can_be_moving == false && (eny_input_step->is_following_path() || eny_input_step->get_movement_step()->is_moving() || (tiggy != NULL && tiggy->get_input_step()->is_following_path()))) {
		return false;
	}

	return true;
}

void show_settings()
{
	if (can_show_settings(true) == false) {
		return;
	}

	M3_GLOBALS->button->play(false);
	Settings_GUI *gui = new Settings_GUI(true, Settings_GUI::LANGUAGE, 0);
	shim::guis.push_back(gui);
}

float get_ship_angle()
{
	Uint32 now = GET_TICKS();
	const int phase = 8000;
	const int half = phase / 2;
	Uint32 t = now % phase;
	float angle;

	if (t < half) {
		if (t < half/2) {
			angle = -(t / (half/2.0f)) * M_PI / 180.0f / 2.0f;
		}
		else {
			angle = -(1.0f - ((t - (half/2.0f)) / (half/2.0f))) * M_PI / 180.0f / 2.0f;
		}
	}
	else {
		if (t < half+half/2) {
			angle = ((t-half) / (half/2.0f)) * M_PI / 180.0f / 2.0f;
		}
		else {
			angle = (1.0f - ((t-half-half/2) / (half/2.0f))) * M_PI / 180.0f / 2.0f;
		}
	}

	return angle;
}

void apply_tv_safe_mode(bool onoff)
{
	if (onoff) {
		shim::black_bar_percent = 0.05f;
	}
	else {
		shim::black_bar_percent = 0.0f;
	}
	gfx::set_screen_size(shim::real_screen_size);
	gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
	gfx::update_projection();
}

void get_use_item_info(int amount, int id, SDL_Colour &colour, SDL_Colour &shadow_colour, std::string &text)
{
	if (amount < 0) {
		colour = shim::palette[9];
		shadow_colour = shim::palette[27];
		text = util::itos(abs(amount));
	}
	else if (amount > 0) {
		colour = shim::palette[13];
		shadow_colour = shim::palette[27];
		text = util::itos(abs(amount));
	}
	else {
		if (id == ITEM_CURE) {
			colour = shim::palette[19];
			shadow_colour = shim::palette[27];
			text = GLOBALS->game_t->translate(1439)/* Originally: CURE! */;
		}
		else if (id == ITEM_ELIXIR) {
			colour = shim::palette[11];
			shadow_colour = shim::palette[7];
			text = GLOBALS->game_t->translate(1440)/* Originally: MP! */;
		}
		else if (id == ITEM_HOLY_WATER) {
			colour = shim::palette[13];
			shadow_colour = shim::palette[27];
			text = GLOBALS->game_t->translate(1441)/* Originally: LIFE! */;
		}
		else {
			colour = shim::white;
			colour.a = 0;
			shadow_colour = shim::white;
			shadow_colour.a = 0;
			text = "";
		}
	}
}

bool is_mapped_key(int code)
{
	if (
		code == GLOBALS->key_b1 ||
		code == GLOBALS->key_b2 ||
		code == GLOBALS->key_b3 ||
		code == M3_GLOBALS->key_b4 ||
		code == M3_GLOBALS->key_switch ||
		code == shim::fullscreen_key ||
		code == GLOBALS->key_l ||
		code == GLOBALS->key_r ||
		code == GLOBALS->key_u ||
		code == GLOBALS->key_d
	) {
		return true;
	}
	return false;
}

SDL_Colour make_translucent(SDL_Colour colour, float alpha)
{
	SDL_Colour c = colour;
	c.r *= alpha;
	c.g *= alpha;
	c.b *= alpha;
	c.a *= alpha;
	return c;
}

std::vector<Dialogue_Step *> active_dialogues(wedge::Game *game)
{
	std::vector<Dialogue_Step *> dialogues;

	wedge::System_List systems = game->get_systems();
	for (wedge::System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		wedge::System *system = *it;
		wedge::Task_List tasks = system->get_tasks();
		for (wedge::Task_List::iterator it2 = tasks.begin(); it2 != tasks.end(); it2++) {
			wedge::Task *task = *it2;
			wedge::Step_List steps = task->get_steps();
			for (wedge::Step_List::iterator it3 = steps.begin(); it3 != steps.end(); it3++) {
				Dialogue_Step *d = dynamic_cast<Dialogue_Step *>(*it3);
				if (d != NULL) {
					dialogues.push_back(d);
				}
			}
		}
	}

	return dialogues;
}

std::vector<Inn_Step *> active_inns()
{
	std::vector<Inn_Step *> inns;

	wedge::System_List systems = AREA->get_systems();
	for (wedge::System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		wedge::System *system = *it;
		wedge::Task_List tasks = system->get_tasks();
		for (wedge::Task_List::iterator it2 = tasks.begin(); it2 != tasks.end(); it2++) {
			wedge::Task *task = *it2;
			wedge::Step_List steps = task->get_steps();
			for (wedge::Step_List::iterator it3 = steps.begin(); it3 != steps.end(); it3++) {
				Inn_Step *d = dynamic_cast<Inn_Step *>(*it3);
				if (d != NULL) {
					inns.push_back(d);
				}
			}
		}
	}

	return inns;
}

std::vector<Question_Step *> active_questions()
{
	std::vector<Question_Step *> questions;

	wedge::System_List systems = AREA->get_systems();
	for (wedge::System_List::iterator it = systems.begin(); it != systems.end(); it++) {
		wedge::System *system = *it;
		wedge::Task_List tasks = system->get_tasks();
		for (wedge::Task_List::iterator it2 = tasks.begin(); it2 != tasks.end(); it2++) {
			wedge::Task *task = *it2;
			wedge::Step_List steps = task->get_steps();
			for (wedge::Step_List::iterator it3 = steps.begin(); it3 != steps.end(); it3++) {
				Question_Step *d = dynamic_cast<Question_Step *>(*it3);
				if (d != NULL) {
					questions.push_back(d);
				}
			}
		}
	}

	return questions;
}

void barycenter(float *v1, float *v2, float *v3, float *x, float *y, float *z)
{
	*x = (v1[0] + v2[0] + v3[0]) / 3.0f;
	*y = (v1[1] + v2[1] + v3[1]) / 3.0f;
	*z = (v1[2] + v2[2] + v3[2]) / 3.0f;
}

util::JSON *load_savegame(int slot, std::string prefix)
{
	std::string filename = save_filename(slot, prefix);
	return wedge::load_savegame(filename);
}

void show_notice(std::string text, bool flip)
{
	gfx::Font::end_batches();

	SDL_Colour tint = shim::black;
	tint.r *= 0.75f;
	tint.g *= 0.75f;
	tint.b *= 0.75f;
	tint.a *= 0.75f;

	gfx::draw_filled_rectangle(tint, util::Point<int>(0, 0), shim::screen_size);

	bool full;
	int num_lines, width;
	int line_height = GLOBALS->bold_font->get_height() + 1;
	GLOBALS->bold_font->draw_wrapped(shim::white, text, util::Point<int>(0, 0), 80, line_height, -1, -1, 0, true, full, num_lines, width);

	int w = width + Dialogue_Step::BORDER*4;
	int h = line_height * num_lines + Dialogue_Step::BORDER*4;
	int x = (shim::screen_size.w-w)/2;
	int y = (shim::screen_size.h-h)/2;

	float little = get_little_bander_offset();

	SDL_Colour *colours;
	SDL_Colour colour;
	SDL_Colour border_colour;
	colour = shim::palette[20];
	colours = start_bander(num_bands(h-4), shim::palette[24], shim::palette[22]);
	border_colour = shim::palette[24];
	shim::current_shader->set_float("alpha", 1.0f);
	gfx::draw_filled_rectangle(colours, util::Point<float>(x+2-little, y+2-little), util::Size<float>(w-4+little*2.0f, h-4+little*2.0f));
	end_bander();

	// draw border and text

	gfx::draw_rectangle(border_colour, util::Point<int>(x, y)+util::Point<int>(1, 1), util::Size<int>(w, h)-util::Size<int>(2, 2));
	gfx::draw_rectangle(shim::palette[27], util::Point<int>(x, y), util::Size<int>(w, h));	
	
	GLOBALS->bold_font->enable_shadow(shim::palette[24], gfx::Font::DROP_SHADOW);
	GLOBALS->bold_font->draw_wrapped(shim::white, text, util::Point<int>(x+Dialogue_Step::BORDER*2, y+Dialogue_Step::BORDER*2), 80, line_height, -1, -1, 0, false, full, num_lines, width);
	GLOBALS->bold_font->disable_shadow();

	if (flip) {
		gfx::flip();
	}
}

