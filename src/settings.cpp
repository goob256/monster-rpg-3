#include <sstream>

#ifdef TVOS
#include <Nooskewl_Shim/ios.h>
#endif

#include <Nooskewl_Wedge/onscreen_controller.h>

#include "audio_settings.h"
#include "controls_settings.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "language_settings.h"
#include "other_settings.h"
#include "settings.h"
#include "video_settings.h"

#if defined IOS || defined MAS
#include "apple.h"
#endif

#if defined IOS
#include "ios.h"
#endif

#define DIRECTORY ""

#ifdef ANDROID
#include <jni.h>

static SDL_mutex *android_mutex;
static SDL_cond *android_cond;
static bool android_bool;
static bool achievements_bool;

extern "C" {
	JNIEXPORT void JNICALL Java_com_nooskewl_m3_Monster_1RPG_13_1Activity_resume_1after_1showing_1license
	  (JNIEnv *env, jobject obj)
	{
		SDL_LockMutex(android_mutex);
		android_bool = true;
		SDL_CondSignal(android_cond);
		SDL_UnlockMutex(android_mutex);
	}

	JNIEXPORT void JNICALL Java_com_nooskewl_m3_Monster_1RPG_13_1Activity_resume_1after_1showing_1manual
	  (JNIEnv *env, jobject obj)
	{
		SDL_LockMutex(android_mutex);
		android_bool = true;
		SDL_CondSignal(android_cond);
		SDL_UnlockMutex(android_mutex);
	}

	JNIEXPORT void JNICALL Java_com_nooskewl_m3_Monster_1RPG_13_1Activity_resume_1after_1showing_1achievements
	  (JNIEnv *env, jobject obj)
	{
		achievements_bool = true;
	}
}

static int get_show_license_result()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "getShowLicenseResult", "()I");

	int result = env->CallIntMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}

static int get_show_manual_result()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "getShowManualResult", "()I");

	int result = env->CallIntMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}
#endif

//#if defined GOOGLE_PLAY || defined AMAZON || defined IOS || defined MAS
#if defined AMAZON || defined IOS || defined MAS
#ifdef AMAZON
static int amazon_initialised()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "initialised", "()I");

	int result = env->CallIntMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}

static std::string amazon_errmsg()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id;
	
	method_id = env->GetMethodID(clazz, "amazon_get_error_message", "()Ljava/lang/String;");

	jstring s = (jstring)env->CallObjectMethod(activity, method_id);

	const char *native = env->GetStringUTFChars(s, 0);

	std::string msg = native;

	env->ReleaseStringUTFChars(s, native);

	env->DeleteLocalRef(s);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return msg;
}

static bool amazon_is_fire_tv()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "is_fire_tv", "()Z");

	bool result = (bool)env->CallBooleanMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}
#endif

static std::vector<int> overwrite_local_save;
static std::vector<int> overwrite_cloud_save;
static std::vector<int> overwrite_local_auto;
static std::vector<int> overwrite_cloud_auto;

static std::map<std::string, char *> byte_cache;
static std::map<std::string, int> size_cache;

static void delete_byte_cache()
{
	std::map<std::string, char *>::iterator it = byte_cache.begin();
	while (it != byte_cache.end()) {
		std::pair<std::string, char *> p = *it;
		delete[] p.second;
		byte_cache.erase(it++);
	}
	byte_cache.clear();
	size_cache.clear();
}

static std::string time_to_string(Sint64 t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

static void real_sync_callback(void *data)
{
	gfx::clear(shim::black);
	show_notice(GLOBALS->game_t->translate(1721), true);

	std::string directory = DIRECTORY;

	std::map<std::string, char *>::iterator it;

	Yes_No_GUI::Callback_Data *d = static_cast<Yes_No_GUI::Callback_Data *>(data);
	if (d->cancelled == false && d->choice == true) {
		bool success = true;
		for (size_t i = 0; i < overwrite_local_save.size(); i++) {
			int slot = overwrite_local_save[i];
			int sz;
			char *bytes;
			std::string fn = directory + "save" + util::itos(slot+1) + ".dat";
			if ((it = byte_cache.find(fn)) != byte_cache.end()) {
				std::pair<std::string, char *> p = *it;
				bytes = p.second;
				sz = size_cache[fn];
				byte_cache.erase(it);
			}
			else {
				bytes = util::cloud_read(fn, &sz);
			}
			if (bytes == NULL) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713) + " (" + util::itos(util::cloud_get_error_code()) + ")");
				shim::guis.push_back(gui);
				delete_byte_cache();
				return;
			}
			else {
#ifdef TVOS
				if (util::tvos_save_bytes(save_filename(slot), bytes, sz) == false) {
					success = false;
				}
				else {
					Sint64 now = util::file_date(save_filename(slot).c_str());
					std::string t_s = time_to_string(now);
					fn = fn.substr(0, fn.rfind('.')) + ".txt";
					util::cloud_save(fn, t_s.c_str(), (int)t_s.length());
				}
#else
				SDL_RWops *file = SDL_RWFromFile(save_filename(slot).c_str(), "wb");
				if (file) {
					if ((int)SDL_RWwrite(file, bytes, 1, sz) != sz) {
						success = false;
					}
					SDL_RWclose(file);
					Sint64 now = util::file_date(save_filename(slot).c_str());
					std::string t_s = time_to_string(now);
					fn = fn.substr(0, fn.rfind('.')) + ".txt";
					util::cloud_save(fn, t_s.c_str(), (int)t_s.length());
				}
				else {
					success = false;
				}
#endif
				delete[] bytes;
			}
		}

		for (size_t i = 0; i < overwrite_local_auto.size(); i++) {
			int slot = overwrite_local_auto[i];
			int sz;
			char *bytes;
			std::string fn = directory + "auto" + util::itos(slot+1) + ".dat";
			if ((it = byte_cache.find(fn)) != byte_cache.end()) {
				std::pair<std::string, char *> p = *it;
				bytes = p.second;
				sz = size_cache[fn];
				byte_cache.erase(it);
			}
			else {
				bytes = util::cloud_read(fn, &sz);
			}
			if (bytes == NULL) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713) + " (" + util::itos(util::cloud_get_error_code()) + ")");
				shim::guis.push_back(gui);
				delete_byte_cache();
				return;
			}
			else {
#ifdef TVOS
				if (util::tvos_save_bytes(save_filename(slot, "auto"), bytes, sz) == false) {
					success = false;
				}
				else {
					Sint64 now = util::file_date(save_filename(slot, "auto").c_str());
					std::string t_s = time_to_string(now);
					fn = fn.substr(0, fn.rfind('.')) + ".txt";
					util::cloud_save(fn, t_s.c_str(), (int)t_s.length());
				}
#else
				SDL_RWops *file = SDL_RWFromFile(save_filename(slot, "auto").c_str(), "wb");
				if (file) {
					if ((int)SDL_RWwrite(file, bytes, 1, sz) != sz) {
						success = false;
					}
					SDL_RWclose(file);
					Sint64 now = util::file_date(save_filename(slot, "auto").c_str());
					std::string t_s = time_to_string(now);
					fn = fn.substr(0, fn.rfind('.')) + ".txt";
					util::cloud_save(fn, t_s.c_str(), (int)t_s.length());
				}
				else {
					success = false;
				}
#endif
				delete[] bytes;
			}
		}

		for (size_t i = 0; i < overwrite_cloud_save.size(); i++) {
			int slot = overwrite_cloud_save[i];
			int sz;
			char *bytes;
			if ((it = byte_cache.find(save_filename(slot))) != byte_cache.end()) {
				std::pair<std::string, char *> p = *it;
				bytes = p.second;
				sz = size_cache[save_filename(slot)];
				byte_cache.erase(it);
			}
			else {
#ifdef TVOS
				bytes = util::tvos_read_bytes(save_filename(slot), &sz);
#else
				bytes = util::slurp_file_from_filesystem(save_filename(slot), &sz);
#endif
			}
			if (bytes == NULL) {
				success = false;
			}
			else {
				std::string fn = directory + "save" + util::itos(slot+1) + ".dat";
				bool ret = util::cloud_save(fn, bytes, sz);
				delete[] bytes;
				if (ret == false) {
					Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713) + " (" + util::itos(util::cloud_get_error_code()) + ")");
					shim::guis.push_back(gui);
					delete_byte_cache();
					return;
				}
#ifdef TVOS
				util::tvos_touch(save_filename(slot));
#else
				utime(save_filename(slot).c_str(), NULL);
#endif
				Sint64 now = util::file_date(save_filename(slot).c_str());
				std::string t_s = time_to_string(now);
				fn = fn.substr(0, fn.rfind('.')) + ".txt";
				util::cloud_save(fn, t_s.c_str(), (int)t_s.length());
			}
		}

		for (size_t i = 0; i < overwrite_cloud_auto.size(); i++) {
			int slot = overwrite_cloud_auto[i];
			int sz;
			char *bytes;
			if ((it = byte_cache.find(save_filename(slot, "auto"))) != byte_cache.end()) {
				std::pair<std::string, char *> p = *it;
				bytes = p.second;
				sz = size_cache[save_filename(slot, "auto")];
				byte_cache.erase(it);
			}
			else {
#ifdef TVOS
				bytes = util::tvos_read_bytes(save_filename(slot, "auto"), &sz);
#else
				bytes = util::slurp_file_from_filesystem(save_filename(slot, "auto"), &sz);
#endif
			}
			if (bytes == NULL) {
				success = false;
			}
			else {
				std::string fn = directory + "auto" + util::itos(slot+1) + ".dat";
				bool ret = util::cloud_save(fn, bytes, sz);
				delete[] bytes;
				if (ret == false) {
					Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713) + " (" + util::itos(util::cloud_get_error_code()) + ")");
					shim::guis.push_back(gui);
					delete_byte_cache();
					return;
				}
#ifdef TVOS
				util::tvos_touch(save_filename(slot, "auto"));
#else
				utime(save_filename(slot, "auto").c_str(), NULL);
#endif
				Sint64 now = util::file_date(save_filename(slot, "auto").c_str());
				std::string t_s = time_to_string(now);
				fn = fn.substr(0, fn.rfind('.')) + ".txt";
				util::cloud_save(fn, t_s.c_str(), (int)t_s.length());
			}
		}

		std::string message;
		if (success) {
			message = GLOBALS->game_t->translate(1719);
		}
		else {
			message = GLOBALS->game_t->translate(1713);
		}
		Notification_GUI *gui = new Notification_GUI(message);
		shim::guis.push_back(gui);

		util::cloud_synchronise();
	}
	
	delete_byte_cache();
}

static void sync()
{
	gfx::clear(shim::black);
	show_notice(GLOBALS->game_t->translate(1721), true);

	std::string directory = DIRECTORY;

	/*
	if (directory != "") {
		std::string dir;
		if (directory[directory.length()-1] == '/') {
			dir = directory.substr(0, directory.length()-1);
		}
		else {
			dir = directory;
		}
		if (util::cloud_mkdir(dir) == false) {
			Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713) + " (mkdir)");
			shim::guis.push_back(gui);
			return;
		}
	}
	*/

	std::vector<Sint64> save_dates;
	std::vector<Sint64> auto_dates;
	std::vector<Sint64> cloud_save_dates;
	std::vector<Sint64> cloud_auto_dates;
	
	for (int i = 0; i < 5; i++) {
		Sint64 d1, d2, d3, d4;

		d1 = util::file_date(save_filename(i));
		d2 = util::file_date(save_filename(i, "auto"));

		d3 = util::cloud_date(directory + "save" + util::itos(i+1) + ".dat");
		int err;
		if ((err = util::cloud_get_error_code()) != 0) {
			Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713) + " (" + util::itos(err) + ")");
			shim::guis.push_back(gui);
			return;
		}

		d4 = util::cloud_date(directory + "auto" + util::itos(i+1) + ".dat");
		if ((err = util::cloud_get_error_code()) != 0) {
			Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713) + " (" + util::itos(err) + ")");
			shim::guis.push_back(gui);
			return;
		}

		if (d1 != d3 && d1 != -1 && d3 != -1) {
			int sz1;
#ifdef TVOS
			char *bytes1 = util::tvos_read_bytes(save_filename(i), &sz1);
#else
			char *bytes1 = util::slurp_file_from_filesystem(save_filename(i), &sz1);
#endif
			if (bytes1 != NULL) {
				int sz2;
				std::string fn = "save" + util::itos(i+1) + ".dat";
				char *bytes2 = util::cloud_read(fn, &sz2);
				if (bytes2 == NULL) {
					delete[] bytes1;
				}
				else {
					std::string md5_1 = util::md5(bytes1, sz1);
					std::string md5_2 = util::md5(bytes2, sz2);
					if (md5_1 == md5_2) {
						d3 = d1;
					}
					byte_cache[save_filename(i)] = bytes1;
					byte_cache[fn] = bytes2;
					size_cache[save_filename(i)] = sz1;
					size_cache[fn] = sz2;
				}
			}
		}

		if (d2 != d4 && d2 != -1 && d4 != -1) {
			int sz1;
#ifdef TVOS
			char *bytes1 = util::tvos_read_bytes(save_filename(i, "auto"), &sz1);
#else
			char *bytes1 = util::slurp_file_from_filesystem(save_filename(i, "auto"), &sz1);
#endif
			if (bytes1 != NULL) {
				int sz2;
				std::string fn = "auto" + util::itos(i+1) + ".dat";
				char *bytes2 = util::cloud_read(fn, &sz2);
				if (bytes2 == NULL) {
					delete[] bytes1;
				}
				else {
					std::string md5_1 = util::md5(bytes1, sz1);
					std::string md5_2 = util::md5(bytes2, sz2);
					if (md5_1 == md5_2) {
						d4 = d2;
					}
					byte_cache[save_filename(i, "auto")] = bytes1;
					byte_cache[fn] = bytes2;
					size_cache[save_filename(i, "auto")] = sz1;
					size_cache[fn] = sz2;
				}
			}
		}

		save_dates.push_back(d1);
		auto_dates.push_back(d2);
		cloud_save_dates.push_back(d3);
		cloud_auto_dates.push_back(d4);
	}

	overwrite_local_save.clear();
	overwrite_cloud_save.clear();
	overwrite_local_auto.clear();
	overwrite_cloud_auto.clear();

	for (int i = 0; i < 5; i++) {
		if (save_dates[i] > cloud_save_dates[i]) {
			overwrite_cloud_save.push_back(i);
		}
		else if (save_dates[i] < cloud_save_dates[i]) {
			overwrite_local_save.push_back(i);
		}

		if (auto_dates[i] > cloud_auto_dates[i]) {
			overwrite_cloud_auto.push_back(i);
		}
		else if (auto_dates[i] < cloud_auto_dates[i]) {
			overwrite_local_auto.push_back(i);
		}
	}

	if (overwrite_local_save.size() == 0 && overwrite_local_auto.size() == 0 && overwrite_cloud_save.size() == 0 && overwrite_cloud_auto.size() == 0) {
		Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1720));
		shim::guis.push_back(gui);
		delete_byte_cache();
		return;
	}

	std::string message = util::string_printf(GLOBALS->game_t->translate(1712).c_str(), overwrite_local_save.size()+overwrite_local_auto.size(), overwrite_cloud_save.size()+overwrite_cloud_auto.size());

	Yes_No_GUI *gui = new Yes_No_GUI(message, true, real_sync_callback);
	gui->set_selected(false);
	shim::guis.push_back(gui);
}

static void erase_callback(void *data)
{
	gfx::clear(shim::black);
	show_notice(GLOBALS->game_t->translate(1721), true);

	std::string directory = DIRECTORY;

	Yes_No_GUI::Callback_Data *d = static_cast<Yes_No_GUI::Callback_Data *>(data);
	if (d->cancelled == false && d->choice == true) {
		for (int i = 0; i < 5; i++) {
			std::vector<std::string> fn;

			fn.push_back(directory + "save" + util::itos(i+1) + ".dat");
			fn.push_back(directory + "auto" + util::itos(i+1) + ".dat");
			fn.push_back(directory + "save" + util::itos(i+1) + ".txt");
			fn.push_back(directory + "auto" + util::itos(i+1) + ".txt");

			for (size_t i = 0; i < fn.size(); i++) {
				if (util::cloud_delete(fn[i]) == false) {
					int err = util::cloud_get_error_code();
					if (err != 0) {
						Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713) + " (" + util::itos(err) + ")");
						shim::guis.push_back(gui);
						return;
					}
				}
			}
		}

		Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1719));
		shim::guis.push_back(gui);
	}
}

static void sync_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = static_cast<Yes_No_GUI::Callback_Data *>(data);
	if (d->cancelled || d->choice == false) {
		Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(1718), true, erase_callback, 0, true);
		gui->set_selected(false);
		shim::guis.push_back(gui);
	}
	else {
		sync();
	}
}
#endif
	
bool Settings_GUI::unset_b3_b4;
bool Settings_GUI::b3_enabled;
bool Settings_GUI::b4_enabled;

void Settings_GUI::static_start()
{
	unset_b3_b4 = false;
	b3_enabled = false;
	b4_enabled = false;
#ifdef ANDROID
	android_bool = true;
#endif
}

Settings_GUI::Settings_GUI(bool fade_in, Setting selected, int top) :
	changed_to_another_settings_gui(false)
{
	if (settings_active() == false) {
		unset_b3_b4 = true;
		b3_enabled = wedge::get_onscreen_controller_b3_enabled();
		b4_enabled = wedge::get_onscreen_controller_b4_enabled();
		wedge::set_onscreen_controller_b3_enabled(false, -1);
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}

	if (AREA && !MENU) {
		Uint32 pause_start_time = GET_TICKS();
		Uint32 played_time = pause_start_time - INSTANCE->play_start;
		INSTANCE->play_time += (played_time / 1000);
	}

#if defined ANDROID || defined IOS || defined RASPBERRYPI
	video_enabled = false;
#else
	video_enabled = true;
#endif
	
#if defined IOS
	keyboard_enabled = false;
#else
	keyboard_enabled = true;
#endif
	joystick_enabled = true;

	fading_in = fade_in;

	caption = GLOBALS->game_t->translate(1517)/* Originally: Settings */;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	list = new Widget_List(0.9f, shim::font->get_height() * 5);
	list->set_centre_x(true);
	list->set_centre_y(true);
	list->set_padding_top(int(GLOBALS->bold_font->get_height()));
	list->set_text_shadow_colour(shim::black);
	list->set_arrow_colour(shim::palette[23]);
	list->set_arrow_shadow_colour(shim::black);
	list->set_parent(modal_main_widget);

	list->set_disabled_text_colour(shim::palette[23]);
	list->set_disabled_text_shadow_colour(shim::transparent);

	std::vector<std::string> items;
	items.push_back(GLOBALS->game_t->translate(1518)/* Originally: Language Settings */);
	if (video_enabled) {
		items.push_back(GLOBALS->game_t->translate(1519)/* Originally: Video Settings */);
	}
	items.push_back(GLOBALS->game_t->translate(1328)/* Originally: Audio Settings */);
	items.push_back(GLOBALS->game_t->translate(1506)/* Originally: Other Settings */);
	if (keyboard_enabled) {
		items.push_back(GLOBALS->game_t->translate(1376)/* Originally: Keyboard Controls */);
	}
	items.push_back(GLOBALS->game_t->translate(1377)/* Originally: Joystick Controls */);

#if defined GOOGLE_PLAY || defined AMAZON || defined IOS || defined MAS
#ifdef AMAZON_XXX
	if (amazon_is_fire_tv() == false)
#endif
	{
		items.push_back(GLOBALS->game_t->translate(1711)/* Originally: Achievements */);
	}
#if !defined GOOGLE_PLAY
	items.push_back(GLOBALS->game_t->translate(1716)/* Originally: Sync Cloud Saves*/);
#endif
#endif

#ifndef RASPBERRYPI // this is the console version, there is no way to show a license over top of the game...
	//items.push_back(GLOBALS->game_t->translate(1723)/* Originally: Manual */);
	items.push_back(GLOBALS->game_t->translate(1524)/* Originally: 3rd Party Licenses */);
#endif

	list->set_items(items);

	int sel = (int)selected;
	int sub = 0;
	if (video_enabled == false) {
		if (sel > 1) {
			sub++;
		}
	}
	if (keyboard_enabled == false) {
		if (sel > 4) {
			sub++;
		}
	}
	if (joystick_enabled == false) {
		if (sel > 5) {
			sub++;
		}
	}
	sel -= sub;
	list->set_selected(sel);
	list->set_top(top);

	if (AREA != NULL) {
		list->set_disabled(0, true);
	}

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Settings_GUI::~Settings_GUI()
{
	if (unset_b3_b4 && changed_to_another_settings_gui == false) {
		unset_b3_b4 = false;
		wedge::set_onscreen_controller_b3_enabled(b3_enabled, M3_GLOBALS->key_switch);
		// b4 could be disabled by Other_Settings_GUI (Hide onscreen settings button)
		if (M3_GLOBALS->hide_onscreen_settings_button == false && b4_enabled == true) {
			wedge::set_onscreen_controller_b4_enabled(b4_enabled, M3_GLOBALS->key_b4);
		}
	}
}

void Settings_GUI::handle_event(TGUI_Event *event)
{
	if (fading_in || fading_out) {
		return;
	}

	if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2 && event->keyboard.is_repeat == false) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2 && event->joystick.is_repeat == false)
	) {
		M3_GLOBALS->button->play(false);
		fading_out = true;
		fade_start = GET_TICKS();
		if (AREA && !MENU) {
			INSTANCE->play_start = GET_TICKS();
		}
	}
	else {
		gui::GUI::handle_event(event);
	}
}

void Settings_GUI::update()
{
	Sliding_Menu_GUI::update();

	if (fading_in || fading_out) {
		return;
	}

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
#ifdef AMAZON_XXX
		if (amazon_is_fire_tv() && pressed >= 6) {
			pressed++;
		}
#endif

		if (video_enabled == false) {
			if (pressed > 0) {
				pressed++;
			}
		}
		if (keyboard_enabled == false) {
			if (pressed > 3) {
				pressed++;
			}
		}
		if (joystick_enabled == false) {
			if (pressed > 4) {
				pressed++;
			}
		}
		if (pressed == 0) {
			Language_Settings_GUI *gui = new Language_Settings_GUI();
			shim::guis.push_back(gui);
			changed_to_another_settings_gui = true;
		}
		else if (pressed == 1) {
			Video_Settings_GUI *gui = new Video_Settings_GUI();
			shim::guis.push_back(gui);
			changed_to_another_settings_gui = true;
		}
		else if (pressed == 2) {
			Audio_Settings_GUI *gui = new Audio_Settings_GUI();
			shim::guis.push_back(gui);
			changed_to_another_settings_gui = true;
		}
		else if (pressed == 3) {
			Other_Settings_GUI *gui = new Other_Settings_GUI();
			shim::guis.push_back(gui);
			changed_to_another_settings_gui = true;
		}
		else if (pressed == 4) {
			Controls_Settings_GUI *gui = new Controls_Settings_GUI(true);
			shim::guis.push_back(gui);
			changed_to_another_settings_gui = true;
		}
		else if (pressed == 5) {
			Controls_Settings_GUI *gui = new Controls_Settings_GUI(false);
			shim::guis.push_back(gui);
			changed_to_another_settings_gui = true;
		}
#if defined GOOGLE_PLAY || defined AMAZON || defined IOS || defined MAS
		else if (pressed == 6) {
#ifdef AMAZON
			if (amazon_initialised() == false) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1715));
				shim::guis.push_back(gui);
			}
			else
#endif
			{
#if defined GOOGLE_PLAY || defined AMAZON
				achievements_bool = false;
#ifdef AMAZON
				SDL_Colour black;
				black.r = 0;
				black.g = 0;
				black.b = 0;
				black.a = 255;
				gfx::clear(black);
				gfx::flip();
#endif
#endif
				if (util::show_achievements() == false) {
					Notification_GUI *gui;
#ifdef AMAZON
					if (util::cloud_get_error_code() == 4) {
						gui = new Notification_GUI(amazon_errmsg());
					}
					else
#endif
					{
						gui = new Notification_GUI(GLOBALS->game_t->translate(1713));
					}
					shim::guis.push_back(gui);
				}

#if defined GOOGLE_PLAY || defined AMAZON
				while (achievements_bool == false) {
					SDL_Delay(10);
				}
				gfx::resize_window(shim::real_screen_size.w, shim::real_screen_size.h);
#endif
			}
		}
#if !defined GOOGLE_PLAY
		else if (pressed == 7) {
#if defined IOS || defined MAS
			if (apple_is_logged_into_icloud() == false) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1722));
				shim::guis.push_back(gui);
				return;
			}
#endif
			util::cloud_synchronise();
			Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(1717), true, sync_callback);
			gui->set_selected(true);
			shim::guis.push_back(gui);
		}
// this was commented out when manual removed, numbers for licenses adjusted below
// these two endifs the next lines were added
#endif
#endif
/*
		else if (pressed == 8) {
#else
		else if (pressed == 7) {
#endif
#else
		else if (pressed == 6) {
#endif
#ifdef ANDROID
			if (android_bool == true) {
				android_bool = false;

				int result = 1;

				android_mutex = SDL_CreateMutex();

				if (android_mutex) {
					android_cond = SDL_CreateCond();

					if (android_cond) {
						JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
						jobject activity = (jobject)SDL_AndroidGetActivity();
						jclass clazz(env->GetObjectClass(activity));

						jstring S = env->NewStringUTF(util::lowercase(GLOBALS->language).c_str());

						jmethodID method_id = env->GetMethodID(clazz, "showManual", "(Ljava/lang/String;)V");

						env->CallVoidMethod(activity, method_id, S);

						env->DeleteLocalRef(S);

						env->DeleteLocalRef(activity);
						env->DeleteLocalRef(clazz);

						float old_volume = shim::music->get_master_volume();

						shim::music->pause();

						SDL_LockMutex(android_mutex);
						while (!android_bool) {
							SDL_CondWait(android_cond, android_mutex);
						}
						SDL_UnlockMutex(android_mutex);

						SDL_DestroyCond(android_cond);

						result = get_show_manual_result();
						
						shim::music->play(old_volume, true);
					}

					SDL_DestroyMutex(android_mutex);
				}

				if (result == 1) {
					Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713));
					shim::guis.push_back(gui);
				}
			}
#elif defined IOS
			shim::music->pause();
			bool ret = ios_show_manual();
			if (ret == false) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1713));
				shim::guis.push_back(gui);
			}
#else
#if defined __APPLE__
			char *base = SDL_GetBasePath();
			std::string filename = std::string(base) + "manual_" + util::lowercase(GLOBALS->language) + ".html";
			SDL_free(base);
#elif defined __linux__
			std::string argv0 = shim::argv[0];
			size_t slash = argv0.rfind("/");
			std::string filename = argv0.substr(0, slash) + "/" + "manual_" + util::lowercase(GLOBALS->language) + ".html";
			filename = std::string("\"") + filename + std::string("\"");
#else
			std::string filename = "manual_" + util::lowercase(GLOBALS->language) + ".html";
#endif
			util::open_with_system(filename);
#endif
		}
#if defined AMAZON || defined IOS || defined MAS
		else if (pressed == 9) {
#elif defined GOOGLE_PLAY
		else if (pressed == 8) {
#else
		else if (pressed == 7) {
#endif
*/
#if defined AMAZON || defined IOS || defined MAS
		else if (pressed == 8) {
#elif defined GOOGLE_PLAY
		else if (pressed == 7) {
#else
		else if (pressed == 6) {
#endif
#ifdef ANDROID
			if (android_bool == true) {
				android_bool = false;

				int result = 1;

				android_mutex = SDL_CreateMutex();

				if (android_mutex) {
					android_cond = SDL_CreateCond();

					if (android_cond) {
						JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
						jobject activity = (jobject)SDL_AndroidGetActivity();
						jclass clazz(env->GetObjectClass(activity));

						jmethodID method_id = env->GetMethodID(clazz, "showLicense", "()V");

						env->CallVoidMethod(activity, method_id);

						env->DeleteLocalRef(activity);
						env->DeleteLocalRef(clazz);

						float old_volume = shim::music->get_master_volume();

						shim::music->pause();

						SDL_LockMutex(android_mutex);
						while (!android_bool) {
							SDL_CondWait(android_cond, android_mutex);
						}
						SDL_UnlockMutex(android_mutex);

						SDL_DestroyCond(android_cond);

						result = get_show_license_result();
						
						shim::music->play(old_volume, true);
					}

					SDL_DestroyMutex(android_mutex);
				}

				if (result == 1) {
					Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1714));
					shim::guis.push_back(gui);
				}
			}
#elif defined IOS
			shim::music->pause();
			bool ret = ios_show_license();
			if (ret == false) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1714));
				shim::guis.push_back(gui);
			}
#else
#if defined __APPLE__
			char *base = SDL_GetBasePath();
			std::string filename = std::string(base) + "3rd_party.html";
			SDL_free(base);
#elif defined __linux__
			std::string argv0 = shim::argv[0];
			size_t slash = argv0.rfind("/");
			std::string filename = argv0.substr(0, slash) + "/" + "3rd_party.html";
			filename = std::string("\"") + filename + std::string("\"");
#else
			std::string filename = "3rd_party.html";
#endif
			util::open_with_system(filename);
#endif
		}
	}
}
