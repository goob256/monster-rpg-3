#include "general.h"
#include "globals.h"
#include "gui.h"
#include "language_settings.h"

static void callback(void *data)
{
	for (int i = MAX(2, (int)shim::guis.size())-2; i >= 0; i--) {
		gui::GUI *gui = shim::guis[i];
		Title_GUI *t;
		if ((t = dynamic_cast<Title_GUI *>(gui)) != NULL) {
			t->reload_text();
		}
		else {
			gui->exit();
		}
	}
}

Language_Settings_GUI::Language_Settings_GUI()
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

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
	
	set_text();
}

Language_Settings_GUI::~Language_Settings_GUI()
{
}

void Language_Settings_GUI::handle_event(TGUI_Event *event)
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

void Language_Settings_GUI::update()
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		std::string translated = list->get_item(list->get_selected());
		std::string lang = languages[translated];
		if (lang != GLOBALS->language) {
			GLOBALS->language = lang;
			GLOBALS->load_translation();
			Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(1593)/* Originally: This menu will now exit. */, callback);
			shim::guis.push_back(gui);
			save_settings();
		}
	}
}

void Language_Settings_GUI::draw_fore()
{
	GLOBALS->bold_font->enable_shadow(shim::black, gfx::Font::DROP_SHADOW);
	GLOBALS->bold_font->draw(shim::white, GLOBALS->game_t->translate(1518)/* Originally: Language Settings */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	GLOBALS->bold_font->disable_shadow();
	
	gfx::draw_rectangle(shim::palette[13], util::Point<int>(-1, -1), shim::screen_size+util::Size<int>(2, 2));

	GUI::draw_fore();
}

void Language_Settings_GUI::draw()
{
	gfx::draw_filled_rectangle(shim::palette[24], util::Point<int>(0, 0), shim::screen_size);
	int x = list->get_x();
	int y = list->get_y();
	int w = list->get_width();
	int h = list->get_height();
	gfx::draw_filled_rectangle(shim::palette[25], util::Point<int>(x, y), util::Size<int>(w, h));
	gui::GUI::draw();
}

void Language_Settings_GUI::set_text()
{
	std::vector<std::string> items;

	items.push_back(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id("English")));
	languages[items.back()] = "English";

	items.push_back(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id("Brazilian Portuguese")));
	languages[items.back()] = "Brazilian";

	items.push_back(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id("French")));
	languages[items.back()] = "French";

	items.push_back(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id("Spanish")));
	languages[items.back()] = "Spanish";

	list->set_items(items);

	std::string curr_lang = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(GLOBALS->language));
	int index = 0;
	for (size_t i = 0; i < items.size(); i++) {
		if (items[i] == curr_lang) {
			index = (int)i;
			break;
		}
	}
	list->set_selected(index);
	int vr = MIN(list->visible_rows(), (int)items.size());
	list->set_top(MIN((int)items.size()-vr, MAX(0, index-vr/2)));
}

void Language_Settings_GUI::transition_in_done()
{
	transition_is_enlarge = false;
	transition_is_shrink = true;
}
