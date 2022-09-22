#ifndef MENU_H
#define MENU_H

#include <Nooskewl_Wedge/main.h>

#include "gui.h"
#include "widgets.h"

class Main_Menu_GUI : public Menu_GUI {
public:
	Main_Menu_GUI(int character_index);

	void update_background();
	void handle_event(TGUI_Event *event);
	void draw_fore();

protected:
	bool is_left(Main_Menu_GUI *current, Main_Menu_GUI *next);
	bool is_key(int code, int desired); // checks if 'desired' is assigned by user, if so returns false
	std::string get_caption();
	std::string add_hotkey(std::string s, int code);
	void maybe_show_third_button_help();
	int get_translated_hotkey(int code);

	bool showing_settings;
};

class Progress_GUI : public Main_Menu_GUI {
public:
	Progress_GUI(int character_index); // for loading the next gui
	virtual ~Progress_GUI();

	void handle_event(TGUI_Event *event);
	void update();

private:
	Widget_Text_Button *resume_button;
	Widget_Text_Button *save_button;
	Widget_Text_Button *quit_button;
};

class Map_GUI : public Main_Menu_GUI {
public:
	Map_GUI(int character_index); // for loading the next gui
	virtual ~Map_GUI();

	void handle_event(TGUI_Event *event);
	void lost_device();
	void found_device();
	void draw_fore();

private:
	void draw_map(int w, int h, bool draw_portals);
	Widget_Window *window;
	Widget_Map *map_widget;
	gfx::Image *map;
	gfx::Image *map2;
};

class Stats_GUI : public Main_Menu_GUI {
public:
	Stats_GUI(int character_index);
	virtual ~Stats_GUI();

	void handle_event(TGUI_Event *event);

private:
	void set_text();

	Widget_Label *status_label;
	Widget_Label *hp_label;
	Widget_Label *mp_label;
	Widget_Label *level_label;
	Widget_Label *experience_label;
	Widget_Label *next_label;
	Widget_Label *attack_label;
	Widget_Label *defense_label;
	Widget_Label *luck_label;
	Widget_Label *gold_label;
};

class Items_GUI : public Main_Menu_GUI {
public:
	Items_GUI(int character_index, int top, int selected);
	virtual ~Items_GUI();

	void handle_event(TGUI_Event *event);
	void update();

private:
	void set_text(int top, int selected);

	Widget_Window *window;
	Widget_Image *profile_pic;
	Widget_Label *status_label;
	Widget_Label *hp_label;
	Widget_Label *mp_label;
	Widget_Quantity_List *list;

	std::vector<int> inventory_indices;
};

class Spells_GUI : public Main_Menu_GUI {
public:
	struct Callback_Data {
		std::string spell;
		wedge::Base_Stats *caster_stats;
		Spells_GUI *gui;
	};

	Spells_GUI(int character_index, int top, int selected);
	virtual ~Spells_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	
	void set_text(int top, int selected);

private:
	std::vector<int> spell_indices;
	Widget_Window *window;
	Widget_Image *profile_pic;
	Widget_Label *status_label;
	Widget_Label *hp_label;
	Widget_Label *mp_label;
	Widget_Quantity_List *list;
};

class Weapons_GUI : public Main_Menu_GUI {
public:
	Weapons_GUI(int character_index, int top, int selected);
	virtual ~Weapons_GUI();

	void handle_event(TGUI_Event *event);
	void update();

private:
	void set_text(int top, int selected);

	Widget_Window *window;
	Widget_Image *profile_pic;
	Widget_Label *status_label;
	Widget_Label *weapon_label;
	Widget_Label *attack_label;
	Widget_Quantity_List *list;

	std::vector<int> inventory_indices;
};

class Armour_GUI : public Main_Menu_GUI {
public:
	Armour_GUI(int character_index, int top, int selected);
	virtual ~Armour_GUI();

	void handle_event(TGUI_Event *event);
	void update();

private:
	void set_text(int top, int selected);

	Widget_Window *window;
	Widget_Image *profile_pic;
	Widget_Label *status_label;
	Widget_Label *armour_label;
	Widget_Label *defense_label;
	Widget_Quantity_List *list;

	std::vector<int> inventory_indices;
};

class Vampires_GUI : public Main_Menu_GUI {
public:
	Vampires_GUI(int character_index);
	virtual ~Vampires_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	
	void set_text();

private:
	Widget_Window *window;
	Widget_Label *hp_label;
	Widget_Label *mp_label;
	Widget_Vampire_List *list;
};

class Discard_GUI : public Main_Menu_GUI {
public:
	struct Callback_Data {
		Discard_GUI *gui;
		int item_index;
	};

	Discard_GUI(int character_index);
	virtual ~Discard_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	
	void set_text();

private:
	Callback_Data userdata;

	Widget_Window *window;
	Widget_Quantity_List *list;

	std::vector<int> inventory_indices;
};

#endif // MENU_H
