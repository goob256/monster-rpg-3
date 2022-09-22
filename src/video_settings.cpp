#include "general.h"
#include "globals.h"
#include "monster_rpg_3.h"
#include "video_settings.h"

#define SPACE std::string("   ")

Video_Settings_GUI::Video_Settings_GUI()
{
	transition = true;
	transition_is_enlarge = true;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	list = new Widget_List(0.9f, shim::font->get_height() * 5);
	list->set_centre_x(true);
	list->set_centre_y(true);
	list->set_padding_top(int(GLOBALS->bold_font->get_height()));
	list->set_text_shadow_colour(shim::black);
	list->set_arrow_colour(shim::palette[23]);
	list->set_arrow_shadow_colour(shim::black);
	list->set_parent(modal_main_widget);

	list->set_disabled_text_colour(shim::palette[11]);
	list->set_disabled_text_shadow_colour(shim::black);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
	
	set_text();
}

Video_Settings_GUI::~Video_Settings_GUI()
{
}

void Video_Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)) {
		M3_GLOBALS->button->play(false);
		save_settings();
		exit();
	}
	else {
		gui::GUI::handle_event(event);
	}
}

void Video_Settings_GUI::update()
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		int top = list->get_top();
		if (pressed >= 1 && pressed <= (int)windowed_modes.size()) {
			force_windowed = true;
			force_fullscreen = false;
			force_screen_size = windowed_modes[pressed-1];
			delete_shim_args();
			set_shim_args(false); // sets command line params for force_* arguments listed above
			gfx::restart(SCR_W, SCR_H, true);
		}
		else {
			int index = pressed - (int)windowed_modes.size() - 2;
			force_windowed = false;
			force_fullscreen = true;
			force_screen_size = fullscreen_modes[index];
			delete_shim_args();
			set_shim_args(false); // sets command line params for force_* arguments listed above
			gfx::restart(SCR_W, SCR_H, true);
		}
		list->set_selected(pressed);
		list->set_top(top);
	}
}

void Video_Settings_GUI::draw_fore()
{
	GLOBALS->bold_font->enable_shadow(shim::black, gfx::Font::DROP_SHADOW);
	GLOBALS->bold_font->draw(shim::white, GLOBALS->game_t->translate(1519)/* Originally: Video Settings */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	GLOBALS->bold_font->disable_shadow();
	
	gfx::draw_rectangle(shim::palette[13], util::Point<int>(-1, -1), shim::screen_size+util::Size<int>(2, 2));

	GUI::draw_fore();
}

void Video_Settings_GUI::draw()
{
	gfx::draw_filled_rectangle(shim::palette[24], util::Point<int>(0, 0), shim::screen_size);
	int x = list->get_x();
	int y = list->get_y();
	int w = list->get_width();
	int h = list->get_height();
	gfx::draw_filled_rectangle(shim::palette[25], util::Point<int>(x, y), util::Size<int>(w, h));
	gui::GUI::draw();
}

void Video_Settings_GUI::set_text()
{
	int curr_sz = (int)list->get_items().size();
	for (int i = 0; i < curr_sz; i++) {
		list->set_disabled(i, false);
	}

	std::vector<std::string> items;
	items.push_back(GLOBALS->game_t->translate(1544)/* Originally: Windowed Modes: */);

	int max = gfx::get_max_comfortable_scale(util::Size<int>(SCR_W, SCR_H));

	windowed_modes.clear();

	windowed_modes.push_back(util::Size<int>(5120, 2880));
	windowed_modes.push_back(util::Size<int>(3840, 2160));
	windowed_modes.push_back(util::Size<int>(2560, 1440));
	windowed_modes.push_back(util::Size<int>(1920, 1080));
	windowed_modes.push_back(util::Size<int>(1440, 810));
	windowed_modes.push_back(util::Size<int>(1280, 720));
	windowed_modes.push_back(util::Size<int>(960, 540));
	windowed_modes.push_back(util::Size<int>(640, 360));

	while (windowed_modes[0].w > max*SCR_W || windowed_modes[0].h > max*SCR_H) {
		windowed_modes.erase(windowed_modes.begin());
	}

	for (size_t i = 0; i < windowed_modes.size(); i++) {
		items.push_back(SPACE + util::itos(windowed_modes[i].w) + "x" + util::itos(windowed_modes[i].h));
	}

	items.push_back(GLOBALS->game_t->translate(1545)/* Originally: Fullscreen Modes: */);

	fullscreen_modes = gfx::get_supported_video_modes();

	std::vector< util::Size<int> >::iterator it;
	for (it = fullscreen_modes.begin(); it != fullscreen_modes.end();) {
		util::Size<int> size = *it;
		if (size.w < SCR_W * 4 || size.h < SCR_H * 4) {
			it = fullscreen_modes.erase(it);
		}
		else {
			it++;
		}
	}

	for (size_t i = 0; i < fullscreen_modes.size(); i++) {
		items.push_back("   " + util::itos(fullscreen_modes[i].w) + "x" + util::itos(fullscreen_modes[i].h));
	}

	list->set_items(items);

	list->set_disabled(0, true);
	list->set_disabled((int)windowed_modes.size()+1, true);

	set_selected();
}

void Video_Settings_GUI::found_device()
{
	set_text();
}

void Video_Settings_GUI::transition_in_done()
{
	transition_is_enlarge = false;
	transition_is_shrink = true;
}

void Video_Settings_GUI::set_selected()
{
	std::vector<std::string> items = list->get_items();

	std::string curr_mode = SPACE + util::itos(shim::real_screen_size.w) + "x" + util::itos(shim::real_screen_size.h);
	int index = 0;
	bool fs = gfx::is_real_fullscreen();
	int start = fs ? (int)windowed_modes.size() + 2 : 1;
	int end = fs ? (int)items.size() : (int)windowed_modes.size() + 1;
	for (int i = start; i < end; i++) {
		if (items[i] == curr_mode) {
			index = i;
			break;
		}
	}
	list->set_selected(index);
	int vr = MIN(list->visible_rows(), (int)items.size());
	list->set_top(MIN((int)items.size()-vr, MAX(0, index-vr/2)));
}
