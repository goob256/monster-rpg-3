#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/input.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/omnipresent.h>

#include "area_game.h"
#include "dialogue.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "gui_drawing_hook.h"
#include "menu.h"
#include "save_slot.h"
#include "settings.h"

#ifdef ANDROID
#include <jni.h>
#elif defined TVOS
#include "ios.h"
#endif

static void title_callback(void *data)
{
	Save_Slot_GUI::Callback_Data *d = static_cast<Save_Slot_GUI::Callback_Data *>(data);
	Title_GUI *gui = static_cast<Title_GUI *>(d->userdata);
	if (d->slot >= 0) {
		gui->set_stop_drawing(true);
		gui->go(d->slot, d->exists, d->is_auto);
	}
	else { // returning to title screen...
		gui->set_transition(true);
#if defined IOS || defined ANDROID
		wedge::set_onscreen_controller_b2_enabled(false);
		OMNIPRESENT->set_hide_red_triangle(true);
#ifdef TVOS
		shim::pass_menu_to_os = true;
#endif
#endif
	}
}

Title_GUI::Title_GUI() :
	update_count(0),
#if defined IOS || defined ANDROID
	in_settings(false),
#endif
	stop_drawing(false)
{
	transition = true;

#if defined IOS || defined ANDROID
	wedge::set_onscreen_controller_b2_enabled(false);
	OMNIPRESENT->set_hide_red_triangle(true);
#endif
#ifdef TVOS
	shim::pass_menu_to_os = true;
#endif

	retrying_boss = GLOBALS->retry_boss;
	
	if (wedge::globals->gameover->is_playing()) {
		// We died, we want to save the play time though
		save_play_time();
		wedge::globals->gameover->stop();
	}

	if (retrying_boss == false) {
		audio::play_music("music/title.mml");
	}

	logo = new gfx::Image("misc/logo.tga");

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	container = new TGUI_Widget();
	container->set_centre_x(true);
	container->set_float_bottom(true);
	container->set_parent(modal_main_widget);

	new_game_button = new Widget_Main_Menu_Text_Button("");
	new_game_button->set_padding_right(3);
	new_game_button->set_parent(container);

	load_button = new Widget_Main_Menu_Text_Button("");
	load_button->set_padding_right(3);
	load_button->set_parent(container);

#if !defined IOS && !defined ANDROID
	exit_button = new Widget_Main_Menu_Text_Button("");
	exit_button->set_padding_right(3);
	exit_button->set_parent(container);
#endif

	settings_button = new Widget_Main_Menu_Icon_Button(new gfx::Image("ui/gear.tga"), true);
	settings_button->set_parent(container);
	
	container->set_padding_bottom(int(shim::screen_size.h * 0.18f));

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(new_game_button);

	start_scale = START_LOGO_SCALE;
	
	reload_text();
}

Title_GUI::~Title_GUI()
{
	delete logo;
}

void Title_GUI::resize(util::Size<int> size)
{
	container->set_padding_bottom(int(size.h * 0.18f));
	gui->resize(size.w, size.h);
}

void Title_GUI::draw()
{
	if (stop_drawing) {
		return;
	}

	if (retrying_boss) {
		gfx::clear(GLOBALS->gameover_fade_colour);
		return;
	}

	SDL_Colour *colours = start_bander(num_bands(shim::screen_size.h), shim::palette[27], shim::palette[24]);
	gfx::draw_filled_rectangle(colours, util::Point<int>(0, 0), shim::screen_size);
	end_bander();

	const int phase = 10000;
	const int half_phase = phase / 2;
	Uint32 ticks = SDL_GetTicks() % phase;
	if (ticks >= half_phase) {
		ticks = half_phase - (ticks-half_phase);
	}
	float p = (float)ticks / half_phase;
	p = p * p;
	float scale = 1.0f + 0.05f * p;

	scale *= start_scale;

	glm::mat4 mv, modelview, proj;
	gfx::get_matrices(modelview, proj);
	mv = modelview;
	mv = glm::translate(mv, glm::vec3(shim::screen_size.w/2.0f, shim::screen_size.h/2.0f-shim::screen_size.h*0.15f, 0.0f));
	mv = glm::scale(mv, glm::vec3(scale, scale, 1.0f));
	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	ticks = SDL_GetTicks() % phase;
	bool pulse;
	if (ticks < 500) {
		pulse = true;
		start_pulse_brighten(0.5f, false, true);
	}
	else {
		pulse = false;
	}

	logo->draw(
		util::Point<int>(
			-logo->size.w/2,
			-logo->size.h/2
		)
	);

	if (pulse) {
		end_pulse_brighten();
	}
	
	gfx::set_matrices(modelview, proj);
	gfx::update_projection();

	shim::font->draw(shim::white, GLOBALS->game_t->translate(1642), util::Point<int>(shim::screen_size.w/2, shim::screen_size.h-shim::font->get_height()*1.25f), true, true);

	GUI::draw();
}

void Title_GUI::update_background()
{
#if defined IOS || defined ANDROID
	if (in_settings && settings_active() == false) {
		in_settings = false;
		wedge::set_onscreen_controller_b2_enabled(false);
		OMNIPRESENT->set_hide_red_triangle(true);
#ifdef TVOS
		shim::pass_menu_to_os = true;
#endif
	}
#endif
}

void Title_GUI::update()
{
	if (M3_GLOBALS->terminate) {
		exit();
	}

	if (transitioning_in || transitioning_out) {
		return;
	}

	if (new_game_button != NULL && new_game_button->pressed()) {
		go(-1, false, false);
	}
	else if (load_button != NULL && load_button->pressed()) {
#if defined IOS || defined ANDROID
		wedge::set_onscreen_controller_b2_enabled(true);
		OMNIPRESENT->set_hide_red_triangle(false);
#endif
#ifdef TVOS
		shim::pass_menu_to_os = false;
#endif
		Save_Slot_GUI *gui = new Save_Slot_GUI(false, 0, title_callback, this);
		shim::guis.push_back(gui);

		transition = false;
	}
#if !defined IOS && !defined ANDROID
	else if (exit_button->pressed()) {
		M3_GLOBALS->terminate = true;
		exit();
	}
#endif
	else if (settings_button->pressed()) {
#if defined IOS || defined ANDROID
		in_settings = true;
		wedge::set_onscreen_controller_b2_enabled(true);
		OMNIPRESENT->set_hide_red_triangle(false);
#endif
#ifdef TVOS
		shim::pass_menu_to_os = false;
#endif
		Settings_GUI *gui = new Settings_GUI(true, Settings_GUI::LANGUAGE, 0);
		shim::guis.push_back(gui);
	}

	if (GLOBALS->retry_boss) {
		GLOBALS->retry_boss = false;
		M3_GLOBALS->loaded = true;
		SDL_RWops *memfile = SDL_RWFromMem((void *)GLOBALS->boss_save.c_str(), (int)GLOBALS->boss_save.length());
		util::JSON *json = new util::JSON(memfile);
		load_game(json);
		delete json;
		SDL_RWclose(memfile);
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		wedge::Map_Entity_Input_Step *meis = eny->get_input_step();
		/* This doesn't work well
		bool l = false;
		bool r = false;
		bool u = false;
		bool d = false;
		switch (GLOBALS->boss_press) {
			case wedge::DIR_N:
				u = true;
				break;
			case wedge::DIR_E:
				r = true;
				break;
			case wedge::DIR_S:
				d = true;
				break;
			case wedge::DIR_W:
				l = true;
				break;
			default:
				break;
		}
		if (l || r || u || d) {
			meis->set_stash(l, r, u, d);
			meis->repeat_pressed();
		}
		*/
		util::Point<int> pos = eny->get_position();
		switch (GLOBALS->boss_press) {
			case wedge::DIR_N:
				pos.y--;
				break;
			case wedge::DIR_E:
				pos.x++;
				break;
			case wedge::DIR_S:
				pos.y++;
				break;
			case wedge::DIR_W:
				pos.x--;
				break;
			default:
				break;
		}
		meis->set_path(pos, false);
#if defined IOS || defined ANDROID
		wedge::set_onscreen_controller_b2_enabled(true);
		OMNIPRESENT->set_hide_red_triangle(false);
#endif
#ifdef TVOS
		shim::pass_menu_to_os = false;
#endif
		exit();
	}
	else {
		GLOBALS->retried_boss = false;
	}

	if (update_count > TICKS_BEFORE_SCALE_DOWN) {
		int u = update_count - TICKS_BEFORE_SCALE_DOWN;
		u *= 1000 / shim::logic_rate;
		u = MIN(u, SCALE_IN_DURATION);
		float p = (float)u / SCALE_IN_DURATION;
		p = p * p;
		p = 1.0f - p;
		start_scale = 1.0f + (START_LOGO_SCALE-1) * p;
	}
	update_count++;
}

void Title_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

#if !defined IOS // Not allowed to terminate on iOS
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)) {
#if !defined ANDROID_XXX
		M3_GLOBALS->terminate = true;
		exit();
#else // Don't really terminate on Android, just background ourself
		JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
		jobject activity = (jobject)SDL_AndroidGetActivity();
		jclass clazz(env->GetObjectClass(activity));

		jmethodID method_id = env->GetMethodID(clazz, "moveTaskToBack", "(Z)Z");

		if (method_id != NULL) {
			env->CallBooleanMethod(activity, method_id, true);
		}

		env->DeleteLocalRef(activity);
		env->DeleteLocalRef(clazz);
#endif
	}
	else {
#endif
		gui::GUI::handle_event(event);
#if !defined IOS
	}
#endif
}

void Title_GUI::go(int slot, bool exists, bool is_auto)
{
	M3_GLOBALS->save_slot = is_auto ? -1 : slot;

	if (exists == false) {
		M3_GLOBALS->loaded = false;

		delete INSTANCE;
		INSTANCE = new Monster_RPG_3_Globals::Instance(NULL);
		INSTANCE->party_following_player = false;

		AREA = new Monster_RPG_3_Area_Game();
		AREA->start_area(NULL);
	}
	else {
		M3_GLOBALS->loaded = true;

		util::JSON *json = load_savegame(slot, is_auto ? "auto" : "save");
		load_game(json);
		delete json;
	}

#if defined IOS || defined ANDROID
	wedge::set_onscreen_controller_b2_enabled(true);
	OMNIPRESENT->set_hide_red_triangle(false);
#endif
#ifdef TVOS
	shim::pass_menu_to_os = false;
#endif

	exit();
}


void Title_GUI::load_game(util::JSON *json)
{
	util::JSON::Node *root = json->get_root();

	// Some post processing for backwards compatibility
	// --
	// Potion Plus and Heal Plus used to be Potion2 and Heal2. Item names aren't saved anymore, but spell names are.
	util::JSON::Node *players = root->find("\"players\"");
	if (players != NULL) {
		for (size_t i = 0; i < players->children.size(); i++) {
			util::JSON::Node *stats = players->children[i]->find("\"stats\"");
			if (stats != NULL) {
				util::JSON::Node *spells = stats->find("\"spells\"");
				if (spells != NULL) {
					for (size_t j = 0; j < spells->children.size(); j++) {
						if (spells->children[j]->value == "\"Heal2\"") {
							spells->children[j]->value = "\"Heal Plus\"";
						}
					}
				}
			}
		}
	}
	// --

	delete INSTANCE;
	INSTANCE = new Monster_RPG_3_Globals::Instance(root);

	AREA = new Monster_RPG_3_Area_Game();

	util::JSON::Node *saved = root->find("\"areas\"");
	wedge::Area *area = AREA->create_area(saved->children[0]);
	area->start();
	area->set_hooks(AREA->get_area_hooks(area->get_name(), area));

	for (size_t i = 0; i < saved->children.size(); i++) {
		INSTANCE->saved_levels[util::JSON::trim_quotes(saved->children[i]->key)] = saved->children[i]->to_string();
	}

	AREA->start_area(area);
}

bool Title_GUI::is_fullscreen()
{
	return true;
}

void Title_GUI::set_stop_drawing(bool stop)
{
	stop_drawing = stop;
}

void Title_GUI::reload_text()
{
#if defined IOS || defined ANDROID
	in_settings = false; // language config can dump is back here
#endif

	if (new_game_button != NULL) {
		new_game_button->set_text(GLOBALS->game_t->translate(1443)/* Originally: Start */);
	}
	if (load_button != NULL) {
		load_button->set_text(GLOBALS->game_t->translate(1444)/* Originally: Load */);
	}
#if !defined IOS && !defined ANDROID
	if (exit_button != NULL) {
		exit_button->set_text(GLOBALS->game_t->translate(1445)/* Originally: Exit */);
	}
#endif

	gui->layout();
}

//--

Menu_GUI::Menu_GUI(int character_index) :
	character_index(character_index),
	stats(&INSTANCE->stats[character_index]),
	next_character_gui(NULL),
	next_gui(NULL),
	has_left_gui(true),
	has_right_gui(true),
	done(false)
{
}

Menu_GUI::~Menu_GUI()
{
}

void Menu_GUI::draw()
{
	SDL_Colour *colours = start_bander(num_bands(shim::screen_size.h), shim::palette[17], shim::palette[19]);
	gfx::draw_filled_rectangle(colours, util::Point<int>(0, 0), shim::screen_size);
	end_bander();

	util::Point<float> o = get_offset();
	
	if (next_character_gui != NULL) {
		Uint32 now = GET_TICKS();
		float p = (now-character_transition_start) / (float)TRANSITION_TIME;
		if (p > 1.0f) {
			p = 1.0f;
		}

		p = p * p;

		TGUI *gui1, *gui2;

		if (p > 0.5f) {
			p = 1.0f - ((p - 0.5f) / 0.5f);
			gui1 = gui;
			gui2 = next_character_gui->gui;
		}
		else {
			p /= 0.5f;
			gui1 = next_character_gui->gui;
			gui2 = gui;
		}

		float scale = p * 0.5f + 0.5f;
		float inv_scale = (1.0f - scale) / 2.0f;
		util::Point<float> o2 = util::Point<float>(shim::screen_size.w, shim::screen_size.h) * inv_scale;
		
		float max_dist = shim::screen_size.w * 52 / 100.0f;
		
		if (next_character_gui_is_left == false) {
			p = -p;
		}

		if (gui1 == gui) {
			o2.x -= p * max_dist;
			o.x += p * max_dist;
		}
		else {
			o.x -= p * max_dist;
			o2.x += p * max_dist;
		}

		glm::mat4 modelview, proj;
		gfx::get_matrices(modelview, proj);

		glm::mat4 matrix = modelview;
		matrix = glm::translate(matrix, glm::vec3(o2.x, o2.y, 0.0f));
		matrix = glm::scale(matrix, glm::vec3(scale, scale, scale));

		shim::current_shader = M3_GLOBALS->darken_both_shader;
		shim::current_shader->use();
		shim::current_shader->set_float("brightness", 0.5f);
		gfx::set_matrices(matrix, proj);
		gfx::update_projection();
		gui1->draw();
		gfx::Font::end_batches();
		
		matrix = glm::translate(modelview, glm::vec3(o.x, o.y, 0.0f));

		shim::current_shader = shim::default_shader;
		shim::current_shader->use();
		gfx::set_matrices(matrix, proj);
		gfx::update_projection();
		gui2->draw();
		gfx::Font::end_batches();

		gfx::set_matrices(modelview, proj);
		gfx::update_projection();
	}
	else if (next_gui != NULL) {
		Uint32 now = GET_TICKS();
		float p = (now-gui_transition_start) / (float)TRANSITION_TIME;

		if (p > 1.0f) {
			p = 1.0f;
		}

		p = p * p;

		float x, x2;

		if (next_gui_is_left) {
			x = p * shim::screen_size.w;
			x2 = x - shim::screen_size.w;
		}
		else {
			x = -p * shim::screen_size.w;
			x2 = x + shim::screen_size.w;
		}

		glm::mat4 modelview, proj;
		gfx::get_matrices(modelview, proj);

		glm::mat4 matrix = modelview;
		matrix = glm::translate(matrix, glm::vec3(o.x+x, o.y, 0.0f));

		shim::current_shader = M3_GLOBALS->darken_both_shader;
		shim::current_shader->use();
		shim::current_shader->set_float("brightness", 0.5f);
		gfx::set_matrices(matrix, proj);
		gfx::update_projection();
		gui->draw();
		gfx::Font::end_batches();
		
		matrix = glm::translate(modelview, glm::vec3(o.x+x2, o.y, 0.0f));

		shim::current_shader = shim::default_shader;
		shim::current_shader->use();
		gfx::set_matrices(matrix, proj);
		gfx::update_projection();
		next_gui->gui->draw();
		gfx::Font::end_batches();

		gfx::set_matrices(modelview, proj);
		gfx::update_projection();
	}
	else {
		glm::mat4 modelview, proj;
		gfx::get_matrices(modelview, proj);
		glm::mat4 translate = glm::translate(modelview, glm::vec3(o.x, o.y, 0.0f));
		gfx::set_matrices(translate, proj);
		gfx::update_projection();
		gui->draw();
		gfx::Font::end_batches();
		gfx::set_matrices(modelview, proj);
		gfx::update_projection();
	}
}

void Menu_GUI::draw_fore()
{
	GUI::draw_fore();

	int y = shim::screen_size.h - (shim::screen_size.h * 0.05f + shim::font->get_height());

	bool draw_arrows = (GET_TICKS() % 500) < 250;

	if (draw_arrows) {
		int yy = y - (M3_GLOBALS->selection_arrow->size.h - GLOBALS->bold_font->get_height()) / 2;
		gfx::Image *arrow = M3_GLOBALS->selection_arrow;
		arrow->start_batch();
		if (has_left_gui) {
			arrow->draw(util::Point<int>(shim::screen_size.w/8-arrow->size.w/2, yy));
		}
		if (has_right_gui) {
			arrow->draw(util::Point<int>(shim::screen_size.w*7/8-arrow->size.w/2, yy), gfx::Image::FLIP_H);
		}
		arrow->end_batch();
	}
	
	gfx::Image *pic1 = M3_GLOBALS->mini_profile_images[ENY];
	gfx::Image *pic2 = M3_GLOBALS->mini_profile_images[TIGGY];

	if (INSTANCE->stats.size() > 1) { // have Tiggy
		shim::font->enable_shadow(shim::palette[27], gfx::Font::FULL_SHADOW);

		if (dynamic_cast<Progress_GUI *>(this) == NULL && dynamic_cast<Discard_GUI *>(this) == NULL && dynamic_cast<Vampires_GUI *>(this) == NULL && dynamic_cast<Map_GUI *>(this) == NULL) {
			int len;
			int total;
#if defined TVOS
			if (input::is_joystick_connected() == false) {
				len = M3_GLOBALS->play_pause->size.w + 2;
				total = len + pic1->size.w + pic2->size.w + 1;
				M3_GLOBALS->play_pause->draw(util::Point<float>(shim::screen_size.w/2-total/2, y+1.5f));
			}
			else
#endif
			{
#ifdef ANDROID
				std::string text = "/" + get_char_swap_text();
				len = shim::font->get_text_width(text);
				int len2 = M3_GLOBALS->play_pause->size.w;
				len += len2;
				total = len + pic1->size.w + pic2->size.w + 1;
				shim::font->draw(shim::palette[14], text, util::Point<float>(shim::screen_size.w/2-total/2+len2, y+0.5f));
				M3_GLOBALS->play_pause->draw(util::Point<float>(shim::screen_size.w/2-total/2, y+1.5f));
#else
				std::string text = get_char_swap_text();
				len = shim::font->get_text_width(text);
				total = len + pic1->size.w + pic2->size.w + 1;
				shim::font->draw(shim::palette[14], text, util::Point<float>(shim::screen_size.w/2-total/2, y+0.5f));
#endif
			}
			draw_shadowed_image(shim::palette[24], pic1, util::Point<int>(shim::screen_size.w/2-total/2+len, y+2), gfx::Font::DROP_SHADOW);
			draw_shadowed_image(shim::palette[24], pic2, util::Point<int>(shim::screen_size.w/2-total/2+len+pic1->size.w+1, y+2), gfx::Font::DROP_SHADOW);
		}

		shim::font->disable_shadow();
	}
}

std::string Menu_GUI::get_char_swap_text()
{
#if defined IOS && !defined TVOS
	return "";
#else
	bool joystick_connected = input::is_joystick_connected();

	std::string text;

	if (joystick_connected) {
		text += get_joystick_button_name(GLOBALS->joy_switch);
	}
	else {
		text += get_key_name(M3_GLOBALS->key_switch);
	}

	text += " ";

	return text;
#endif
}

void Menu_GUI::update()
{
	if (next_character_gui != NULL) {
		Uint32 now = GET_TICKS();
		float p = (now-character_transition_start) / (float)TRANSITION_TIME;
		if (p >= 1.0f) {
			shim::guis.push_back(next_character_gui);
			exit();
		}
	}
	else if (next_gui != NULL) {
		Uint32 now = GET_TICKS();
		float p = (now-gui_transition_start) / (float)TRANSITION_TIME;

		if (p >= 1.0f) {
			shim::guis.push_back(next_gui);
			exit();
		}
	}

	if (done) {
		if (next_character_gui != NULL) {
			delete next_character_gui;
		}
		if (next_gui != NULL) {
			delete next_gui;
		}
		exit();
	}
}

void Menu_GUI::handle_event(TGUI_Event *event)
{
	if (event->type == TGUI_MOUSE_AXIS || event->type == TGUI_MOUSE_DOWN || event->type == TGUI_MOUSE_UP) {
		util::Point<float> o = get_offset();
		event->mouse.x -= o.x;
		event->mouse.y -= o.y;
	}

	int font_height = shim::font->get_height();

	int ymin = shim::screen_size.h - (shim::screen_size.h * 0.05f + font_height * 1.5f);
	int ymax = ymin + font_height;

	if (event->type == TGUI_MOUSE_DOWN && event->mouse.y >= ymin && event->mouse.y < ymax) {
		SDL_Event ev;
		if (event->mouse.x < shim::screen_size.w/3) {
			ev.type = SDL_KEYDOWN;
			ev.key.keysym.sym = GLOBALS->key_l;
			ev.key.repeat = 0;
			SDL_PushEvent(&ev);
		}
		else if (event->mouse.x < shim::screen_size.w*2/3) {
			ev.type = SDL_KEYDOWN;
			ev.key.keysym.sym = M3_GLOBALS->key_switch;
			ev.key.repeat = 0;
			SDL_PushEvent(&ev);
		}
		else {
			ev.type = SDL_KEYDOWN;
			ev.key.keysym.sym = GLOBALS->key_r;
			ev.key.repeat = 0;
			SDL_PushEvent(&ev);
		}
	}
	else if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2 && event->keyboard.is_repeat == false) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2 && event->joystick.is_repeat == false)
	) {
		if (next_gui == NULL && next_character_gui == NULL) {
			done = true;
		}
	}
	else if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_b4) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b4)
	) {
		show_settings();
	}
	else {
		gui::GUI::handle_event(event);
	}
}

void Menu_GUI::resize(util::Size<int> size)
{
	size.w *= 0.9f;
	size.h *= 0.9f;
	gui->resize(size.w, size.h);
}

void Menu_GUI::set_next_character_gui(Menu_GUI *next_character_gui, bool next_character_gui_is_left)
{
	shim::widget_mml->play(false);
	this->next_character_gui = next_character_gui;
	this->next_character_gui_is_left = next_character_gui_is_left;
	character_transition_start = GET_TICKS();
}

void Menu_GUI::set_next_gui(Menu_GUI *next_gui, bool next_gui_is_left)
{
	shim::widget_mml->play(false);
	this->next_gui = next_gui;
	this->next_gui_is_left = next_gui_is_left;
	gui_transition_start = GET_TICKS();
}

util::Point<float> Menu_GUI::get_offset()
{
	util::Point<float> o;
	int w = gui->get_width();
	int h = gui->get_height();
	o.x = (shim::screen_size.w - w) / 2.0f;
	o.y = (shim::screen_size.h - h) / 2.0f;
	return o;
}

//--

Multiple_Choice_GUI::Multiple_Choice_GUI(bool tint_screen, std::string caption, std::vector<std::string> choices, int escape_choice, util::Callback callback, void *callback_data, int lines_to_show, int width, bool shrink_to_fit, Widget_List *custom_list) :
	caption(caption),
	callback(callback),
	callback_data(callback_data),
	exit_menu(false),
	escape_choice(escape_choice),
	lines_to_show(lines_to_show),
	shrink_to_fit(shrink_to_fit)
{
	transition = true;
	transition_is_shrink = true;

	int w = width;
	int h;
	int num_lines;

	get_height(w, h, num_lines);

	if (shrink_to_fit) {
		for (size_t i = 0; i < choices.size(); i++) {
			int choice_w = shim::font->get_text_width(choices[i]) + 1;
			w = MAX(w+1, choice_w);
		}
		w += Dialogue_Step::BORDER*2;
	}

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	
	if (tint_screen) {
		SDL_Colour background_colour = shim::black;
		background_colour.r *= 0.75;
		background_colour.g *= 0.75;
		background_colour.b *= 0.75;
		background_colour.a *= 0.75;
		modal_main_widget->set_background_colour(background_colour);
	}

	window = new Widget_Window(w + Dialogue_Step::BORDER*2, h + Dialogue_Step::BORDER * (caption == "" ? 2 : 4));
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(Dialogue_Step::BORDER);
	pad->set_parent(window);

	if (caption != "") {
		caption_label = new Widget_Label(caption, w - Dialogue_Step::BORDER*2, GLOBALS->bold_font);
		caption_label->set_centre_x(true);
		caption_label->set_padding(Dialogue_Step::BORDER);
		caption_label->set_parent(pad);
	}
	else {
		caption_label = NULL;
	}

	if (custom_list != NULL) {
		list = custom_list;
		list->set_width(1.0f);
		list->set_height(lines_to_show * GLOBALS->bold_font->get_height() + 4); // Widget_List constructor adds + 4
	}
	else {
		list = new Widget_List(1.0f, lines_to_show * GLOBALS->bold_font->get_height());
	}
	list->set_items(choices);
	list->set_float_bottom(true);
	list->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(list);
}

Multiple_Choice_GUI::~Multiple_Choice_GUI()
{
}

void Multiple_Choice_GUI::update()
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		Callback_Data data;
		data.choice = pressed;
		data.userdata = callback_data;
		callback((void *)&data);
		exit();
	}
}

void Multiple_Choice_GUI::handle_event(TGUI_Event *event)
{
	if (escape_choice >= 0 && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2))) {
		M3_GLOBALS->button->play(false);
		Callback_Data data;
		data.choice = escape_choice;
		data.userdata = callback_data;
		callback((void *)&data);
		exit();
	}
	if (escape_choice == -2 && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2))) {
		M3_GLOBALS->button->play(false);
		exit();
	}
	else {
		GUI::handle_event(event);
	}
}

void Multiple_Choice_GUI::resize(util::Size<int> new_size)
{
	assert(0 && "OVERRIDE THIS");
}

void Multiple_Choice_GUI::get_height(int &w, int &h, int &num_lines)
{
	bool full;
	int width;

	// Get caption # lines
	if (caption == "") {
		num_lines = 0;
		h = GLOBALS->bold_font->get_height() * lines_to_show + 4;
	}
	else {
		GLOBALS->bold_font->draw_wrapped(shim::palette[20], caption, util::Point<float>(0, 0), w, GLOBALS->bold_font->get_height(), -1, 0, 0, true, full, num_lines, width);
		if (shrink_to_fit) {
			w = MIN(w, width);
		}
		h = GLOBALS->bold_font->get_height() * (lines_to_show + num_lines) + 4;
	}
}

//--

Battle_Multiple_Choice_GUI::Battle_Multiple_Choice_GUI(std::string caption, std::vector<std::string> choices, int escape_choice, util::Callback callback, void *callback_data) :
	Multiple_Choice_GUI(false, caption, choices, escape_choice, callback, callback_data)
{
	transition = false;
}

Battle_Multiple_Choice_GUI::~Battle_Multiple_Choice_GUI()
{
}

void Battle_Multiple_Choice_GUI::resize(util::Size<int> new_size)
{
	util::Point<int> pad(new_size.w * 0.03f, new_size.h * 0.03f);

	window->set_padding_left(pad.x);
	window->set_padding_top(new_size.h - (pad.y + window->get_height()) - 1);

	int w = (new_size.w - pad.x*2) / 2;
	window->set_width(w);

	list->set_width(w-Dialogue_Step::BORDER*2);

	if (caption_label) {
		caption_label->set_max_width(w-Dialogue_Step::BORDER*2);
	}

	gui->resize(new_size.w, new_size.h);
}

int Battle_Multiple_Choice_GUI::get_selected()
{
	if (list) {
		return list->get_selected();
	}
	else {
		return -1;
	}
}

//--

Positioned_Multiple_Choice_GUI::Positioned_Multiple_Choice_GUI(bool tint_screen, std::string caption, std::vector<std::string> choices, int escape_choice, int horiz_pos/*-1, 0, 1=left, center, right*/, int vert_pos/*same as horiz_pos*/, int padding_left, int padding_right, int padding_top, int padding_bottom, float screen_margin_x, float screen_margin_y, util::Callback callback, void *callback_data, int lines_to_show, int width, bool shrink_to_fit, float width_ratio, Widget_List *custom_list) :
	Multiple_Choice_GUI(tint_screen, caption, choices, escape_choice, callback, callback_data, lines_to_show, width, shrink_to_fit, custom_list),
	horiz_pos(horiz_pos),
	vert_pos(vert_pos),
	padding_left(padding_left),
	padding_right(padding_right),
	padding_top(padding_top),
	padding_bottom(padding_bottom),
	screen_margin_x(screen_margin_x),
	screen_margin_y(screen_margin_y),
	width_ratio(width_ratio),
	width(width)
{
}

Positioned_Multiple_Choice_GUI::~Positioned_Multiple_Choice_GUI()
{
}

void Positioned_Multiple_Choice_GUI::resize(util::Size<int> new_size)
{
	int margin_x = new_size.w * screen_margin_x;
	int margin_y = new_size.h * screen_margin_y;

	int pad_left = padding_left;
	int pad_right = padding_right;
	int pad_top = padding_top;
	int pad_bottom = padding_bottom;

	if (horiz_pos == -1) {
		window->set_float_right(false);
		window->set_centre_x(false);
		pad_left += margin_x;
	}
	else if (horiz_pos == 0) {
		window->set_float_right(false);
		window->set_centre_x(true);
	}
	else {
		window->set_float_right(true);
		window->set_centre_x(false);
		pad_right += margin_x;
	}

	if (vert_pos == -1) {
		window->set_float_bottom(false);
		window->set_centre_y(false);
		pad_top += margin_y;
	}
	else if (vert_pos == 0) {
		window->set_float_bottom(false);
		window->set_centre_y(true);
	}
	else {
		window->set_float_bottom(true);
		window->set_centre_y(false);
		pad_bottom += margin_y;
	}

	window->set_padding_left(pad_left);
	window->set_padding_right(pad_right);
	window->set_padding_top(pad_top);
	window->set_padding_bottom(pad_bottom);

	int w = width;
	int h, num_lines;
	get_height(w, h, num_lines);
	if (shrink_to_fit) {
		w += Dialogue_Step::BORDER*2 + 1;
		std::vector<std::string> choices = list->get_items();
		for (size_t i = 0; i < choices.size(); i++) {
			int choice_w = shim::font->get_text_width(choices[i]) + 1;
			w = MAX(w, choice_w + Dialogue_Step::BORDER * 2);
		}
		w += Dialogue_Step::BORDER*2;
		if (list->visible_rows() < (int)choices.size()) {
			w += 7; // for scrollbar (arrow_size == 5 in widgets.cpp)
		}
	}
	else {
		w = new_size.w * width_ratio;
	}
	window->set_width(w);
	window->set_height(h+Dialogue_Step::BORDER*(caption_label == NULL ? 2 : 4));
	list->set_width(w-Dialogue_Step::BORDER*2);
	list->set_height(lines_to_show * GLOBALS->bold_font->get_height() + 4); // Widget_List constructor adds + 4
	if (caption_label) {
		caption_label->set_max_width(w-Dialogue_Step::BORDER*2);
	}

	gui->resize(new_size.w, new_size.h);
}

//--

Battle_List_GUI::Battle_List_GUI(std::vector< std::pair<int, std::string> > items, int escape_choice, util::Callback callback, void *callback_data) :
	callback(callback),
	callback_data(callback_data),
	exit_menu(false),
	escape_choice(escape_choice)
{
	int w = 100;
	int h = shim::font->get_height() * 2;

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	window = new Widget_Window(w + Dialogue_Step::BORDER * 2, h + Dialogue_Step::BORDER * 2 + 4); // list adds 4
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(Dialogue_Step::BORDER);
	pad->set_parent(window);

	list = new Widget_Quantity_List(1.0f, h);
	list->set_items(items);
	list->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(list);
}

Battle_List_GUI::~Battle_List_GUI()
{
}

void Battle_List_GUI::update()
{
	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		Callback_Data data;
		data.choice = pressed;
		data.cancelled = false;
		data.userdata = callback_data;
		callback((void *)&data);
		exit();
	}
}

void Battle_List_GUI::handle_event(TGUI_Event *event)
{
	if (escape_choice >= 0 && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2))) {
		M3_GLOBALS->button->play(false);
		Callback_Data data;
		data.choice = escape_choice;
		data.cancelled = true;
		data.userdata = callback_data;
		callback((void *)&data);
		exit();
	}
	if (escape_choice == -2 && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2))) {
		M3_GLOBALS->button->play(false);
		exit();
	}
	else {
		GUI::handle_event(event);
	}
}

void Battle_List_GUI::resize(util::Size<int> new_size)
{
	util::Point<int> pad(new_size.w * 0.03f, new_size.h * 0.03f);

	window->set_padding_left(pad.x);
	window->set_padding_top(new_size.h - (pad.y + window->get_height()) - 1);

	int w = (new_size.w - pad.x*2);
	window->set_width(w);

	list->set_width(w-Dialogue_Step::BORDER*2);

	gui->layout();
}

void Battle_List_GUI::set_disabled(int index, bool disabled)
{
	list->set_disabled(index, disabled);
}

void Battle_List_GUI::set_descriptions(std::vector<std::string> descriptions)
{
	list->set_descriptions(descriptions);
}

//--

Battle_Vampire_List_GUI::Battle_Vampire_List_GUI(std::vector< std::pair<std::string, std::string> > items, int escape_choice, util::Callback callback, void *callback_data) :
	callback(callback),
	callback_data(callback_data),
	exit_menu(false),
	escape_choice(escape_choice)
{
	int w = 100;
	int h = shim::font->get_height() * 2;

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	window = new Widget_Window(w + Dialogue_Step::BORDER * 2, h + Dialogue_Step::BORDER * 2 + 4); // list adds 4
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(Dialogue_Step::BORDER);
	pad->set_parent(window);

	list = new Widget_Vampire_List(1.0f, h);
	list->set_items(items);
	list->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(list);
}

Battle_Vampire_List_GUI::~Battle_Vampire_List_GUI()
{
}

void Battle_Vampire_List_GUI::update()
{
	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		Callback_Data data;
		data.choice = pressed;
		data.cancelled = false;
		data.userdata = callback_data;
		callback((void *)&data);
		exit();
	}
}

void Battle_Vampire_List_GUI::handle_event(TGUI_Event *event)
{
	if (escape_choice >= 0 && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2))) {
		M3_GLOBALS->button->play(false);
		Callback_Data data;
		data.choice = escape_choice;
		data.cancelled = true;
		data.userdata = callback_data;
		callback((void *)&data);
		exit();
	}
	if (escape_choice == -2 && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2))) {
		M3_GLOBALS->button->play(false);
		exit();
	}
	else {
		GUI::handle_event(event);
	}
}

void Battle_Vampire_List_GUI::resize(util::Size<int> new_size)
{
	util::Point<int> pad(new_size.w * 0.03f, new_size.h * 0.03f);

	window->set_padding_left(pad.x);
	window->set_padding_top(new_size.h - (pad.y + window->get_height()) - 1);

	int w = (new_size.w - pad.x*2);
	window->set_width(w);

	list->set_width(w-Dialogue_Step::BORDER*2);

	gui->layout();
}

void Battle_Vampire_List_GUI::set_disabled(int index, bool disabled)
{
	list->set_disabled(index, disabled);
}

int Battle_Vampire_List_GUI::get_selected()
{
	return list->get_selected();
}

//--

Get_Number_GUI::Get_Number_GUI(std::string text, int stops, int initial_value, util::Callback callback, void *callback_data) :
	callback(callback),
	callback_data(callback_data)
{
	transition = true;
	transition_is_shrink = true;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	SDL_Colour background_colour = shim::black;
	background_colour.r *= 0.75;
	background_colour.g *= 0.75;
	background_colour.b *= 0.75;
	background_colour.a *= 0.75;
	modal_main_widget->set_background_colour(background_colour);

	Widget_Label *label = new Widget_Label(text, 100, GLOBALS->bold_font);
	label->set_centre_x(true);
	label->set_padding(Dialogue_Step::BORDER);

	slider = new Widget_Slider(100, stops, initial_value);
	slider->set_centre_x(true);
	slider->set_clear_float_x(true);
	slider->set_break_line(true);
	slider->set_padding_left(Dialogue_Step::BORDER);
	slider->set_padding_right(Dialogue_Step::BORDER);

	value_label = new Widget_Label("", 50, GLOBALS->bold_font);
	value_label->set_centre_x(true);
	value_label->set_clear_float_x(true);
	value_label->set_break_line(true);
	value_label->set_padding(Dialogue_Step::BORDER);

	ok_button = new Widget_Text_Button(GLOBALS->game_t->translate(1434));
	ok_button->set_centre_x(true);
	ok_button->set_padding_bottom(Dialogue_Step::BORDER);

	slider->set_down_widget(ok_button);

	cancel_button = new Widget_Text_Button(GLOBALS->game_t->translate(1732));
	cancel_button->set_centre_x(true);
	cancel_button->set_padding_left(2);

	Widget *button_container = new Widget(1.0f, ok_button->get_height());
	button_container->set_float_bottom(true);

	Widget_Window *window = new Widget_Window(100 + Dialogue_Step::BORDER*4, label->get_height() + slider->get_height() + value_label->get_height() + ok_button->get_height() + Dialogue_Step::BORDER*6);
	window->set_centre_x(true);
	window->set_centre_y(true);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(Dialogue_Step::BORDER);
	pad->set_parent(window);
	
	label->set_parent(pad);
	slider->set_parent(pad);
	value_label->set_parent(pad);
	ok_button->set_padding_right(2);
	button_container->set_parent(pad);
	ok_button->set_parent(button_container);
	cancel_button->set_parent(button_container);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(slider);
}

Get_Number_GUI::~Get_Number_GUI()
{
}

void Get_Number_GUI::handle_event(TGUI_Event *event)
{
	if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)) {
		M3_GLOBALS->button->play(false);
		Callback_Data d;
		d.number = -1;
		d.userdata = callback_data;
		callback(&d);
		exit();
	}
	else {
		GUI::handle_event(event);
	}
}

void Get_Number_GUI::update()
{
	value_label->set_text(util::itos(slider->get_value()));

	gui->layout();

	if (ok_button->pressed()) {
		Callback_Data d;
		d.number = slider->get_value();
		d.userdata = callback_data;
		callback(&d);
		exit();
	}
	else if (cancel_button->pressed()) {
		Callback_Data d;
		d.number = -1;
		d.userdata = callback_data;
		callback(&d);
		exit();
	}
}

//--

Button_GUI::Button_GUI(std::string text, int horiz_pos/*-1, 0, 1=left, center, right*/, int vert_pos/*same as horiz_pos*/, int padding_left, int padding_right, int padding_top, int padding_bottom, float screen_margin_x, float screen_margin_y, util::Callback callback, void *callback_data) :
	callback(callback),
	callback_data(callback_data),
	horiz_pos(horiz_pos),
	vert_pos(vert_pos),
	padding_left(padding_left),
	padding_right(padding_right),
	padding_top(padding_top),
	padding_bottom(padding_bottom),
	screen_margin_x(screen_margin_x),
	screen_margin_y(screen_margin_y)
{
	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	button = new Widget_Text_Button(text);
	
	window = new Widget_Window(button->get_width() + Dialogue_Step::BORDER * 2, button->get_height() + Dialogue_Step::BORDER * 2);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(Dialogue_Step::BORDER);
	pad->set_parent(window);

	button->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(button);

	resize(shim::screen_size);
}

Button_GUI::~Button_GUI()
{
}

void Button_GUI::resize(util::Size<int> new_size)
{
	int margin_x = new_size.w * screen_margin_x;
	int margin_y = new_size.h * screen_margin_y;

	int pad_left = padding_left;
	int pad_right = padding_right;
	int pad_top = padding_top;
	int pad_bottom = padding_bottom;

	if (horiz_pos == -1) {
		window->set_float_right(false);
		window->set_centre_x(false);
		pad_left += margin_x;
	}
	else if (horiz_pos == 0) {
		window->set_float_right(false);
		window->set_centre_x(true);
	}
	else {
		window->set_float_right(true);
		window->set_centre_x(false);
		pad_right += margin_x;
	}

	if (vert_pos == -1) {
		window->set_float_bottom(false);
		window->set_centre_y(false);
		pad_top += margin_y;
	}
	else if (vert_pos == 0) {
		window->set_float_bottom(false);
		window->set_centre_y(true);
	}
	else {
		window->set_float_bottom(true);
		window->set_centre_y(false);
		pad_bottom += margin_y;
	}

	window->set_padding_left(pad_left);
	window->set_padding_right(pad_right);
	window->set_padding_top(pad_top);
	window->set_padding_bottom(pad_bottom);

	gui->resize(new_size.w, new_size.h);
}

void Button_GUI::handle_event(TGUI_Event *event)
{
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)) {
		M3_GLOBALS->button->play(false);
		if (callback) {
			Callback_Data d;
			d.cancelled = true;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else {
		gui->handle_event(event);
	}
}

void Button_GUI::update()
{
	if (button->pressed()) {
		if (callback) {
			Callback_Data d;
			d.cancelled = false;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
}

//--

Player_Stats_GUI::Player_Stats_GUI(int character_index, util::Callback callback, void *callback_data) :
	callback(callback),
	callback_data(callback_data)
{
	transition = true;
	transition_is_shrink = true;

	wedge::Player_Stats *stats = &INSTANCE->stats[character_index];

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	
	SDL_Colour background_colour = shim::black;
	background_colour.r *= 0.75;
	background_colour.g *= 0.75;
	background_colour.b *= 0.75;
	background_colour.a *= 0.75;
	modal_main_widget->set_background_colour(background_colour);

	Widget_Image *profile_pic = new Widget_Image(M3_GLOBALS->profile_images[character_index], false);
	Widget_Label *name_label = new Widget_Label(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(INSTANCE->stats[character_index].name)) + TAG_END, -1);
	name_label->set_padding_left(3);
	name_label->set_padding_top(1);
	status_label = new Widget_Label("", -1);
	status_label->set_padding_top(1);
	status_label->set_padding_left(1);

	Widget_Label *l1 = new Widget_Label(GLOBALS->game_t->translate(1338)/* Originally: HP */, -1);
	Widget_Label *l2 = new Widget_Label(GLOBALS->game_t->translate(1337)/* Originally: MP */, -1);
	
	l2->set_break_line(true);

	Widget *labels1 = new Widget();
	labels1->set_break_line(true);
	labels1->set_padding_top(1);

	l1->set_parent(labels1);
	l2->set_parent(labels1);

	Widget *labels2 = new Widget();
	labels2->set_padding_top(1);

	hp_label = new Widget_Label("", -1);
	hp_label->set_padding_left(5);
	hp_label->set_parent(labels2);

	mp_label = new Widget_Label("", -1);
	mp_label->set_break_line(true);
	mp_label->set_padding_left(5);
	mp_label->set_parent(labels2);

	Widget *labels3 = new Widget();
	labels3->set_break_line(true);

	status_label->set_text(get_status_name(stats->base.hp, stats->base.status));
	status_label->set_text_colour(get_status_colour(stats->base.hp, stats->base.status));

	hp_label->set_text(util::itos(stats->base.hp) + "/" + util::itos(stats->base.fixed.max_hp));
	hp_label->set_text_colour(get_status_colour(stats->base.hp - stats->base.fixed.max_hp * 0.15f, wedge::STATUS_OK));
	mp_label->set_text(util::itos(stats->base.mp) + "/" + util::itos(stats->base.fixed.max_mp));

	button = new Widget_Text_Button(GLOBALS->game_t->translate(1434)/* Originally: OK */);
	button->set_centre_x(true);
	button->set_float_bottom(true);

	int w = 0;
	w = MAX(w, name_label->get_width() + profile_pic->get_width() + status_label->get_width() + 4);
	w = MAX(w, l1->get_width() + hp_label->get_width() + 5);
	w = MAX(w, l2->get_width() + mp_label->get_width() + 5);
	w += Dialogue_Step::BORDER;

	int h = profile_pic->get_height() + hp_label->get_height() + mp_label->get_height() + button->get_height() + 1 + Dialogue_Step::BORDER;

	Widget_Window *window = new Widget_Window(w + Dialogue_Step::BORDER * 2, h + Dialogue_Step::BORDER * 2);
	window->set_centre_x(true);
	window->set_centre_y(true);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(Dialogue_Step::BORDER);
	pad->set_parent(window);

	profile_pic->set_parent(pad);
	name_label->set_parent(pad);
	status_label->set_parent(pad);
	labels1->set_parent(pad);
	labels2->set_parent(pad);
	labels3->set_parent(pad);
	button->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Player_Stats_GUI::~Player_Stats_GUI()
{
}

void Player_Stats_GUI::handle_event(TGUI_Event *event)
{
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)) {
		M3_GLOBALS->button->play(false);
		if (callback) {
			Callback_Data d;
			d.cancelled = true;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else {
		gui->handle_event(event);
	}
}

void Player_Stats_GUI::update()
{
	if (button->pressed()) {
		if (callback) {
			Callback_Data d;
			d.cancelled = false;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
}

//--

Notification_GUI::Notification_GUI(std::string text, util::Callback callback, void *callback_data, bool shrink_to_fit) :
	callback(callback),
	callback_data(callback_data)
{
	transition = true;
	transition_is_shrink = true;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	SDL_Colour background_colour = shim::black;
	background_colour.r *= 0.75;
	background_colour.g *= 0.75;
	background_colour.b *= 0.75;
	background_colour.a *= 0.75;
	modal_main_widget->set_background_colour(background_colour);

	int window_w = 130;

	bool full;
	int num_lines, width;
	int line_height = GLOBALS->bold_font->get_height() + 1;
	GLOBALS->bold_font->draw_wrapped(shim::white, text, util::Point<int>(0, 0), window_w - Dialogue_Step::BORDER*4, line_height, -1, -1, 0, true, full, num_lines, width);
	
	ok_button = new Widget_Text_Button(GLOBALS->game_t->translate(1434));

	if (shrink_to_fit) {
		window_w = MIN(window_w, MAX(ok_button->get_width(), width) + Dialogue_Step::BORDER * 4);
	}
	
	label = new Widget_Label(text, window_w - Dialogue_Step::BORDER*4, GLOBALS->bold_font);
	label->set_padding(Dialogue_Step::BORDER);
	label->set_centre_x(true);

	ok_button->set_centre_x(true);
	ok_button->set_float_bottom(true);

	Widget_Window *window = new Widget_Window(window_w, line_height * num_lines + ok_button->get_height() + Dialogue_Step::BORDER*4);
	window->set_centre_x(true);
	window->set_centre_y(true);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(Dialogue_Step::BORDER);
	pad->set_parent(window);
	
	label->set_parent(pad);
	ok_button->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(ok_button);
}

Notification_GUI::~Notification_GUI()
{
}

void Notification_GUI::update()
{
	if (ok_button->pressed()) {
		if (callback) {
			Callback_Data d;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
}

void Notification_GUI::handle_event(TGUI_Event *event)
{
	if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)
	) {
		M3_GLOBALS->button->play(false);
		if (callback) {
			Callback_Data d;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else {
		gui::GUI::handle_event(event);
	}
}

//--

Yes_No_GUI::Yes_No_GUI(std::string text, bool escape_cancels, util::Callback callback, void *callback_data, bool shrink_to_fit) :
	escape_cancels(escape_cancels),
	callback(callback),
	callback_data(callback_data),
	_hook_omnipresent(false),
	hook_draw_last(false),
	count(0),
	drawing_hook(NULL)
{
	transition = true;
	transition_is_shrink = true;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	SDL_Colour background_colour = shim::black;
	background_colour.r *= 0.75;
	background_colour.g *= 0.75;
	background_colour.b *= 0.75;
	background_colour.a *= 0.75;
	modal_main_widget->set_background_colour(background_colour);

	int window_w = int(shim::screen_size.w * 0.75f);

	bool full;
	int num_lines, width;
	int line_height = GLOBALS->bold_font->get_height() + 1;
	GLOBALS->bold_font->draw_wrapped(shim::white, text, util::Point<int>(0, 0), window_w - Dialogue_Step::BORDER*4, line_height, -1, -1, 0, true, full, num_lines, width);
	
	yes_button = new Widget_Text_Button(GLOBALS->game_t->translate(1729));
	no_button = new Widget_Text_Button(GLOBALS->game_t->translate(1730));

	if (shrink_to_fit) {
		window_w = MIN(window_w, MAX(yes_button->get_width() + no_button->get_width() + 2, width) + Dialogue_Step::BORDER * 4);
	}

	Widget_Label *label = new Widget_Label(text, window_w - Dialogue_Step::BORDER*4, GLOBALS->bold_font);
	label->set_centre_x(true);
	label->set_padding(Dialogue_Step::BORDER);

	yes_button->set_centre_x(true);
	yes_button->set_padding_right(2);

	no_button->set_centre_x(true);
	no_button->set_padding_left(2);

	Widget *button_container = new Widget(1.0f, yes_button->get_height());
	button_container->set_float_bottom(true);

	Widget_Window *window = new Widget_Window(window_w, line_height * num_lines + yes_button->get_height() + Dialogue_Step::BORDER*4);
	window->set_centre_x(true);
	window->set_centre_y(true);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(Dialogue_Step::BORDER);
	pad->set_parent(window);

	label->set_parent(pad);
	button_container->set_parent(pad);
	yes_button->set_parent(button_container);
	no_button->set_parent(button_container);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(yes_button);
}

Yes_No_GUI::~Yes_No_GUI()
{
	delete drawing_hook;
}

void Yes_No_GUI::update()
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if (yes_button->pressed()) {
		if (callback) {
			Callback_Data d;
			d.choice = true;
			d.cancelled = false;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else if (no_button->pressed()) {
		if (callback) {
			Callback_Data d;
			d.choice = false;
			d.cancelled = false;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
}

void Yes_No_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if (escape_cancels && (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b2) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b2)
	)) {
		M3_GLOBALS->button->play(false);
		if (callback) {
			Callback_Data d;
			d.choice = false;
			d.cancelled = true;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else {
		GUI::handle_event(event);
	}
}

void Yes_No_GUI::set_selected(bool yes_no)
{
	if (yes_no) {
		gui->set_focus(yes_button);
	}
	else {
		gui->set_focus(no_button);
	}
}

void Yes_No_GUI::hook_omnipresent(bool hook, bool last)
{
	_hook_omnipresent = hook;
	hook_draw_last = last;
}

void Yes_No_GUI::draw()
{
	if (_hook_omnipresent) {
		return;
	}

	gui::GUI::draw();
}

void Yes_No_GUI::draw_fore()
{
	if (_hook_omnipresent == false) {
		gui::GUI::draw_fore();
		return;
	}

	int mod = count % 2;
	count++;
	if (mod == 0) {
		if (drawing_hook == NULL) {
			drawing_hook = new GUI_Drawing_Hook_Step(this, hook_draw_last);
		}
		drawing_hook->hook();
		gui::GUI::draw_fore();
		return;
	}

	gfx::Font::end_batches();

	gui::GUI::draw_back();
	gui::GUI::draw();
	gui::GUI::draw_fore();
}
