#ifdef TVOS
#include <Nooskewl_Shim/ios.h>
#endif

#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>

#include "general.h"
#include "globals.h"
#include "gui.h"
#include "save_slot.h"

static void erase_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = static_cast<Yes_No_GUI::Callback_Data *>(data);
	if (d->cancelled == false && d->choice) {
		Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
		gui->erase();
	}
}

static void load_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = static_cast<Yes_No_GUI::Callback_Data *>(data);
	if (d->cancelled == false && d->choice) {
		Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
		gui->load();
	}
	else if (d->cancelled == false && d->choice == false) {
		std::string text = GLOBALS->game_t->translate(1507)/* Originally: Erase this game? */;
		Yes_No_GUI *yes_no_gui = new Yes_No_GUI(text, true, erase_callback, d->userdata);
		yes_no_gui->set_selected(false);
		shim::guis.push_back(yes_no_gui);
	}
}

static void overwrite_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = static_cast<Yes_No_GUI::Callback_Data *>(data);
	if (d->cancelled == false && d->choice) {
		Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
		gui->load();
	}
}

Save_Slot_GUI::Save_Slot_GUI(bool is_save, int start_selection, util::Callback callback, void *callback_data, bool is_auto) :
	callback(callback),
	callback_data(callback_data),
	is_save(is_save),
	is_auto(is_auto),
	changing_guis(false)
{
	caption = GLOBALS->game_t->translate(1508)/* Originally: Choose a save slot: */;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	list = new Widget_Save_Game_List(0.9f, shim::font->get_height() * NUM_SLOTS);
	list->set_centre_x(true);
	list->set_centre_y(true);
	if (is_save) {
		list->set_padding_top(int(GLOBALS->bold_font->get_height()));
	}
	list->set_text_shadow_colour(shim::black);
	list->set_parent(modal_main_widget);

	set_text();
	list->set_selected(start_selection);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Save_Slot_GUI::~Save_Slot_GUI()
{
}

void Save_Slot_GUI::handle_event(TGUI_Event *event)
{
	if (fading_in || fading_out || changing_guis) {
		return;
	}

	// Taken from Menu drawing...
	int y = shim::screen_size.h - (shim::screen_size.h * 0.05f + shim::font->get_height());
	int yy = y - (M3_GLOBALS->selection_arrow->size.h - GLOBALS->bold_font->get_height()) / 2;
	int x;
	util::Point<int> arrow_pos;
	util::Point<int> arrow_end;
	if (is_auto) {
		x = shim::screen_size.w/8-M3_GLOBALS->selection_arrow->size.w/2;
		arrow_pos = util::Point<int>(x, yy);
		arrow_end = arrow_pos + util::Point<int>(M3_GLOBALS->selection_arrow->size.w+shim::font->get_text_width(GLOBALS->game_t->translate(1643)/* Originally: Saves */)+2, M3_GLOBALS->selection_arrow->size.h-1);
	}
	else {
		x = shim::screen_size.w*7/8-M3_GLOBALS->selection_arrow->size.w/2;
		arrow_end = util::Point<int>(x+M3_GLOBALS->selection_arrow->size.w-1, yy+M3_GLOBALS->selection_arrow->size.h-1);
		arrow_pos = arrow_end - util::Point<int>(M3_GLOBALS->selection_arrow->size.w+shim::font->get_text_width(GLOBALS->game_t->translate(1644)/* Originally: Autosaves */)+2, M3_GLOBALS->selection_arrow->size.h-1);
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)) {
		M3_GLOBALS->button->play(false);
		Callback_Data d;
		d.slot = -1;
		d.userdata = callback_data;
		callback(&d);
		fading_out = true;
		fade_start = GET_TICKS();
	}
	else if (is_save == false && is_auto && event->type == TGUI_FOCUS && event->focus.type == TGUI_FOCUS_LEFT) {
		change_guis();
	}
	else if (is_save == false && is_auto == false && event->type == TGUI_FOCUS && event->focus.type == TGUI_FOCUS_RIGHT) {
		change_guis();
	}
	else if (is_save == false && is_auto && event->type == TGUI_MOUSE_DOWN && cd::box_box(arrow_pos, arrow_end, util::Point<int>(event->mouse.x, event->mouse.y), util::Point<int>(event->mouse.x+1, event->mouse.y+1))) {
		change_guis();
	}
	else if (is_save == false && is_auto == false && event->type == TGUI_MOUSE_DOWN && cd::box_box(arrow_pos, arrow_end, util::Point<int>(event->mouse.x, event->mouse.y), util::Point<int>(event->mouse.x+1, event->mouse.y+1))) {
		change_guis();
	}
	else {
		gui::GUI::handle_event(event);
	}
}

void Save_Slot_GUI::update()
{
	Sliding_Menu_GUI::update();

	if (changing_guis) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - next_gui_start;
		if (elapsed >= Menu_GUI::TRANSITION_TIME) {
			shim::guis.push_back(next_gui);
			exit();
		}
	}

	if (fading_in || fading_out) {
		return;
	}

	if ((pressed = list->pressed()) >= 0) {
		if (exists[pressed]) {
			if (is_save) {
				std::string text = GLOBALS->game_t->translate(1509)/* Originally: Overwrite this game? */;
				Yes_No_GUI *yes_no_gui = new Yes_No_GUI(text, true, overwrite_callback, this);
				shim::guis.push_back(yes_no_gui);
			}
			else {
				std::string text = GLOBALS->game_t->translate(1510)/* Originally: Load this game? */;
				Yes_No_GUI *yes_no_gui = new Yes_No_GUI(text, true, load_callback, this);
				shim::guis.push_back(yes_no_gui);
			}
		}
		else {
			if (corrupt[pressed]) {
				std::string text = GLOBALS->game_t->translate(1507)/* Originally: Erase this game? */;
				Yes_No_GUI *yes_no_gui = new Yes_No_GUI(text, true, erase_callback, this);
				yes_no_gui->set_selected(false);
				shim::guis.push_back(yes_no_gui);
			}
			else {
				load();
			}
		}
	}
}

void Save_Slot_GUI::set_text()
{
	std::vector<std::string> times;
	std::vector< std::pair<int, int> > experience;

	for (int i = 0; i < NUM_SLOTS; i++) {
		util::JSON *json = NULL;
		try {
			if (is_auto) {
				json = load_savegame(i, "auto");
			}
			else {
				json = load_savegame(i);
			}
		}
		catch (util::FileNotFoundError e) {
			times.push_back("");
			experience.push_back(std::pair<int, int>(-1, -1));
			exists[i] = false;
			corrupt[i] = false;
			continue;
		}
		catch (util::Error e) {
			util::debugmsg("Corrupt save, error=%s\n", e.error_message.c_str());
			times.push_back("");
			experience.push_back(std::pair<int, int>(-2, -2));
			exists[i] = false;
			corrupt[i] = true;
			continue;
		}
		util::JSON::Node *root = json->get_root();
		if (root) {
			util::JSON::Node *n;
			n = root->find("\"players\"");
			if (n) {
				if (n->children.size() == 2/*can't use MAX_PARTY here, INSTANCE doesn't exist yet*/) {
					util::JSON::Node *eny = n->children[0];
					util::JSON::Node *tiggy = n->children[1];
					int eny_exp, tiggy_exp;
					n = eny->find("\"experience\"");
					if (n) {
						eny_exp = wedge::json_to_integer(n);
						n = tiggy->find("\"experience\"");
						if (n) {
							tiggy_exp = wedge::json_to_integer(n);
							n = root->find("\"game\"");
							if (n) {
								n = n->find("\"play_time\"");
								if (n) {
									times.push_back(play_time_to_string(wedge::json_to_integer(n)));
									experience.push_back(std::pair<int, int>(eny_exp, tiggy_exp));
									exists[i] = true;
								}
								else {
									exists[i] = false;
								}
							}
							else {
								exists[i] = false;
							}
						}
						else {
							exists[i] = false;
						}
					}
					else {
						exists[i] = false;
					}
				}
				else {
					exists[i] = false;
				}
			}
			else {
				exists[i] = false;
			}
		}
		else {
			exists[i] = false;
		}
		if (exists[i] == false) {
			corrupt[i] = true;
			times.push_back("");
			experience.push_back(std::pair<int, int>(-2, -2));
		}
		else {
			corrupt[i] = false;
		}

		delete json;
	}

	static_cast<Widget_Save_Game_List *>(list)->set_items(times, experience);
}

void Save_Slot_GUI::load()
{
	Callback_Data d;
	d.slot = pressed;
	d.exists = exists[pressed];
	d.userdata = callback_data;
	d.is_auto = is_auto;
	callback(&d);
	transition = true;
	exit();
}

void Save_Slot_GUI::erase()
{
	std::string filename = save_filename(pressed, is_auto ? "auto" : "save");
#ifdef TVOS
	util::tvos_delete_file(filename);
#else
	remove(filename.c_str());
#endif
	set_text();
}

void Save_Slot_GUI::draw()
{
	if (changing_guis) {
		float p = get_change_p();

		gfx::get_matrices(old_mv, old_p);

		if (is_auto) {
			float p2 = -(1.0f - p);
			glm::mat4 m;
			m = glm::translate(glm::mat4(), glm::vec3(p2*shim::screen_size.w, 0.0f, 0.0f));
			m = old_mv * m;
			gfx::set_matrices(m, old_p);
			gfx::update_projection();
			next_gui->draw_back();
			next_gui->draw();
			next_gui->draw_fore();

			m = glm::translate(glm::mat4(), glm::vec3(p*shim::screen_size.w, 0.0f, 0.0f));
			m = old_mv * m;
			gfx::set_matrices(m, old_p);
			gfx::update_projection();
		}
		else {
			float p2 = 1.0f - p;
			glm::mat4 m;
			m = glm::translate(glm::mat4(), glm::vec3(p2*shim::screen_size.w, 0.0f, 0.0f));
			m = old_mv * m;
			gfx::set_matrices(m, old_p);
			gfx::update_projection();
			next_gui->draw_back();
			next_gui->draw();
			next_gui->draw_fore();

			float p3 = -p;
			m = glm::translate(glm::mat4(), glm::vec3(p3*shim::screen_size.w, 0.0f, 0.0f));
			m = old_mv * m;
			gfx::set_matrices(m, old_p);
			gfx::update_projection();
		}
	}

	Sliding_Menu_GUI::draw();
}


void Save_Slot_GUI::draw_fore()
{
	Sliding_Menu_GUI::draw_fore();

	if (fading_in || fading_out || is_save) {
		return;
	}
	
	bool draw_arrows = (GET_TICKS() % 500) < 250 && shim::guis.size() > 0 && this == shim::guis.back() && changing_guis == false;

	if (draw_arrows) {
		// Taken from Menu drawing...
		int y = shim::screen_size.h - (shim::screen_size.h * 0.05f + shim::font->get_height());
		int yy = y - (M3_GLOBALS->selection_arrow->size.h - GLOBALS->bold_font->get_height()) / 2;

		if (is_auto) {
			int x = shim::screen_size.w/8-M3_GLOBALS->selection_arrow->size.w/2;
			M3_GLOBALS->selection_arrow->draw(util::Point<int>(x, yy));
			M3_GLOBALS->selection_arrow->end_batch();
				
			shim::font->enable_shadow(shim::palette[27], gfx::Font::FULL_SHADOW);
			std::string text = GLOBALS->game_t->translate(1643)/* Originally: Saves */;
			shim::font->draw(shim::palette[14], text, util::Point<float>(x+2+M3_GLOBALS->selection_arrow->size.w, y+0.5f));
			shim::font->disable_shadow();
		}
		else {
			int x = shim::screen_size.w*7/8-M3_GLOBALS->selection_arrow->size.w/2;
			M3_GLOBALS->selection_arrow->draw(util::Point<int>(x, yy), gfx::Image::FLIP_H);
			M3_GLOBALS->selection_arrow->end_batch();
				
			shim::font->enable_shadow(shim::palette[27], gfx::Font::FULL_SHADOW);
			std::string text = GLOBALS->game_t->translate(1644)/* Originally: Autosaves */;
			shim::font->draw(shim::palette[14], text, util::Point<float>(x-shim::font->get_text_width(text)-2, y+0.5f));
			shim::font->disable_shadow();
		}

		gfx::Font::end_batches();
	}

	if (changing_guis) {
		gfx::set_matrices(old_mv, old_p);
		gfx::update_projection();
	}
}

void Save_Slot_GUI::change_guis()
{
	if (changing_guis) {
		return;
	}

	next_gui = new Save_Slot_GUI(is_save, 0, callback, callback_data, !is_auto);
	next_gui->set_caption(is_auto ? GLOBALS->game_t->translate(1508)/* Originally: Choose a save slot: */ : GLOBALS->game_t->translate(1709)/* Originally: Choose an autosave: */);
	next_gui->set_fading(false, false);
	next_gui_start = GET_TICKS();
	changing_guis = true;

	shim::widget_mml->play(false);
}

float Save_Slot_GUI::get_change_p()
{
	Uint32 now = GET_TICKS();
	float p = (now-next_gui_start) / (float)Menu_GUI::TRANSITION_TIME;

	if (p > 1.0f) {
		p = 1.0f;
	}

	p = p * p;

	return p;
}

#ifdef _WIN32
void *Save_Slot_GUI::operator new(size_t i)
{
	return _mm_malloc(i,16);
}

void Save_Slot_GUI::operator delete(void* p)
{
	_mm_free(p);
}
#endif
