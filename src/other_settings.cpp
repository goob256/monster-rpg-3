#ifdef ANDROID
#include <jni.h>
#endif

#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/onscreen_controller.h>

#include "other_settings.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "widgets.h"

static void callback(void *data)
{
	shim::use_hires_font = !shim::use_hires_font;
	M3_GLOBALS->reload_fonts();

	for (int i = MAX(1, (int)shim::guis.size())-1; i >= 0; i--) {
		gui::GUI *gui = shim::guis[i];
		Title_GUI *t;
		Notification_GUI *n;
		if ((t = dynamic_cast<Title_GUI *>(gui)) != NULL) {
			t->reload_text();
		}
		else if ((n = dynamic_cast<Notification_GUI *>(gui)) != NULL) {
			//n->set_font(GLOBALS->bold_font);
			// don't need this now
		}
		else {
			gui->exit();
		}
	}
}

Other_Settings_GUI::Other_Settings_GUI()
{
	transition = true;
	transition_is_enlarge = true;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	//modal_main_widget->set_background_colour(shim::palette[24]);

	Widget *container = new Widget();
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_parent(modal_main_widget);

#if !defined IOS || defined TVOS
	safe_mode_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(1502)/* Originally: TV safe mode */, M3_GLOBALS->tv_safe_mode);
	safe_mode_checkbox->set_centre_x(true);
	safe_mode_checkbox->set_parent(container);
	safe_mode_checkbox->set_padding_bottom(2);
	safe_mode_checkbox->set_padding_top(5);
#else
	safe_mode_checkbox = NULL;
#endif

	bool go = util::system_has_touchscreen();

	if (go) {
		onscreen_controller_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(1503)/* Originally: Use on-screen controller */, GLOBALS->onscreen_controller_was_enabled);
		onscreen_controller_checkbox->set_centre_x(true);
		onscreen_controller_checkbox->set_parent(container);
		onscreen_controller_checkbox->set_padding_bottom(2);
		onscreen_controller_checkbox->set_break_line(true);
		onscreen_controller_checkbox->set_clear_float_x(true);
	}
	else {
		onscreen_controller_checkbox = NULL;
	}

#ifndef TVOS
#ifdef ANDROID
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "has_vibrator", "()Z");

	bool has_vibrator = (bool)env->CallBooleanMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	if (has_vibrator) {
#endif
		rumble_enabled_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(1504)/* Originally: Rumble enabled */, GLOBALS->rumble_enabled);
		rumble_enabled_checkbox->set_centre_x(true);
		rumble_enabled_checkbox->set_parent(container);
		rumble_enabled_checkbox->set_break_line(true);
		rumble_enabled_checkbox->set_padding_bottom(2);
		rumble_enabled_checkbox->set_clear_float_x(true);
#ifdef ANDROID
	}
	else {
		rumble_enabled_checkbox = NULL;
	}
#endif
#else
	rumble_enabled_checkbox = NULL;
#endif

#if 1 // don't use this anymore, can be problematic if player hides it (maybe not knowing better) and then needs it
	hide_onscreen_settings_button_checkbox = NULL;
#else
#ifdef TVOS
	hide_onscreen_settings_button_checkbox = NULL;
#else
	hide_onscreen_settings_button_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(1505)/* Originally: Hide settings button */, M3_GLOBALS->hide_onscreen_settings_button);
	hide_onscreen_settings_button_checkbox->set_centre_x(true);
	hide_onscreen_settings_button_checkbox->set_parent(container);
	hide_onscreen_settings_button_checkbox->set_break_line(true);
	hide_onscreen_settings_button_checkbox->set_clear_float_x(true);
#endif
#endif

	hires_font_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(1754)/* Originally: Use alternative font */, shim::use_hires_font);
	hires_font_checkbox->set_break_line(true);
	hires_font_checkbox->set_clear_float_x(true);
	hires_font_checkbox->set_centre_x(true);
	hires_font_checkbox->set_parent(container);
	hires_font_checkbox->set_padding_bottom(2);
	if (AREA != NULL) {
		hires_font_checkbox->set_disabled(true);
	}

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Other_Settings_GUI::~Other_Settings_GUI()
{
}

void Other_Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)) {
		exit();
	}
	else {
		bool onscreen_controller_checked;
		bool safe_mode_checked;
		bool rumble_enabled_checked;
		if (onscreen_controller_checkbox != NULL) {
			onscreen_controller_checked = onscreen_controller_checkbox->is_checked();
		}
		else {
			onscreen_controller_checked = false;
		}
		if (safe_mode_checkbox != NULL) {
			safe_mode_checked = safe_mode_checkbox->is_checked();
		}
		else {
			safe_mode_checked = false;
		}
		if (rumble_enabled_checkbox != NULL) {
			rumble_enabled_checked = rumble_enabled_checkbox->is_checked();
		}
		else {
			rumble_enabled_checked = false;
		}
		gui::GUI::handle_event(event);
		if (onscreen_controller_checkbox != NULL) {
			if (onscreen_controller_checkbox->is_checked() != onscreen_controller_checked) {
				onscreen_controller_checked = !onscreen_controller_checked;
				//wedge::enable_onscreen_controller(onscreen_controller_checked);
				GLOBALS->onscreen_controller_was_enabled = onscreen_controller_checked;
			}
		}
		if (safe_mode_checkbox != NULL) {
			if (safe_mode_checkbox->is_checked() != safe_mode_checked) {
				safe_mode_checked = !safe_mode_checked;
				apply_tv_safe_mode(safe_mode_checked);
			}
		}
		if (rumble_enabled_checkbox != NULL) {
			if (rumble_enabled_checkbox->is_checked() != rumble_enabled_checked) {
				rumble_enabled_checked = !rumble_enabled_checked;
				GLOBALS->rumble_enabled = rumble_enabled_checked;
				if (rumble_enabled_checked) {
					wedge::rumble(1.0f, 1000);
				}
			}
		}
		if (hires_font_checkbox != NULL) {
			bool b = hires_font_checkbox->is_checked();
			if (b != shim::use_hires_font) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1593)/* Originally: This menu will now exit. */, callback);
				shim::guis.push_back(gui);
			}
		}
		// onscreen settings button doesn't need any special handling
	}
}

void Other_Settings_GUI::draw_fore()
{
	GLOBALS->bold_font->enable_shadow(shim::black, gfx::Font::DROP_SHADOW);
	GLOBALS->bold_font->draw(shim::white, GLOBALS->game_t->translate(1506)/* Originally: Other Settings */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	GLOBALS->bold_font->disable_shadow();
	
	gfx::draw_rectangle(shim::palette[13], util::Point<int>(-1, -1), shim::screen_size+util::Size<int>(2, 2));

	GUI::draw_fore();
}

void Other_Settings_GUI::draw()
{
	gfx::draw_filled_rectangle(shim::palette[24], util::Point<int>(0, 0), shim::screen_size);
	gui::GUI::draw();
}

void Other_Settings_GUI::transition_in_done()
{
	transition_is_enlarge = false;
	transition_is_shrink = true;
}

void Other_Settings_GUI::exit()
{
	if (onscreen_controller_checkbox != NULL) {
		//wedge::enable_onscreen_controller(onscreen_controller_checkbox->is_checked());
		GLOBALS->onscreen_controller_was_enabled = onscreen_controller_checkbox->is_checked();
	}
	if (safe_mode_checkbox != NULL) {
		M3_GLOBALS->tv_safe_mode = safe_mode_checkbox->is_checked();
	}
	if (hide_onscreen_settings_button_checkbox != NULL) {
		M3_GLOBALS->hide_onscreen_settings_button = hide_onscreen_settings_button_checkbox->is_checked();
	}
	if (rumble_enabled_checkbox != NULL) {
		GLOBALS->rumble_enabled = rumble_enabled_checkbox->is_checked();
	}
	wedge::enable_onscreen_controller(GLOBALS->onscreen_controller_was_enabled);
	save_settings();
	wedge::enable_onscreen_controller(false);
	gui::GUI::exit();
}
