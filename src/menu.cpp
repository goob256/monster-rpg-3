#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/onscreen_controller.h>
#include <Nooskewl_Wedge/special_number.h>
#include <Nooskewl_Wedge/spells.h>
#include <Nooskewl_Wedge/systems.h>

#include "dialogue.h"
#include "globals.h"
#include "general.h"
#include "gui.h"
#include "inventory.h"
#include "menu.h"
#include "milestones.h"
#include "monster_rpg_3.h"
#include "widgets.h"
#include "stats.h"
#include "vampires.h"

static std::string menu_name(Main_Menu_GUI *gui)
{
	std::string s;

	if (dynamic_cast<Progress_GUI *>(gui)) {
		s = "Progress";
	}
	else if (dynamic_cast<Map_GUI *>(gui)) {
		s = "Map";
	}
	else if (dynamic_cast<Stats_GUI *>(gui)) {
		s = "Stats";
	}
	else if (dynamic_cast<Items_GUI *>(gui)) {
		s = "Items";
	}
	else if (dynamic_cast<Spells_GUI *>(gui)) {
		s = "Spells";
	}
	else if (dynamic_cast<Weapons_GUI *>(gui)) {
		s = "Weapons";
	}
	else if (dynamic_cast<Armour_GUI *>(gui)) {
		s = "Armour";
	}
	else if (dynamic_cast<Vampires_GUI *>(gui)) {
		s = "Vampires";
	}
	else if (dynamic_cast<Discard_GUI *>(gui)) {
		s = "Discard";
	}

	return s;
}

static void spells_callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Spells_GUI::Callback_Data *userdata = static_cast<Spells_GUI::Callback_Data *>(d->userdata);

	if (d->choice >= 0) {
		std::vector<wedge::Base_Stats *> v;
		if (d->choice < (int)INSTANCE->stats.size()) {
			v.push_back(&INSTANCE->stats[d->choice].base);
		}
		else {
			for (size_t i = 0; i < MAX_PARTY; i++) {
				v.push_back(&INSTANCE->stats[i].base);
			}
		}

		SPELLS->use(userdata->spell, v);

		SPELLS->play_sound(userdata->spell);

		userdata->caster_stats->mp -= SPELLS->get_cost(userdata->spell);

		userdata->gui->set_text(-1, -1);
	}

	delete userdata;
}

static void discard_callback(void *data)
{
	Get_Number_GUI::Callback_Data *d = (Get_Number_GUI::Callback_Data *)data;
	Discard_GUI::Callback_Data *user = static_cast<Discard_GUI::Callback_Data *>(d->userdata);

	if (d->number > 0) {
		INSTANCE->inventory.remove(user->item_index, d->number);
		user->gui->set_text();
	}
}

Main_Menu_GUI::Main_Menu_GUI(int character_index) :
	Menu_GUI(character_index),
	showing_settings(false)
{
}

void Main_Menu_GUI::update_background()
{
	if (showing_settings) {
		if (settings_active() == false) {
			showing_settings = false;
			if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
				wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
			}
		}
	}
	else if (settings_active()) {
		showing_settings = true;
	}
}

void Main_Menu_GUI::handle_event(TGUI_Event *event)
{
	util::Point<int> settings_button_pos = GLOBALS->get_onscreen_button_position(wedge::ONSCREEN_B4);
	GLOBALS->dpad->set_animation("button4");
	gfx::Image *image = GLOBALS->dpad->get_image(0);

	if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_p))) {
		if (dynamic_cast<Progress_GUI *>(this) == NULL) {
			Progress_GUI *new_gui = new Progress_GUI(character_index);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_m))) {
		if (dynamic_cast<Map_GUI *>(this) == NULL) {
			Map_GUI *new_gui = new Map_GUI(character_index);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_t))) {
		if (dynamic_cast<Stats_GUI *>(this) == NULL) {
			Stats_GUI *new_gui = new Stats_GUI(character_index);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_i))) {
		if (dynamic_cast<Items_GUI *>(this) == NULL) {
			Items_GUI *new_gui = new Items_GUI(character_index, -1, -1);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_s))) {
		int num_spells = 0;
		for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
			num_spells += INSTANCE->stats[i].base.num_spells();
		}
		if (num_spells > 0 && dynamic_cast<Spells_GUI *>(this) == NULL) {
			Spells_GUI *new_gui = new Spells_GUI(character_index, -1, -1);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_w))) {
		if (dynamic_cast<Weapons_GUI *>(this) == NULL) {
			Weapons_GUI *new_gui = new Weapons_GUI(character_index, -1, -1);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_a))) {
		if (dynamic_cast<Armour_GUI *>(this) == NULL) {
			Armour_GUI *new_gui = new Armour_GUI(character_index, -1, -1);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_v))) {
		if (M3_INSTANCE->num_vampires() > 0 && dynamic_cast<Vampires_GUI *>(this) == NULL) {
			Vampires_GUI *new_gui = new Vampires_GUI(character_index);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (next_character_gui == NULL && next_gui == NULL && is_key(event->keyboard.code, get_translated_hotkey(TGUIK_d))) {
		if (dynamic_cast<Discard_GUI *>(this) == NULL) {
			Discard_GUI *new_gui = new Discard_GUI(character_index);
			set_next_gui(new_gui, is_left(this, new_gui));
		}
	}
	else if (can_show_settings() && M3_GLOBALS->hide_onscreen_settings_button == false && wedge::is_onscreen_controller_enabled() == false && next_character_gui == NULL && next_gui == NULL && event->type == TGUI_MOUSE_DOWN && cd::box_box(settings_button_pos, settings_button_pos+image->size-1, util::Point<int>(event->mouse.x, event->mouse.y), util::Point<int>(event->mouse.x+1, event->mouse.y+1))) {
		show_settings();
	}
	else {
		Menu_GUI::handle_event(event);
	}
}

bool Main_Menu_GUI::is_left(Main_Menu_GUI *current, Main_Menu_GUI *next)
{
	std::string c = menu_name(current);
	std::string n = menu_name(next);

	std::map<std::string, int> map;
	map["Map"] = 0;
	map["Progress"] = 1;
	map["Stats"] = 2;
	map["Items"] = 3;
	map["Spells"] = 4;
	map["Weapons"] = 5;
	map["Armour"] = 6;
	map["Vampires"] = 7;
	map["Discard"] = 8;

	int c_i, n_i;

	if (map.find(c) == map.end()) {
		c_i = -1;
	}
	else {
		c_i = map[c];
	}

	if (map.find(n) == map.end()) {
		n_i = -1;
	}
	else {
		n_i = map[n];
	}

	if (c_i > n_i) {
		return true;
	}
	return false;
}

std::string Main_Menu_GUI::add_hotkey(std::string s, int code)
{
#ifdef IOS // Only way to do text input on iOS is a hidden text field but it's a hack...
	return s;
#else
	// Don't add highlight to keys mapped differently
	if (is_mapped_key(code)) {
		return s;
	}
	std::string keyname = get_key_name(code);
	char hotkey = keyname[0];
	size_t pos = s.find(hotkey);
	if (pos != std::string::npos) {
		s = "|14" + s.substr(0, pos) + "|0E" + s.substr(pos, 1) + "|14" + s.substr(pos+1);
	}
	return s;
#endif
}

std::string Main_Menu_GUI::get_caption()
{
	std::string caption;

	int code = -1;

	if (dynamic_cast<Progress_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1458)/* Originally: PROGRESS */;
		code = get_translated_hotkey(TGUIK_p);
	}
	else if (dynamic_cast<Map_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1739)/* Originally: MAP */;
		code = get_translated_hotkey(TGUIK_m);
	}
	else if (dynamic_cast<Stats_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1459)/* Originally: STATS */;
		code = get_translated_hotkey(TGUIK_t);
	}
	else if (dynamic_cast<Items_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1460)/* Originally: ITEMS */;
		code = get_translated_hotkey(TGUIK_i);
	}
	else if (dynamic_cast<Spells_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1461)/* Originally: SPELLS */;
		code = get_translated_hotkey(TGUIK_s);
	}
	else if (dynamic_cast<Weapons_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1462)/* Originally: WEAPONS */;
		code = get_translated_hotkey(TGUIK_w);
	}
	else if (dynamic_cast<Armour_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1463)/* Originally: ARMOUR */;
		code = get_translated_hotkey(TGUIK_a);
	}
	else if (dynamic_cast<Vampires_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1464)/* Originally: VAMPIRES */;
		code = get_translated_hotkey(TGUIK_v);
	}
	else if (dynamic_cast<Discard_GUI *>(this)) {
		caption = GLOBALS->game_t->translate(1465)/* Originally: DISCARD */;
		code = get_translated_hotkey(TGUIK_d);
	}

	// FIXME: hotkey will change by language!
	if (code != -1) {
		caption = add_hotkey(caption, code);
	}

	return caption;
}

void Main_Menu_GUI::draw_fore()
{
	Menu_GUI::draw_fore();

	GLOBALS->bold_font->enable_shadow(shim::black, gfx::Font::FULL_SHADOW);
	std::string text = get_caption();
	int w = GLOBALS->bold_font->get_text_width(text);
	GLOBALS->bold_font->draw(shim::palette[20], text, util::Point<int>(shim::screen_size.w * 0.95f - w - 5, shim::screen_size.h * 0.05f));
	GLOBALS->bold_font->disable_shadow();

#ifndef TVOS
	if (can_show_settings() && M3_GLOBALS->hide_onscreen_settings_button == false && wedge::is_onscreen_controller_enabled() == false) {
		util::Point<int> settings_button_pos = GLOBALS->get_onscreen_button_position(wedge::ONSCREEN_B4);
		GLOBALS->dpad->set_animation("button4");
		gfx::Image *image = GLOBALS->dpad->get_image(0);
		SDL_Colour tint = shim::white;
		tint.r *= GLOBALS->onscreen_controls_alpha;
		tint.g *= GLOBALS->onscreen_controls_alpha;
		tint.b *= GLOBALS->onscreen_controls_alpha;
		tint.a *= GLOBALS->onscreen_controls_alpha;
		image->draw_tinted(tint, settings_button_pos);
	}
#endif
}

bool Main_Menu_GUI::is_key(int code, int desired)
{
	if (is_mapped_key(desired)) {
		return false;
	}
	return code == desired;
}

void Main_Menu_GUI::maybe_show_third_button_help()
{
	if (next_character_gui == NULL && next_gui == NULL && INSTANCE->is_milestone_complete(MS_THIRD_BUTTON_HELP) == false) {
		INSTANCE->set_milestone_complete(MS_THIRD_BUTTON_HELP, true);
		std::string text;
#ifdef TVOS
		if (true) {
#else
		if (input::is_joystick_connected()) {
#endif
			text = GLOBALS->game_t->translate(1024)/* Originally: Press */ + " " + get_joystick_button_name(GLOBALS->joy_b3) + " " + GLOBALS->game_t->translate(1467)/* Originally: for info on your inventory. */;
		}
		else {
#if !defined IOS && !defined ANDROID
			bool touch = GLOBALS->onscreen_controller_was_enabled;
#else
			bool touch = util::system_has_touchscreen();
#endif
			if (touch) {
				text = GLOBALS->game_t->translate(1468)/* Originally: Press and hold for info on your inventory. */;
			}
			else {
				text = GLOBALS->game_t->translate(1024)/* Originally: Press */ + " " + get_key_name(GLOBALS->key_b3) + " " + GLOBALS->game_t->translate(1467)/* Originally: for info on your inventory. */;
			}
		}
		M3_GLOBALS->set_darken_screen_on_next_dialogue(true);
		GLOBALS->do_dialogue("", text, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_BOTTOM, NULL);
	}
}

int Main_Menu_GUI::get_translated_hotkey(int code)
{
	if (GLOBALS->language == "French") {
		if (code == TGUIK_i) {
			code = TGUIK_o;
		}
		else if (code == TGUIK_w) {
			code = TGUIK_m;
		}
		else if (code == TGUIK_d) {
			code = TGUIK_j;
		}
		else if (code == TGUIK_m) {
			code = TGUIK_c;
		}
	}
	else if (GLOBALS->language == "Spanish") {
		if (code == TGUIK_i) {
			code = TGUIK_o;
		}
		else if (code == TGUIK_s) {
			code = TGUIK_g;
		}
		else if (code == TGUIK_w) {
			code = TGUIK_r;
		}
		else if (code == TGUIK_a) {
			code = TGUIK_u;
		}
	}
	else if (GLOBALS->language == "Brazilian") {
		if (code == TGUIK_w) {
			code = TGUIK_r;
		}
	}
	
	return code;
}

//--

Progress_GUI::Progress_GUI(int character_index) :
	Main_Menu_GUI(character_index)
{
	wedge::set_onscreen_controller_b3_enabled(false, -1);
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = true;
	has_right_gui = true;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	Widget_Window *window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	TGUI_Widget *column = new TGUI_Widget();
	column->set_centre_x(true);
	column->set_centre_y(true);
	column->set_parent(window);

	resume_button = new Widget_Text_Button(GLOBALS->game_t->translate(1471)/* Originally: Resume Game */);
	resume_button->set_centre_x(true);
	resume_button->set_parent(column);

	save_button = new Widget_Text_Button(GLOBALS->game_t->translate(1472)/* Originally: Save Game */);
	save_button->set_centre_x(true);
	save_button->set_padding_top(5+resume_button->get_height());
	save_button->set_clear_float_x(true);
	save_button->set_parent(column);
	if (AREA->get_current_area()->can_save() == false) {
		save_button->set_enabled(false);
	}

	quit_button = new Widget_Text_Button(GLOBALS->game_t->translate(1473)/* Originally: Quit to title */);
	quit_button->set_centre_x(true);
	quit_button->set_padding_top(10+resume_button->get_height()*2);
	quit_button->set_clear_float_x(true);
	quit_button->set_parent(column);

	gui = new TGUI(window, window_w, window_h);

	gui->set_focus(resume_button);
}

Progress_GUI::~Progress_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Progress_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_LEFT) {
				Map_GUI *new_gui = new Map_GUI(character_index);
				set_next_gui(new_gui, true);
			}
			else if (event->focus.type == TGUI_FOCUS_RIGHT) {
				Stats_GUI *new_gui = new Stats_GUI(character_index);
				set_next_gui(new_gui, false);
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Progress_GUI::update()
{
	if (resume_button->pressed()) {
		// cause game to be resume as if escape key pressed (so we don't repeat code)
		TGUI_Event event;
		event.type = TGUI_KEY_DOWN;
		event.keyboard.code = GLOBALS->key_b2;
		event.keyboard.is_repeat = false;
		Main_Menu_GUI::handle_event(&event);
	}
	else if (save_button->pressed()) {
		show_save_screen(NULL);
	}
	else if (quit_button->pressed()) {
		GLOBALS->mini_pause();
	}
	
	Main_Menu_GUI::update();
}

//--

Map_GUI::Map_GUI(int character_index) :
	Main_Menu_GUI(character_index)
{
	wedge::set_onscreen_controller_b3_enabled(false, -1);
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = false;
	has_right_gui = true;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	map_widget = new Widget_Map();
	map_widget->set_centre_x(true);
	map_widget->set_centre_y(true);
	map_widget->set_parent(window);
	
	found_device();

	gui = new TGUI(window, window_w, window_h);

	gui->set_focus(map_widget);
}

Map_GUI::~Map_GUI()
{
	delete map;
	delete map2;

	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Map_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_RIGHT) {
				Progress_GUI *new_gui = new Progress_GUI(character_index);
				set_next_gui(new_gui, false);
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Map_GUI::lost_device()
{
	delete map;
	delete map2;
	map = NULL;
	map2 = NULL;
	map_widget->set_images(NULL, NULL);
}

void Map_GUI::found_device()
{
	gfx::Font::end_batches();

	int w = window->get_width() * 0.65f;
	int h = window->get_height() * 0.65f;

	map = new gfx::Image(util::Size<int>(w, h));
	map2 = new gfx::Image(util::Size<int>(w, h));

	gfx::set_target_image(map);

	draw_map(w, h, false);

	set_target_image(map2);

	draw_map(w, h, true);

	gfx::set_target_backbuffer();

	map_widget->set_images(map, map2);
}

void Map_GUI::draw_map(int w, int h, bool draw_portals)
{
	gfx::clear(shim::transparent);

	wedge::Area *area = AREA->get_current_area();
	std::string name = area->get_name();

	if (name == "mountains2" || name.substr(0, 6) == "desert" || name == "boatin") {
		shim::font->draw(shim::white, "?", util::Point<float>(w/2.0f, h/2.0f-shim::font->get_height()/2.0f), false, true);
		gfx::Font::end_batches();
	}
	else {
		gfx::Tilemap *tilemap = area->get_tilemap();

		wedge::Map_Entity *eny = AREA->get_player(ENY);
		util::Point<int> epos = eny->get_position();

		wedge::Area_Hooks *hooks = area->get_hooks();

		gfx::draw_primitives_start();

		for (int y = 0; y < h; y ++) {
			int yy = y - h/2 + epos.y;
			if (yy < 0 || yy >= tilemap->get_size().h) {
				continue;
			}
			int dy = MIN(y, h-y-1);
			for (int x = 0; x < w; x++) {
				int xx = x - w/2 + epos.x;
				if (xx < 0 || xx >= tilemap->get_size().w) {
					continue;
				}
				int dx = MIN(x, w-x-1);
				int d = MIN(dx, dy);
				util::Point<int> pos(xx, yy);
				if (draw_portals && hooks != NULL && (hooks->get_fade_zone(pos) || hooks->get_scroll_zones(pos).size() > 0)) {
					SDL_Colour c = shim::magenta;
					if (d <= 8) {
						float p = d / 8.0f;
						c.r *= p;
						c.g *= p;
						c.b *= p;
						c.a *= p;
					}
					gfx::draw_filled_rectangle(c, util::Point<int>(x, y), util::Size<int>(1, 1));
				}
				else if (draw_portals == false && tilemap->is_solid(-1, pos)) {
					SDL_Colour c = shim::white;
					if (d <= 8) {
						float p = d / 8.0f;
						c.r *= p;
						c.g *= p;
						c.b *= p;
						c.a *= p;
					}
					gfx::draw_filled_rectangle(c, util::Point<int>(x, y), util::Size<int>(1, 1));
				}
			}
		}
	
		if (draw_portals == false) {
			if (AREA->get_players().size() > 1) {
				wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);
				util::Point<int> tpos = tiggy->get_position();
				util::Point<int> diff = tpos - epos;
				gfx::draw_filled_rectangle(shim::palette[13]/*green*/, util::Point<int>(w/2+diff.x, h/2+diff.y), util::Size<int>(1, 1));
			}

			gfx::draw_filled_rectangle(shim::palette[10]/*orange*/, util::Point<int>(w/2, h/2), util::Size<int>(1, 1));
			
		}
		
		gfx::draw_primitives_end();
	}
}

void Map_GUI::draw_fore()
{
	Main_Menu_GUI::draw_fore();

	int y = shim::screen_size.h - (shim::screen_size.h * 0.05f + shim::font->get_height());

	shim::font->enable_shadow(shim::palette[27], gfx::Font::FULL_SHADOW);

	std::string text = GLOBALS->game_t->translate(1741);
	int total = shim::font->get_text_width(text);

	shim::font->draw(shim::palette[14], text, util::Point<float>(shim::screen_size.w/2-total/2, y+0.5f));

	shim::font->disable_shadow();
}

//--

Stats_GUI::Stats_GUI(int character_index) :
	Main_Menu_GUI(character_index)
{
	if (INSTANCE->stats.size() > 1) {
		wedge::set_onscreen_controller_b3_enabled(true, M3_GLOBALS->key_switch);
	}
	else {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
	}
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = true;
	has_right_gui = true;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	Widget_Window *window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	Widget *column0 = new Widget(TGUI_Widget::FIT_Y, 1.0f);
	column0->set_parent(window);

	Widget_Image *profile_pic = new Widget_Image(M3_GLOBALS->profile_images[character_index], false);
	profile_pic->set_padding_top(3);
	profile_pic->set_padding_left(3);
	Widget_Label *name_label = new Widget_Label(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(INSTANCE->stats[character_index].name)) + TAG_END, -1);
	name_label->set_padding_top(4);
	name_label->set_padding_left(3);
	status_label = new Widget_Label("", -1);
	status_label->set_padding_top(4);
	status_label->set_padding_left(1);

	profile_pic->set_parent(column0);
	name_label->set_parent(column0);
	status_label->set_parent(column0);

	Widget *column1 = new Widget(0.555f, 1.0f);
	column1->set_parent(window);

	Widget_Label *l1 = new Widget_Label(GLOBALS->game_t->translate(1338)/* Originally: HP */, -1);
	Widget_Label *l2 = new Widget_Label(GLOBALS->game_t->translate(1337)/* Originally: MP */, -1);
	Widget_Label *l3 = new Widget_Label(GLOBALS->game_t->translate(1476)/* Originally: Level */, -1);
	Widget_Label *l4 = new Widget_Label(GLOBALS->game_t->translate(1477)/* Originally: Exp */, -1);
	Widget_Label *l5 = new Widget_Label(GLOBALS->game_t->translate(1478)/* Originally: Next */, -1);
	
	l2->set_break_line(true);
	l4->set_break_line(true);
	l5->set_break_line(true);

	l1->set_padding_left(3);
	l2->set_padding_left(3);
	l3->set_padding_left(3);
	l4->set_padding_left(3);
	l5->set_padding_left(3);

	Widget *labels1 = new Widget();
	labels1->set_break_line(true);
	labels1->set_parent(column1);

	l1->set_parent(labels1);
	l2->set_parent(labels1);

	Widget *labels2 = new Widget();
	labels2->set_parent(column1);

	hp_label = new Widget_Label("", -1);
	hp_label->set_padding_left(5);
	hp_label->set_parent(labels2);

	mp_label = new Widget_Label("", -1);
	mp_label->set_break_line(true);
	mp_label->set_padding_left(5);
	mp_label->set_parent(labels2);

	Widget *labels3 = new Widget();
	labels3->set_break_line(true);
	labels3->set_parent(column1);

	l3->set_parent(labels3);
	l4->set_parent(labels3);
	l5->set_parent(labels3);

	Widget *labels4 = new Widget();
	labels4->set_parent(column1);

	level_label = new Widget_Label("", -1);
	level_label->set_padding_left(5);
	level_label->set_parent(labels4);

	experience_label = new Widget_Label("", -1);
	experience_label->set_break_line(true);
	experience_label->set_padding_left(5);
	experience_label->set_parent(labels4);

	next_label = new Widget_Label("", -1);
	next_label->set_break_line(true);
	next_label->set_padding_left(5);
	next_label->set_parent(labels4);

	Widget *column2 = new Widget(0.44f, 1.0f);
	column2->set_parent(window);

	Widget_Label *l6 = new Widget_Label(GLOBALS->game_t->translate(1479)/* Originally: ATK */, -1);
	Widget_Label *l7 = new Widget_Label(GLOBALS->game_t->translate(1480)/* Originally: DEF */, -1);
	Widget_Label *l8 = new Widget_Label(GLOBALS->game_t->translate(1481)/* Originally: LUK */, -1);
	Widget_Label *l9 = new Widget_Label(GLOBALS->game_t->translate(1482)/* Originally: Gold */, -1);
	l9->set_text_colour(shim::palette[11]);
	l9->set_shadow_colour(shim::palette[24]);
	
	l7->set_break_line(true);
	l8->set_break_line(true);
	l9->set_break_line(true);

	l9->set_padding_top(next_label->get_height());

	Widget *labels5 = new Widget();
	//labels5->set_padding_top(3 + M3_GLOBALS->profile_images[character_index]->size.h);
	labels5->set_parent(column2);

	l6->set_parent(labels5);
	l7->set_parent(labels5);
	l8->set_parent(labels5);
	l9->set_parent(labels5);

	Widget *labels6 = new Widget();
	//labels6->set_padding_top(3 + M3_GLOBALS->profile_images[character_index]->size.h);
	labels6->set_parent(column2);

	attack_label = new Widget_Label("", -1);
	attack_label->set_padding_left(5);
	attack_label->set_parent(labels6);

	defense_label = new Widget_Label("", -1);
	defense_label->set_padding_left(5);
	defense_label->set_break_line(true);
	defense_label->set_parent(labels6);

	luck_label = new Widget_Label("", -1);
	luck_label->set_padding_left(5);
	luck_label->set_break_line(true);
	luck_label->set_parent(labels6);

	gold_label = new Widget_Label("", -1);
	gold_label->set_padding_left(5);
	gold_label->set_padding_top(next_label->get_height());
	gold_label->set_break_line(true);
	gold_label->set_parent(labels6);
	gold_label->set_text_colour(shim::palette[11]);
	gold_label->set_shadow_colour(shim::palette[24]);

	set_text();

	gui = new TGUI(window, window_w, window_h);
}

Stats_GUI::~Stats_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Stats_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (INSTANCE->stats.size() > 1 && // Have Tiggy
		((event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_switch) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_switch))
	) {
		if (next_character_gui == NULL && next_gui == NULL) {
			int next_index = character_index == 0 ? 1 : 0;
			Stats_GUI *new_gui = new Stats_GUI(next_index);
			set_next_character_gui(new_gui, false);
		}
	}
	else if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_LEFT) {
				Progress_GUI *new_gui = new Progress_GUI(character_index);
				set_next_gui(new_gui, true);
			}
			else if (event->focus.type == TGUI_FOCUS_RIGHT) {
				Items_GUI *new_gui = new Items_GUI(character_index, -1, -1);
				set_next_gui(new_gui, false);
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Stats_GUI::set_text()
{
	status_label->set_text(get_status_name(stats->base.hp, stats->base.status));
	status_label->set_text_colour(get_status_colour(stats->base.hp, stats->base.status));

	hp_label->set_text(util::itos(stats->base.hp) + "/" + util::itos(stats->base.fixed.max_hp));
	hp_label->set_text_colour(get_status_colour(stats->base.hp - stats->base.fixed.max_hp * 0.15f, wedge::STATUS_OK));
	mp_label->set_text(util::itos(stats->base.mp) + "/" + util::itos(stats->base.fixed.max_mp));

	level_label->set_text(util::itos(stats->level));
	experience_label->set_text(util::itos(stats->experience));

	if (stats->level < (int)wedge::globals->levels.size()+1) {
		int next = wedge::globals->levels[stats->level-1].experience - stats->experience;
		if (next < 0) {
			next = 0;
		}
		next_label->set_text(util::itos(next));
	}
	else {
		next_label->set_text("-");
	}

	int attack = 0;
	attack += stats->base.fixed.attack;
	attack += stats->weapon.stats.attack;
	attack += stats->armour.stats.attack;

	int defense = 0;
	defense += stats->base.fixed.defense;
	defense += stats->armour.stats.defense;
	defense += stats->weapon.stats.defense;

	int luck = 0;
	luck += stats->base.fixed.get_extra(LUCK);
	luck += stats->weapon.stats.get_extra(LUCK);
	luck += stats->armour.stats.get_extra(LUCK);

	attack_label->set_text(util::itos(attack));
	defense_label->set_text(util::itos(defense));
	luck_label->set_text(util::itos(luck));

	gold_label->set_text(util::itos(INSTANCE->get_gold()));
}

//--

Items_GUI::Items_GUI(int character_index, int top, int selected) :
	Main_Menu_GUI(character_index)
{
	if (INSTANCE->stats.size() > 1) {
		wedge::set_onscreen_controller_b3_enabled(true, M3_GLOBALS->key_switch);
	}
	else {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
	}
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = true;
	has_right_gui = true;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	Widget *column1 = new Widget(TGUI_Widget::FIT_Y, 1.0f);
	column1->set_parent(window);

	gfx::Image *pic = M3_GLOBALS->mini_profile_images[character_index];

	profile_pic = new Widget_Image(pic, false);
	profile_pic->set_shadow_colour(shim::palette[24]);
	profile_pic->set_padding_top(4);
	profile_pic->set_padding_left(4);
	status_label = new Widget_Label("", -1);
	status_label->set_padding_top(2);
	status_label->set_padding_left(5);

	profile_pic->set_parent(column1);
	status_label->set_parent(column1);

	Widget_Label *l1 = new Widget_Label(GLOBALS->game_t->translate(1338)/* Originally: HP */, -1);
	Widget_Label *l2 = new Widget_Label(GLOBALS->game_t->translate(1337)/* Originally: MP */, -1);
	
	l2->set_break_line(true);

	l1->set_padding_left(3);
	l2->set_padding_left(3);

	Widget *labels1 = new Widget();
	labels1->set_break_line(true);
	labels1->set_parent(column1);

	l1->set_parent(labels1);
	l2->set_parent(labels1);

	Widget *labels2 = new Widget();
	labels2->set_parent(column1);

	hp_label = new Widget_Label("", -1);
	hp_label->set_padding_left(5);
	hp_label->set_parent(labels2);

	mp_label = new Widget_Label("", -1);
	mp_label->set_break_line(true);
	mp_label->set_padding_left(5);
	mp_label->set_parent(labels2);

	list = new Widget_Quantity_List(1.0f, shim::font->get_height() * 3);
	list->set_break_line(true);
	list->set_padding_left(3);
	list->set_padding_right(3);
	list->set_parent(window);
	list->set_reserved_size(SCR_H - (3.0f * shim::font->get_height() + 4));

	set_text(top, selected);

	gui = new TGUI(window, window_w, window_h);
}

Items_GUI::~Items_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Items_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (INSTANCE->stats.size() > 1 && // Have Tiggy
		((event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_switch) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_switch))
	) {
		if (next_character_gui == NULL && next_gui == NULL) {
			int next_index = character_index == 0 ? 1 : 0;
			Items_GUI *new_gui = new Items_GUI(next_index, list == NULL ? -1 : list->get_top(), list == NULL ? -1 : list->get_selected());
			set_next_character_gui(new_gui, false);
		}
	}
	else if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_LEFT) {
				Stats_GUI *new_gui = new Stats_GUI(character_index);
				set_next_gui(new_gui, true);
			}
			else if (event->focus.type == TGUI_FOCUS_RIGHT) {
				bool have_spells = false;
				for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
					if (INSTANCE->stats[i].base.num_spells() > 0) {
						have_spells = true;
						break;
					}
				}
				if (have_spells) {
					Spells_GUI *new_gui = new Spells_GUI(character_index, -1, -1);
					set_next_gui(new_gui, false);
				}
				else {
					Weapons_GUI *new_gui = new Weapons_GUI(character_index, -1, -1);
					set_next_gui(new_gui, false);
				}
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Items_GUI::update()
{
	if (list != NULL) {
		maybe_show_third_button_help();
	}

	if (list != NULL) {
		int pressed;
		if ((pressed = list->pressed()) >= 0) {
			// set and reset this (reset because do_dialogue might not be called) in case it's a scroll
			int index = inventory_indices[pressed];
			int id = INSTANCE->inventory.get_all()[index].id;
			M3_GLOBALS->set_darken_screen_on_next_dialogue(true);
			int amount = INSTANCE->inventory.use(index, &stats->base);
			M3_GLOBALS->set_darken_screen_on_next_dialogue(false);
			if (amount != 0 || id >= 0) {
				SDL_Colour colour, shadow_colour;
				std::string text;
				get_use_item_info(amount, id, colour, shadow_colour, text);
				if (colour.a != 0 || shadow_colour.a != 0) {
					util::Point<int> number_pos = util::Point<int>(profile_pic->get_x() + profile_pic->get_width() + shim::font->get_text_width(status_label->get_text()) + shim::tile_size, profile_pic->get_y());
					number_pos += get_offset();
					NEW_SYSTEM_AND_TASK(MENU)
					wedge::Special_Number_Step *step = new wedge::Special_Number_Step(colour, shadow_colour, text, number_pos, wedge::Special_Number_Step::RISE, new_task, true);
					ADD_STEP(step)
					ADD_TASK(new_task)
					FINISH_SYSTEM(MENU)
				}
			}
			set_text(-1, -1);
		}
	}
	
	Main_Menu_GUI::update();
}

void Items_GUI::set_text(int top, int selected)
{
	status_label->set_text(get_status_name(stats->base.hp, stats->base.status));
	status_label->set_text_colour(get_status_colour(stats->base.hp, stats->base.status));

	hp_label->set_text(util::itos(stats->base.hp) + "/" + util::itos(stats->base.fixed.max_hp));
	hp_label->set_text_colour(get_status_colour(stats->base.hp - stats->base.fixed.max_hp * 0.15f, wedge::STATUS_OK));
	mp_label->set_text(util::itos(stats->base.mp) + "/" + util::itos(stats->base.fixed.max_mp));

	inventory_indices.clear();

	std::vector< std::pair<int, std::string> > v;
	std::vector<std::string> descriptions;

	wedge::Object *objects = INSTANCE->inventory.get_all();

	std::vector<int> disabled;

	for (int i = 0; i < wedge::Inventory::MAX_OBJECTS; i++) {
		std::pair<int, std::string> p;
		if (objects[i].type == wedge::OBJECT_ITEM || objects[i].type == wedge::OBJECT_SPECIAL) {
			if (objects[i].type == wedge::OBJECT_SPECIAL) {
				disabled.push_back((int)v.size());
			}
			p.first = objects[i].quantity;
			p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(objects[i].name));
			v.push_back(p);
			inventory_indices.push_back(i);
			descriptions.push_back(objects[i].description);
		}
	}

	if (v.size() == 0) {
		list->set_parent(NULL);
		delete list;
		list = NULL;
		if (gui != NULL) {
			gui->focus_something();
		}
		Widget_Label *empty_label = new Widget_Label(GLOBALS->game_t->translate(1485)/* Originally: No items */, -1);
		empty_label->set_padding_top(10);
		empty_label->set_centre_x(true);
		empty_label->set_break_line(true);
		empty_label->set_parent(window);
		if (gui) {
			gui->layout();
		}
	}
	else {
		list->set_items(v);
		list->set_descriptions(descriptions);
		int selected = list->get_selected();
		if (selected >= (int)v.size()) {
			list->set_selected(selected-1);
		}

		for (size_t i = 0; i < v.size(); i++) {
			list->set_disabled((int)i, false);
		}
		for (size_t i = 0; i < disabled.size(); i++) {
			list->set_disabled(disabled[i], true);
		}
	}

	if (list != NULL && top >= 0 && selected >= 0) {
		list->set_top(top);
		list->set_selected(selected);
	}
}

//--

Spells_GUI::Spells_GUI(int character_index, int top, int selected) :
	Main_Menu_GUI(character_index)
{
	if (INSTANCE->stats.size() > 1) {
		wedge::set_onscreen_controller_b3_enabled(true, M3_GLOBALS->key_switch);
	}
	else {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
	}
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = true;
	has_right_gui = true;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	Widget *column1 = new Widget(TGUI_Widget::FIT_Y, 1.0f);
	column1->set_parent(window);

	gfx::Image *pic = M3_GLOBALS->mini_profile_images[character_index];

	profile_pic = new Widget_Image(pic, false);
	profile_pic->set_shadow_colour(shim::palette[24]);
	profile_pic->set_padding_top(4);
	profile_pic->set_padding_left(4);
	status_label = new Widget_Label("", -1);
	status_label->set_padding_top(2);
	status_label->set_padding_left(5);

	profile_pic->set_parent(column1);
	status_label->set_parent(column1);

	Widget_Label *l1 = new Widget_Label(GLOBALS->game_t->translate(1338)/* Originally: HP */, -1);
	Widget_Label *l2 = new Widget_Label(GLOBALS->game_t->translate(1337)/* Originally: MP */, -1);
	
	l2->set_break_line(true);

	l1->set_padding_left(3);
	l2->set_padding_left(3);

	Widget *labels1 = new Widget();
	labels1->set_break_line(true);
	labels1->set_parent(column1);

	l1->set_parent(labels1);
	l2->set_parent(labels1);

	Widget *labels2 = new Widget();
	labels2->set_parent(column1);

	hp_label = new Widget_Label("", -1);
	hp_label->set_padding_left(5);
	hp_label->set_parent(labels2);

	mp_label = new Widget_Label("", -1);
	mp_label->set_break_line(true);
	mp_label->set_padding_left(5);
	mp_label->set_parent(labels2);

	list = new Widget_Quantity_List(1.0f, shim::font->get_height() * 3);
	list->set_break_line(true);
	list->set_padding_left(3);
	list->set_padding_right(3);
	list->set_parent(window);
	list->set_reserved_size(SCR_H - (3.0f * shim::font->get_height() + 4));

	set_text(top, selected);

	gui = new TGUI(window, window_w, window_h);
}

Spells_GUI::~Spells_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Spells_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (INSTANCE->stats.size() > 1 && // Have Tiggy
		((event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_switch) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_switch))
	) {
		if (next_character_gui == NULL && next_gui == NULL) {
			int next_index = character_index == 0 ? 1 : 0;
			Spells_GUI *new_gui = new Spells_GUI(next_index, list == NULL ? -1 : list->get_top(), list == NULL ? -1 : list->get_selected());
			set_next_character_gui(new_gui, false);
		}
	}
	else if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_LEFT) {
				Items_GUI *new_gui = new Items_GUI(character_index, -1, -1);
				set_next_gui(new_gui, true);
			}
			else if (event->focus.type == TGUI_FOCUS_RIGHT) {
				Weapons_GUI *new_gui = new Weapons_GUI(character_index, -1, -1);
				set_next_gui(new_gui, false);
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Spells_GUI::update()
{
	if (list) {
		int pressed;
		if ((pressed = list->pressed()) >= 0) {
			std::string spell = stats->base.spell(list->get_selected());
			int cost = SPELLS->get_cost(spell);
			if (stats->base.mp < cost) {
				Notification_GUI *notification_gui = new Notification_GUI(GLOBALS->game_t->translate(1488)/* Originally: Not enough MP! */);
				shim::guis.push_back(notification_gui);
			}
			else {
				std::vector<std::string> choices;
				for (size_t i = 0; i < MAX_PARTY; i++) {
					choices.push_back(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(INSTANCE->stats[i].name)));
				}
				if (SPELLS->can_multi(spell)) {
					choices.push_back(GLOBALS->game_t->translate(1489)/* Originally: All */);
				}
				Callback_Data *userdata = new Callback_Data;
				userdata->spell = spell;
				userdata->caster_stats = &stats->base;
				userdata->gui = this;
				Widget_Spell_Target_List *spell_target_list = new Widget_Spell_Target_List(1.0f, 100); // size gets set in GUI constructor
				Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(true, GLOBALS->game_t->translate(1490)/* Originally: Select target: */, choices, -2, 0, 0, 0, 0, 0, 0, 0.03f, 0.03f, spells_callback, userdata, 3, 150, true, 0.75f, spell_target_list);
				gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
				shim::guis.push_back(gui);
			}
		}
	}
	
	Main_Menu_GUI::update();
}

void Spells_GUI::set_text(int top, int selected)
{
	status_label->set_text(get_status_name(stats->base.hp, stats->base.status));
	status_label->set_text_colour(get_status_colour(stats->base.hp, stats->base.status));

	hp_label->set_text(util::itos(stats->base.hp) + "/" + util::itos(stats->base.fixed.max_hp));
	hp_label->set_text_colour(get_status_colour(stats->base.hp - stats->base.fixed.max_hp * 0.15f, wedge::STATUS_OK));
	mp_label->set_text(util::itos(stats->base.mp) + "/" + util::itos(stats->base.fixed.max_mp));

	std::vector< std::pair<int, std::string> > v;

	std::vector<int> disabled;

	for (int i = 0; i < stats->base.num_spells(); i++) {
		std::pair<int, std::string> p;
		p.first = SPELLS->get_cost(stats->base.spell(i));
		p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(stats->base.spell(i)));
		if (SPELLS->can_cast_in_menus(stats->base.spell(i)) == false || SPELLS->get_cost(stats->base.spell(i)) > stats->base.mp) {
			disabled.push_back((int)v.size());
		}
		v.push_back(p);
	}

	if (v.size() == 0) {
		list->set_parent(NULL);
		delete list;
		list = NULL;
		if (gui != NULL) {
			gui->focus_something();
		}
		Widget_Label *empty_label = new Widget_Label(GLOBALS->game_t->translate(1491)/* Originally: No spells */, -1);
		empty_label->set_padding_top(10);
		empty_label->set_centre_x(true);
		empty_label->set_break_line(true);
		empty_label->set_parent(window);
		if (gui) {
			gui->layout();
		}
	}
	else {
		list->set_items(v);
		int sel = list->get_selected();
		if (sel >= (int)v.size()) {
			list->set_selected(sel-1);
		}

		for (size_t i = 0; i < v.size(); i++) {
			list->set_disabled((int)i, false);
		}
		for (size_t i = 0; i < disabled.size(); i++) {
			list->set_disabled(disabled[i], true);
		}
	}

	if (list != NULL && top >= 0 && selected >= 0) {
		list->set_top(top);
		list->set_selected(selected);
	}
}

//--

Weapons_GUI::Weapons_GUI(int character_index, int top, int selected) :
	Main_Menu_GUI(character_index)
{
	if (INSTANCE->stats.size() > 1) {
		wedge::set_onscreen_controller_b3_enabled(true, M3_GLOBALS->key_switch);
	}
	else {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
	}
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = true;
	has_right_gui = true;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	Widget *column1 = new Widget(TGUI_Widget::FIT_Y, 1.0f);
	column1->set_parent(window);

	gfx::Image *pic = M3_GLOBALS->mini_profile_images[character_index];

	profile_pic = new Widget_Image(pic, false);
	profile_pic->set_shadow_colour(shim::palette[24]);
	profile_pic->set_padding_top(4);
	profile_pic->set_padding_left(4);
	status_label = new Widget_Label("", -1);
	status_label->set_padding_top(2);
	status_label->set_padding_left(5);

	profile_pic->set_parent(column1);
	status_label->set_parent(column1);

	Widget_Label *l1 = new Widget_Label(GLOBALS->game_t->translate(1492)/* Originally: Weapon */, -1);
	Widget_Label *l2 = new Widget_Label(GLOBALS->game_t->translate(1347)/* Originally: Attack */, -1);
	
	l2->set_break_line(true);

	l1->set_padding_left(3);
	l2->set_padding_left(3);

	Widget *labels1 = new Widget();
	labels1->set_break_line(true);
	labels1->set_parent(column1);

	l1->set_parent(labels1);
	l2->set_parent(labels1);

	Widget *labels2 = new Widget();
	labels2->set_parent(column1);

	weapon_label = new Widget_Label("", -1);
	weapon_label->set_padding_left(5);
	weapon_label->set_parent(labels2);

	attack_label = new Widget_Label("", -1);
	attack_label->set_break_line(true);
	attack_label->set_padding_left(5);
	attack_label->set_parent(labels2);

	list = new Widget_Quantity_List(1.0f, shim::font->get_height() * 3);
	list->set_break_line(true);
	list->set_padding_left(3);
	list->set_padding_right(3);
	list->set_parent(window);
	list->set_reserved_size(SCR_H - (3.0f * shim::font->get_height() + 4));

	set_text(top, selected);

	gui = new TGUI(window, window_w, window_h);
}

Weapons_GUI::~Weapons_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Weapons_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (INSTANCE->stats.size() > 1 && // Have Tiggy
		((event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_switch) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_switch))
	) {
		if (next_character_gui == NULL && next_gui == NULL) {
			int next_index = character_index == 0 ? 1 : 0;
			Weapons_GUI *new_gui = new Weapons_GUI(next_index, list == NULL ? -1 : list->get_top(), list == NULL ? -1 : list->get_selected());
			set_next_character_gui(new_gui, false);
		}
	}
	else if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_LEFT) {
				bool have_spells = false;
				for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
					if (INSTANCE->stats[i].base.num_spells() > 0) {
						have_spells = true;
						break;
					}
				}
				if (have_spells) {
					Spells_GUI *new_gui = new Spells_GUI(character_index, -1, -1);
					set_next_gui(new_gui, true);
				}
				else {
					Items_GUI *new_gui = new Items_GUI(character_index, -1, -1);
					set_next_gui(new_gui, true);
				}
			}
			else if (event->focus.type == TGUI_FOCUS_RIGHT) {
				Armour_GUI *new_gui = new Armour_GUI(character_index, -1, -1);
				set_next_gui(new_gui, false);
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Weapons_GUI::update()
{
	if (list != NULL) {
		maybe_show_third_button_help();
	}

	int start_attack = 0;
	start_attack += stats->base.fixed.attack;
	start_attack += stats->weapon.stats.attack;
	start_attack += stats->armour.stats.attack;

	int end_attack = -1;

	if (list != NULL) {
		int pressed;
		if ((pressed = list->pressed()) >= 0) {
			wedge::Weapon w = INSTANCE->inventory.get_all()[inventory_indices[pressed]].id;
			if (stats->weapon.id == w) {
				stats->weapon = wedge::Weapon_Stats();
			}
			else {
				stats->weapon = wedge::Weapon_Stats(w);
			}
			set_text(-1, -1);
		}

		if (list->get_selected() >= 0) {
			wedge::Weapon w = INSTANCE->inventory.get_all()[inventory_indices[list->get_selected()]].id;
			if (stats->weapon.id != w) {
				end_attack = stats->base.fixed.attack;
				end_attack += wedge::Weapon_Stats(w).stats.attack;
				end_attack += stats->armour.stats.attack;
			}
		}
	}

	if (end_attack == -1) {
		attack_label->set_text(util::itos(start_attack));
	}
	else {
		std::string c;
		if (end_attack < start_attack) {
			c = "|09";
		}
		else if (end_attack > start_attack) {
			c = "|0d";
		}	
		attack_label->set_text(util::itos(start_attack) + " → " + c + util::itos(end_attack));
	}

	Main_Menu_GUI::update();
}

void Weapons_GUI::set_text(int top, int selected)
{
	status_label->set_text(get_status_name(stats->base.hp, stats->base.status));
	status_label->set_text_colour(get_status_colour(stats->base.hp, stats->base.status));

	wedge::Object weapon = OBJECT->make_object(wedge::OBJECT_WEAPON, stats->weapon.id, 1);
	weapon_label->set_text(weapon.name == "" ? "" : GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(weapon.name)));

	inventory_indices.clear();

	std::vector< std::pair<int, std::string> > v;
	std::vector<std::string> descriptions;

	wedge::Object *objects = INSTANCE->inventory.get_all();

	int highlight = -1;
	int disabled = -1;

	for (int i = 0; i < wedge::Inventory::MAX_OBJECTS; i++) {
		std::pair<int, std::string> p;
		if (objects[i].type == wedge::OBJECT_WEAPON) {
			int other_player_index = 1 - character_index;
			if (objects[i].quantity == 1 && objects[i].id == INSTANCE->stats[other_player_index].weapon.id) {
				disabled = (int)v.size();
			}
			else if (objects[i].id == stats->weapon.id) {
				highlight = (int)v.size();
			}
			p.first = objects[i].quantity;
			p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(objects[i].name));
			v.push_back(p);
			inventory_indices.push_back(i);
			descriptions.push_back(objects[i].description);
		}
	}

	if (v.size() == 0) {
		list->set_parent(NULL);
		delete list;
		list = NULL;
		if (gui != NULL) {
			gui->focus_something();
		}
		Widget_Label *empty_label = new Widget_Label(GLOBALS->game_t->translate(1494)/* Originally: No weapons */, -1);
		empty_label->set_padding_top(10);
		empty_label->set_centre_x(true);
		empty_label->set_break_line(true);
		empty_label->set_parent(window);
		if (gui) {
			gui->layout();
		}
	}
	else {
		list->set_items(v);
		list->set_descriptions(descriptions);
		int selected = list->get_selected();
		if (selected >= (int)v.size()) {
			list->set_selected(selected-1);
		}
		for (size_t i = 0; i < v.size(); i++) {
			list->set_highlight((int)i, false);
			list->set_disabled((int)i, false);
		}
		if (highlight != -1) {
			list->set_highlight(highlight, true);
		}
		if (disabled != -1) {
			list->set_disabled(disabled, true);
		}
	}

	if (list != NULL && top >= 0 && selected >= 0) {
		list->set_top(top);
		list->set_selected(selected);
	}
}

//--

Armour_GUI::Armour_GUI(int character_index, int top, int selected) :
	Main_Menu_GUI(character_index)
{
	if (INSTANCE->stats.size() > 1) {
		wedge::set_onscreen_controller_b3_enabled(true, M3_GLOBALS->key_switch);
	}
	else {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
	}
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = true;
	has_right_gui = true;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	Widget *column1 = new Widget(TGUI_Widget::FIT_Y, 1.0f);
	column1->set_parent(window);

	gfx::Image *pic = M3_GLOBALS->mini_profile_images[character_index];

	profile_pic = new Widget_Image(pic, false);
	profile_pic->set_shadow_colour(shim::palette[24]);
	profile_pic->set_padding_top(4);
	profile_pic->set_padding_left(4);
	status_label = new Widget_Label("", -1);
	status_label->set_padding_top(2);
	status_label->set_padding_left(5);

	profile_pic->set_parent(column1);
	status_label->set_parent(column1);

	Widget_Label *l1 = new Widget_Label(GLOBALS->game_t->translate(1495)/* Originally: Armour */, -1);
	Widget_Label *l2 = new Widget_Label(GLOBALS->game_t->translate(1496)/* Originally: Defense */, -1);
	
	l2->set_break_line(true);

	l1->set_padding_left(3);
	l2->set_padding_left(3);

	Widget *labels1 = new Widget();
	labels1->set_break_line(true);
	labels1->set_parent(column1);

	l1->set_parent(labels1);
	l2->set_parent(labels1);

	Widget *labels2 = new Widget();
	labels2->set_parent(column1);

	armour_label = new Widget_Label("", -1);
	armour_label->set_padding_left(5);
	armour_label->set_parent(labels2);

	defense_label = new Widget_Label("", -1);
	defense_label->set_break_line(true);
	defense_label->set_padding_left(5);
	defense_label->set_parent(labels2);

	list = new Widget_Quantity_List(1.0f, shim::font->get_height() * 3);
	list->set_break_line(true);
	list->set_padding_left(3);
	list->set_padding_right(3);
	list->set_parent(window);
	list->set_reserved_size(SCR_H - (3.0f * shim::font->get_height() + 4));

	set_text(top, selected);

	gui = new TGUI(window, window_w, window_h);
}

Armour_GUI::~Armour_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Armour_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (INSTANCE->stats.size() > 1 && // Have Tiggy
		((event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_switch) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_switch))
	) {
		if (next_character_gui == NULL && next_gui == NULL) {
			int next_index = character_index == 0 ? 1 : 0;
			Armour_GUI *new_gui = new Armour_GUI(next_index, list == NULL ? -1 : list->get_top(), list == NULL ? -1 : list->get_selected());
			set_next_character_gui(new_gui, false);
		}
	}
	else if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_LEFT) {
				Weapons_GUI *new_gui = new Weapons_GUI(character_index, -1, -1);
				set_next_gui(new_gui, true);
			}
			else if (event->focus.type == TGUI_FOCUS_RIGHT) {
				if (M3_INSTANCE->num_vampires() > 0) {
					Vampires_GUI *new_gui = new Vampires_GUI(character_index);
					set_next_gui(new_gui, false);
				}
				else {
					Discard_GUI *new_gui = new Discard_GUI(character_index);
					set_next_gui(new_gui, false);
				}
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Armour_GUI::update()
{
	if (list != NULL) {
		maybe_show_third_button_help();
	}

	int start_defense = 0;
	start_defense += stats->base.fixed.defense;
	start_defense += stats->armour.stats.defense;
	start_defense += stats->weapon.stats.defense;

	int end_defense = -1;

	if (list != NULL) {
		int pressed;
		if ((pressed = list->pressed()) >= 0) {
			wedge::Armour a = INSTANCE->inventory.get_all()[inventory_indices[pressed]].id;
			if (stats->armour.id == a) {
				stats->armour = wedge::Armour_Stats();
			}
			else {
				stats->armour = wedge::Armour_Stats(a);
			}
			set_text(-1, -1);
		}

		if (list->get_selected() >= 0) {
			wedge::Armour a = INSTANCE->inventory.get_all()[inventory_indices[list->get_selected()]].id;
			if (stats->armour.id != a) {
				end_defense = stats->base.fixed.defense;
				end_defense += wedge::Armour_Stats(a).stats.defense;
				end_defense += stats->weapon.stats.defense;
			}
		}
	}

	if (end_defense == -1) {
		defense_label->set_text(util::itos(start_defense));
	}
	else {
		std::string c;
		if (end_defense < start_defense) {
			c = "|09";
		}
		else if (end_defense > start_defense) {
			c = "|0d";
		}	
		defense_label->set_text(util::itos(start_defense) + " → " + c + util::itos(end_defense));
	}

	Main_Menu_GUI::update();
}

void Armour_GUI::set_text(int top, int selected)
{
	status_label->set_text(get_status_name(stats->base.hp, stats->base.status));
	status_label->set_text_colour(get_status_colour(stats->base.hp, stats->base.status));

	wedge::Object armour = OBJECT->make_object(wedge::OBJECT_ARMOUR, stats->armour.id, 1);
	armour_label->set_text(armour.name == "" ? "" : GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(armour.name)));

	inventory_indices.clear();

	std::vector< std::pair<int, std::string> > v;
	std::vector<std::string> descriptions;

	wedge::Object *objects = INSTANCE->inventory.get_all();

	int highlight = -1;
	int disabled = -1;

	for (int i = 0; i < wedge::Inventory::MAX_OBJECTS; i++) {
		std::pair<int, std::string> p;
		if (objects[i].type == wedge::OBJECT_ARMOUR) {
			int other_player_index = 1 - character_index;
			if (objects[i].quantity == 1 && objects[i].id == INSTANCE->stats[other_player_index].armour.id) {
				disabled = (int)v.size();
			}
			else if (objects[i].id == stats->armour.id) {
				highlight = (int)v.size();
			}
			p.first = objects[i].quantity;
			p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(objects[i].name));
			v.push_back(p);
			inventory_indices.push_back(i);
			descriptions.push_back(objects[i].description);
		}
	}

	if (v.size() == 0) {
		list->set_parent(NULL);
		delete list;
		list = NULL;
		if (gui != NULL) {
			gui->focus_something();
		}
		Widget_Label *empty_label = new Widget_Label(GLOBALS->game_t->translate(1497)/* Originally: No armour */, -1);
		empty_label->set_padding_top(10);
		empty_label->set_centre_x(true);
		empty_label->set_break_line(true);
		empty_label->set_parent(window);
		if (gui) {
			gui->layout();
		}
	}
	else {
		list->set_items(v);
		list->set_descriptions(descriptions);
		int selected = list->get_selected();
		if (selected >= (int)v.size()) {
			list->set_selected(selected-1);
		}
		for (size_t i = 0; i < v.size(); i++) {
			list->set_highlight((int)i, false);
			list->set_disabled((int)i, false);
		}
		if (highlight != -1) {
			list->set_highlight(highlight, true);
		}
		if (disabled != -1) {
			list->set_disabled(disabled, true);
		}
	}

	if (list != NULL && top >= 0 && selected >= 0) {
		list->set_top(top);
		list->set_selected(selected);
	}
}

//--

Vampires_GUI::Vampires_GUI(int character_index) :
	Main_Menu_GUI(character_index)
{
	wedge::set_onscreen_controller_b3_enabled(false, -1);
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = true;
	has_right_gui = true;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	Widget *column1 = new Widget(TGUI_Widget::FIT_Y, 1.0f);
	column1->set_padding_top(3);
	column1->set_parent(window);

	Widget_Label *l1 = new Widget_Label(GLOBALS->game_t->translate(1498)/* Originally: Cost (HP) */, -1);
	Widget_Label *l2 = new Widget_Label(GLOBALS->game_t->translate(1499)/* Originally: Cost (MP) */, -1);
	
	l2->set_break_line(true);

	l1->set_padding_left(3);
	l2->set_padding_left(3);

	Widget *labels1 = new Widget();
	labels1->set_break_line(true);
	labels1->set_parent(column1);

	l1->set_parent(labels1);
	l2->set_parent(labels1);

	Widget *labels2 = new Widget();
	labels2->set_parent(column1);

	hp_label = new Widget_Label("", -1);
	hp_label->set_padding_left(5);
	hp_label->set_parent(labels2);

	mp_label = new Widget_Label("", -1);
	mp_label->set_break_line(true);
	mp_label->set_padding_left(5);
	mp_label->set_parent(labels2);

	list = new Widget_Vampire_List(1.0f, shim::font->get_height() * 4);
	list->set_break_line(true);
	list->set_padding_left(3);
	list->set_padding_right(3);
	list->set_parent(window);
	list->set_reserved_size(SCR_H - (4.0f * shim::font->get_height() + 4));

	set_text();

	gui = new TGUI(window, window_w, window_h);
}

Vampires_GUI::~Vampires_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Vampires_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_LEFT) {
				Armour_GUI *new_gui = new Armour_GUI(character_index, -1, -1);
				set_next_gui(new_gui, true);
			}
			else if (event->focus.type == TGUI_FOCUS_RIGHT) {
				Discard_GUI *new_gui = new Discard_GUI(character_index);
				set_next_gui(new_gui, false);
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Vampires_GUI::update()
{
	std::string vampire = M3_INSTANCE->vampire(list->get_selected());

	int hp, mp;

	get_vampire_cost(vampire, hp, mp);

	hp_label->set_text(util::itos(hp));
	mp_label->set_text(util::itos(mp));
	
	Main_Menu_GUI::update();
}

void Vampires_GUI::set_text()
{
	std::vector< std::pair<std::string, std::string> > v;

	for (int i = 0; i < M3_INSTANCE->num_vampires(); i++) {
		std::pair<std::string, std::string> p;
		std::string vampire = M3_INSTANCE->vampire(i);
		int hp, mp;
		get_vampire_cost(vampire, hp, mp);
		p.first = util::itos(hp) + "/" + util::itos(mp);
		//p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(vampire));
		p.second = vampire;
		v.push_back(p);
	}

	list->set_items(v);
	int sel = list->get_selected();
	if (sel >= (int)v.size()) {
		list->set_selected(sel-1);
	}

	for (size_t i = 0; i < v.size(); i++) {
		list->set_disabled((int)i, true);
	}
}

//--

Discard_GUI::Discard_GUI(int character_index) :
	Main_Menu_GUI(character_index)
{
	wedge::set_onscreen_controller_b3_enabled(false, -1);
	if (M3_GLOBALS->hide_onscreen_settings_button == false && can_show_settings()) {
		wedge::set_onscreen_controller_b4_enabled(true, M3_GLOBALS->key_b4);
	}
	
	has_left_gui = true;
	has_right_gui = false;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	list = new Widget_Quantity_List(1.0f, shim::font->get_height() * 5);
	list->set_break_line(true);
	list->set_padding_top(5);
	list->set_padding_left(3);
	list->set_padding_right(3);
	list->set_parent(window);
	list->set_reserved_size(SCR_H - (5.0f * shim::font->get_height() + 4));

	set_text();

	gui = new TGUI(window, window_w, window_h);
}

Discard_GUI::~Discard_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b4_enabled(false, -1);
	}
}

void Discard_GUI::handle_event(TGUI_Event *event)
{
	if (GLOBALS->dialogue_active(MENU)) {
		return;
	}

	if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (event->focus.type == TGUI_FOCUS_LEFT) {
				if (M3_INSTANCE->num_vampires() > 0) {
					Vampires_GUI *new_gui = new Vampires_GUI(character_index);
					set_next_gui(new_gui, true);
				}
				else {
					Armour_GUI *new_gui = new Armour_GUI(character_index, -1, -1);
					set_next_gui(new_gui, true);
				}
			}
		}
	}
	else {
		Main_Menu_GUI::handle_event(event);
	}
}

void Discard_GUI::update()
{
	if (list != NULL) {
		maybe_show_third_button_help();
	}

	if (list != NULL) {
		int pressed;
		if ((pressed = list->pressed()) >= 0) {
			wedge::Object &obj = INSTANCE->inventory.get_all()[inventory_indices[pressed]];
			userdata.gui = this;
			userdata.item_index = inventory_indices[pressed];
			Get_Number_GUI *quantity_gui = new Get_Number_GUI(GLOBALS->game_t->translate(1500)/* Originally: Discard how many? */, obj.quantity+1, 0, discard_callback, &userdata);
			shim::guis.push_back(quantity_gui);
		}
	}
	
	Main_Menu_GUI::update();
}

void Discard_GUI::set_text()
{
	inventory_indices.clear();

	std::vector< std::pair<int, std::string> > v;
	std::vector<std::string> descriptions;

	wedge::Object *objects = INSTANCE->inventory.get_all();

	std::vector<int> highlight;

	for (int i = 0; i < wedge::Inventory::MAX_OBJECTS; i++) {
		std::pair<int, std::string> p;
		if (objects[i].type != wedge::OBJECT_SPECIAL && objects[i].type != wedge::OBJECT_NONE && (objects[i].type != wedge::OBJECT_ITEM || is_scroll(objects[i].id) == false)) {
			if (objects[i].type == wedge::OBJECT_WEAPON) {
				bool weapon_equipped = false;
				for (size_t j = 0; j < INSTANCE->stats.size(); j++) {
					if (objects[i].id == INSTANCE->stats[j].weapon.id) {
						weapon_equipped = true;
						break;
					}
				}
				if (weapon_equipped) {
					highlight.push_back((int)v.size());
				}
			}
			else if (objects[i].type == wedge::OBJECT_ARMOUR) {
				bool armour_equipped = false;
				for (size_t j = 0; j < INSTANCE->stats.size(); j++) {
					if (objects[i].id == INSTANCE->stats[j].armour.id) {
						armour_equipped = true;
						break;
					}
				}
				if (armour_equipped) {
					highlight.push_back((int)v.size());
				}
			}

			p.first = objects[i].quantity;
			p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(objects[i].name));
			v.push_back(p);
			inventory_indices.push_back(i);
			descriptions.push_back(objects[i].description);
		}
	}

	if (v.size() == 0) {
		list->set_parent(NULL);
		delete list;
		list = NULL;
		if (gui != NULL) {
			gui->focus_something();
		}
		Widget_Label *empty_label = new Widget_Label(GLOBALS->game_t->translate(1485)/* Originally: No items */, -1);
		empty_label->set_padding_top(10);
		empty_label->set_centre_x(true);
		empty_label->set_break_line(true);
		empty_label->set_parent(window);
		if (gui) {
			gui->layout();
		}
	}
	else {
		list->set_items(v);
		list->set_descriptions(descriptions);
		int selected = list->get_selected();
		if (selected >= (int)v.size()) {
			list->set_selected(selected-1);
		}

		for (size_t i = 0; i < v.size(); i++) {
			list->set_highlight((int)i, false);
		}

		for (size_t i = 0; i < highlight.size(); i++) {
			list->set_highlight(highlight[i], true);
		}
	}
}
