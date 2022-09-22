#include "controls_settings.h"
#include "dialogue.h"
#include "general.h"
#include "globals.h"
#include "gui.h"

static void apply_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = static_cast<Yes_No_GUI::Callback_Data *>(data);
	Controls_Settings_GUI *gui = static_cast<Controls_Settings_GUI *>(d->userdata);
	if (d->cancelled == false && d->choice) {
		gui->quit(true);
	}
	else if (d->cancelled == false) {
		gui->quit(false);
	}
}

static void duplicates_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = static_cast<Yes_No_GUI::Callback_Data *>(data);
	Controls_Settings_GUI *gui = static_cast<Controls_Settings_GUI *>(d->userdata);
	if (d->cancelled == false && d->choice) {
		gui->quit(false);
	}
}

Controls_Settings_GUI::Controls_Settings_GUI(bool keyboard) :
	keyboard(keyboard),
	assigning(false)
{
	transition = true;
	transition_is_enlarge = true;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	list = new Widget_Controls_List(0.9f, shim::font->get_height() * 5);
	list->set_centre_x(true);
	list->set_centre_y(true);
	list->set_padding_top(int(GLOBALS->bold_font->get_height()));
	list->set_text_shadow_colour(shim::black);
	list->set_arrow_colour(shim::palette[23]);
	list->set_arrow_shadow_colour(shim::black);
	list->set_parent(modal_main_widget);

	if (keyboard) {
		controls[B1] = GLOBALS->key_b1;
		controls[B2] = GLOBALS->key_b2;
		controls[B3] = GLOBALS->key_b3;
		controls[B4] = M3_GLOBALS->key_b4;
		controls[SWITCH] = M3_GLOBALS->key_switch;
		controls[FS] = shim::fullscreen_key;
		controls[L] = GLOBALS->key_l;
		controls[R] = GLOBALS->key_r;
		controls[U] = GLOBALS->key_u;
		controls[D] = GLOBALS->key_d;
	}
	else {
		controls[B1] = GLOBALS->joy_b1;
		controls[B2] = GLOBALS->joy_b2;
		controls[B3] = GLOBALS->joy_b3;
		controls[B4] = GLOBALS->joy_b4;
		controls[SWITCH] = GLOBALS->joy_switch;
	}

	set_text();

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Controls_Settings_GUI::~Controls_Settings_GUI()
{
}

void Controls_Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if (assigning) {
		input::convert_focus_to_original(event);
		if (keyboard) {
			if (event->type == TGUI_KEY_DOWN) {
				M3_GLOBALS->button->play(false);
				controls[which_assigning] = event->keyboard.code;
				set_text();
				list->set_selected((int)which_assigning);
				gui->set_focus(list);
				assigning = false;
			}
			else if (event->type == TGUI_MOUSE_DOWN || event->type == TGUI_JOY_DOWN) {
				M3_GLOBALS->button->play(false);
				gui->set_focus(list);
				assigning = false;
			}
		}
		else {
			if (event->type == TGUI_JOY_DOWN) {
				if (
					shim::convert_xbox_dpad_to_arrows == false ||
					(event->joystick.button != shim::xbox_l &&
					event->joystick.button != shim::xbox_r &&
					event->joystick.button != shim::xbox_u &&
					event->joystick.button != shim::xbox_d)
				) {
					M3_GLOBALS->button->play(false);
					controls[which_assigning] = event->joystick.button;
					set_text();
					list->set_selected((int)which_assigning);
					gui->set_focus(list);
					assigning = false;
				}
			}
			else if (event->type == TGUI_MOUSE_DOWN || event->type == TGUI_KEY_DOWN) {
				M3_GLOBALS->button->play(false);
				gui->set_focus(list);
				assigning = false;
			}
		}
		if (assigning == false) {
			shim::convert_directions_to_focus_events = true;
		}
	}
	else {
		if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)) {
			M3_GLOBALS->button->play(false);
			bool changed = false;
			if (keyboard) {
				changed |= controls[B1] != GLOBALS->key_b1;
				changed |= controls[B2] != GLOBALS->key_b2;
				changed |= controls[B3] != GLOBALS->key_b3;
				changed |= controls[B4] != M3_GLOBALS->key_b4;
				changed |= controls[SWITCH] != M3_GLOBALS->key_switch;
				changed |= controls[FS] != shim::fullscreen_key;
				changed |= controls[L] != GLOBALS->key_l;
				changed |= controls[R] != GLOBALS->key_r;
				changed |= controls[U] != GLOBALS->key_u;
				changed |= controls[D] != GLOBALS->key_d;
			}
			else {
				changed |= controls[B1] != GLOBALS->joy_b1;
				changed |= controls[B2] != GLOBALS->joy_b2;
				changed |= controls[B3] != GLOBALS->joy_b3;
				changed |= controls[B4] != GLOBALS->joy_b4;
				changed |= controls[SWITCH] != GLOBALS->joy_switch;
			}
			if (changed == false) {
				save_settings();
				exit();
			}
			else {
				Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(1373)/* Originally: Apply changes? */, true, apply_callback, this);
				shim::guis.push_back(gui);
			}
		}
		else {
			gui::GUI::handle_event(event);
		}
	}
}

void Controls_Settings_GUI::update()
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		assigning = true;
		shim::convert_directions_to_focus_events = false;
		which_assigning = (Control)pressed;
		gui->set_focus(NULL);
	}
}

void Controls_Settings_GUI::draw_fore()
{
	if (assigning) {
		std::string text;

		if (keyboard) {
			text = GLOBALS->game_t->translate(1374)/* Originally: Press a key... */;
		}
		else {
			text = GLOBALS->game_t->translate(1375)/* Originally: Press a button... */;
		}

		show_notice(text);
	}
	else {
		GLOBALS->bold_font->enable_shadow(shim::black, gfx::Font::DROP_SHADOW);
		std::string menu_name = keyboard ? GLOBALS->game_t->translate(1376)/* Originally: Keyboard Controls */ : GLOBALS->game_t->translate(1377)/* Originally: Joystick Controls */;
		GLOBALS->bold_font->draw(shim::white, menu_name, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
		GLOBALS->bold_font->disable_shadow();
	}
	
	gfx::draw_rectangle(shim::palette[13], util::Point<int>(-1, -1), shim::screen_size+util::Size<int>(2, 2));

	GUI::draw_fore();
}

void Controls_Settings_GUI::draw()
{
	gfx::draw_filled_rectangle(shim::palette[24], util::Point<int>(0, 0), shim::screen_size);
	int x = list->get_x();
	int y = list->get_y();
	int w = list->get_width();
	int h = list->get_height();
	gfx::draw_filled_rectangle(shim::palette[25], util::Point<int>(x, y), util::Size<int>(w, h));
	gui::GUI::draw();
}

void Controls_Settings_GUI::set_text()
{
	std::vector<std::string> names;
	std::vector<std::string> assignments;

	if (keyboard) {
		names.push_back("Action");
		names.push_back("Back/Menu");
		names.push_back("Examine");
		names.push_back("Settings");
		names.push_back("Next Character");
		names.push_back("Fullscreen");
		names.push_back("Left");
		names.push_back("Right");
		names.push_back("Up");
		names.push_back("Down");

		assignments.push_back(get_key_name(controls[B1]));
		assignments.push_back(get_key_name(controls[B2]));
		assignments.push_back(get_key_name(controls[B3]));
		assignments.push_back(get_key_name(controls[B4]));
		assignments.push_back(get_key_name(controls[SWITCH]));
		assignments.push_back(get_key_name(controls[FS]));
		assignments.push_back(get_key_name(controls[L]));
		assignments.push_back(get_key_name(controls[R]));
		assignments.push_back(get_key_name(controls[U]));
		assignments.push_back(get_key_name(controls[D]));
	}
	else {
		names.push_back("Action");
		names.push_back("Back/Menu");
		names.push_back("Examine");
		names.push_back("Settings");
		names.push_back("Next Character");

		assignments.push_back(get_joystick_button_name(controls[B1]));
		assignments.push_back(get_joystick_button_name(controls[B2]));
		assignments.push_back(get_joystick_button_name(controls[B3]));
		assignments.push_back(get_joystick_button_name(controls[B4]));
		assignments.push_back(get_joystick_button_name(controls[SWITCH]));
	}

	list->set_items(names, assignments);
}
	
std::string Controls_Settings_GUI::get_key_name(int code)
{
	if (code < 0) {
		return "";
	}
	else {
		return ::get_key_name(code);
	}
}

void Controls_Settings_GUI::quit(bool apply)
{
	if (apply) {
		bool same = false;
		int max = keyboard ? CONTROL_SIZE : CONTROL_SIZE-5;
		for (int i = 0; i < max; i++) {
			for (int j = i+1; j < max; j++) {
				if (controls[i] == controls[j]) {
					same = true;
					break;
				}
			}
			if (same) {
				break;
			}
		}
		if (same) {
			Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(1393)/* Originally: Duplicates found! Discard? */, true, duplicates_callback, this);
			shim::guis.push_back(gui);
			return;
		}
		else {
			if (keyboard) {
				GLOBALS->key_b1 = controls[B1];
				GLOBALS->key_b2 = controls[B2];
				GLOBALS->key_b3 = controls[B3];
				M3_GLOBALS->key_b4 = controls[B4];
				M3_GLOBALS->key_switch = controls[SWITCH];
				shim::fullscreen_key = controls[FS];
				GLOBALS->key_l = shim::key_l = controls[L];
				GLOBALS->key_r = shim::key_r = controls[R];
				GLOBALS->key_u = shim::key_u = controls[U];
				GLOBALS->key_d = shim::key_d = controls[D];
			}
			else {
				GLOBALS->joy_b1 = controls[B1];
				GLOBALS->joy_b2 = controls[B2];
				GLOBALS->joy_b3 = controls[B3];
				GLOBALS->joy_b4 = controls[B4];
				GLOBALS->joy_switch = controls[SWITCH];
			}
		}
	}

	save_settings();
	exit();
}

void Controls_Settings_GUI::transition_in_done()
{
	transition_is_enlarge = false;
	transition_is_shrink = true;
}
