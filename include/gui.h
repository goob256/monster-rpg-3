#ifndef GUI_H
#define GUI_H

#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/stats.h>

class GUI_Drawing_Hook_Step;
class Widget_Label;
class Widget_List;
class Widget_Quantity_List;
class Widget_Vampire_List;
class Widget_Slider;
class Widget_Text_Button;
class Widget_Window;

class Title_GUI : public gui::GUI {
public:
	static const int START_LOGO_SCALE = 10;
	static const int SCALE_IN_DURATION = 2000;
	static const int TICKS_BEFORE_SCALE_DOWN = 60; // logic rate

	Title_GUI();
	virtual ~Title_GUI();

	virtual void draw();
	virtual void update();
	virtual void update_background();
	virtual void handle_event(TGUI_Event *event);
	virtual bool is_fullscreen();
	virtual void resize(util::Size<int> size);

	void go(int slot, bool exists, bool is_auto);

	void set_stop_drawing(bool stop);

	void reload_text();
	
private:
	void load_game(util::JSON *json);

	TGUI_Widget *container;
	Widget_Text_Button *new_game_button;
	Widget_Text_Button *load_button;
	Widget_Text_Button *settings_button;
#if !defined ANDROID && !defined IOS
	Widget_Text_Button *exit_button;
#endif
	gfx::Image *logo;
	float start_scale;
	int update_count;
	bool retrying_boss;
#if defined IOS || defined ANDROID
	bool in_settings;
#endif
	bool stop_drawing;
};

class Menu_GUI : public gui::GUI {
public:
	static const int TRANSITION_TIME = 250;

	Menu_GUI(int character_index);
	virtual ~Menu_GUI();

	virtual void draw();
	virtual void draw_fore();
	virtual void update();
	virtual void handle_event(TGUI_Event *event);
	void resize(util::Size<int> size);

	void set_next_character_gui(Menu_GUI *next_character_gui, bool next_character_gui_is_left);
	void set_next_gui(Menu_GUI *next_gui, bool next_gui_is_left);

protected:
	util::Point<float> get_offset();
	std::string get_char_swap_text();

	int character_index;
	wedge::Player_Stats *stats;
	Menu_GUI *next_character_gui;
	bool next_character_gui_is_left;
	Uint32 character_transition_start;
	Menu_GUI *next_gui;
	bool next_gui_is_left;
	Uint32 gui_transition_start;
	bool has_left_gui;
	bool has_right_gui;

	bool done;
};

// NOTE: *_Multiple_Choice_GUIs need to be resized, then layed out right after creating them!!!

class Multiple_Choice_GUI : public gui::GUI {
public:
	struct Callback_Data {
		int choice;
		void *userdata;
	};

	Multiple_Choice_GUI(bool tint_screen, std::string caption, std::vector<std::string> choices, int escape_choice, util::Callback callback, void *callback_data, int lines_to_show = 2, int width = 100, bool shrink_to_fit = true, Widget_List *custom_list = NULL);
	virtual ~Multiple_Choice_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	void get_height(int &w, int &h, int &num_lines);

	virtual void resize(util::Size<int> new_size); // must override this

protected:
	Widget_Window *window;
	Widget_Label *caption_label;
	Widget_List *list;

	std::string caption;

	util::Callback callback;
	void *callback_data;

	bool exit_menu;

	int escape_choice; // escape activates this choice. If it's -1, escape does nothing. If it's -2, escape dismisses the dialog with no action.

	int lines_to_show;
	bool shrink_to_fit;
};

class Battle_Multiple_Choice_GUI : public Multiple_Choice_GUI {
public:
	Battle_Multiple_Choice_GUI(std::string caption, std::vector<std::string> choices, int escape_choice, util::Callback callback, void *callback_data);
	virtual ~Battle_Multiple_Choice_GUI();

	void resize(util::Size<int> new_size);

	int get_selected();

private:
};

class Positioned_Multiple_Choice_GUI : public Multiple_Choice_GUI {
public:
	Positioned_Multiple_Choice_GUI(bool tint_screen, std::string caption, std::vector<std::string> choices, int escape_choice, int horiz_pos/*-1, 0, 1=left, center, right*/, int vert_pos/*same as horiz_pos*/, int padding_left, int padding_right, int padding_top, int padding_bottom, float screen_margin_x, float screen_margin_y, util::Callback callback, void *callback_data, int lines_to_show = 2, int width = 100, bool shrink_to_fit = true, float width_ratio = 0.571f/*4/7ths*/, Widget_List *custom_list = NULL);
	virtual ~Positioned_Multiple_Choice_GUI();

	void resize(util::Size<int> new_size);

private:
	int horiz_pos;
	int vert_pos;
	int padding_left;
	int padding_right;
	int padding_top;
	int padding_bottom;
	float screen_margin_x;
	float screen_margin_y;
	float width_ratio;
	int width;
};

class Battle_List_GUI : public gui::GUI {
public:
	struct Callback_Data {
		int choice;
		bool cancelled;
		void *userdata;
	};

	Battle_List_GUI(std::vector< std::pair<int, std::string> > items, int escape_choice, util::Callback callback, void *callback_data);
	virtual ~Battle_List_GUI();

	void handle_event(TGUI_Event *event);
	void update();

	void resize(util::Size<int> new_size);

	void set_disabled(int index, bool disabled);

	void set_descriptions(std::vector<std::string> descriptions);

protected:
	Widget_Window *window;
	Widget_Quantity_List *list;

	util::Callback callback;
	void *callback_data;

	bool exit_menu;

	int escape_choice; // escape activates this choice. If it's -1, escape does nothing. If it's -2, escape dismisses the dialog with no action.
};

class Battle_Vampire_List_GUI : public gui::GUI {
public:
	struct Callback_Data {
		int choice;
		bool cancelled;
		void *userdata;
	};

	Battle_Vampire_List_GUI(std::vector< std::pair<std::string, std::string> > items, int escape_choice, util::Callback callback, void *callback_data);
	virtual ~Battle_Vampire_List_GUI();

	void handle_event(TGUI_Event *event);
	void update();

	void resize(util::Size<int> new_size);

	void set_disabled(int index, bool disabled);

	int get_selected();

protected:
	Widget_Window *window;
	Widget_Vampire_List *list;

	util::Callback callback;
	void *callback_data;

	bool exit_menu;

	int escape_choice; // escape activates this choice. If it's -1, escape does nothing. If it's -2, escape dismisses the dialog with no action.
};

class Get_Number_GUI : public gui::GUI
{
public:
	struct Callback_Data {
		int number;
		void *userdata;
	};

	Get_Number_GUI(std::string text, int stops, int initial_value, util::Callback callback, void *callback_data);
	virtual ~Get_Number_GUI();

	void handle_event(TGUI_Event *event);
	void update();

private:
	Widget_Slider *slider;
	Widget_Label *value_label;
	Widget_Text_Button *ok_button;
	Widget_Text_Button *cancel_button;

	util::Callback callback;
	void *callback_data;
};

class Button_GUI : public gui::GUI {
public:
	struct Callback_Data {
		bool cancelled;
		void *userdata;
	};

	Button_GUI(std::string text, int horiz_pos/*-1, 0, 1=left, center, right*/, int vert_pos/*same as horiz_pos*/, int padding_left, int padding_right, int padding_top, int padding_bottom, float screen_margin_x, float screen_margin_y, util::Callback callback, void *callback_data);
	virtual ~Button_GUI();

	void handle_event(TGUI_Event *event);
	void update();

	void resize(util::Size<int> new_size);

protected:
	Widget_Window *window;
	Widget_Text_Button *button;

	util::Callback callback;
	void *callback_data;
	
	int horiz_pos;
	int vert_pos;
	int padding_left;
	int padding_right;
	int padding_top;
	int padding_bottom;
	float screen_margin_x;
	float screen_margin_y;
};

class Player_Stats_GUI : public gui::GUI {
public:
	struct Callback_Data {
		bool cancelled;
		void *userdata;
	};

	Player_Stats_GUI(int character_index, util::Callback callback, void *callback_data);
	virtual ~Player_Stats_GUI();

	void update();
	void handle_event(TGUI_Event *event);

private:
	Widget_Label *status_label;
	Widget_Label *hp_label;
	Widget_Label *mp_label;
	Widget_Text_Button *button;

	util::Callback callback;
	void *callback_data;
};

class Notification_GUI : public gui::GUI
{
public:
	struct Callback_Data {
		void *userdata;
	};

	Notification_GUI(std::string text, util::Callback callback = 0, void *callback_data = 0, bool shrink_to_fit = true);
	virtual ~Notification_GUI();

	void update();
	void handle_event(TGUI_Event *event);

private:
	Widget_Label *label;
	Widget_Text_Button *ok_button;

	util::Callback callback;
	void *callback_data;
};

class Yes_No_GUI : public wedge::Yes_No_GUI
{
public:
	struct Callback_Data {
		bool choice;
		bool cancelled;
		void *userdata;
	};

	Yes_No_GUI(std::string text, bool escape_cancels, util::Callback callback = 0, void *callback_data = 0, bool shrink_to_fit = true);
	virtual ~Yes_No_GUI();

	void update();
	void handle_event(TGUI_Event *event);
	void draw();
	void draw_fore();

	void set_selected(bool yes_no);
	void hook_omnipresent(bool hook, bool last = false);

private:
	Widget_Text_Button *yes_button;
	Widget_Text_Button *no_button;

	bool escape_cancels;

	util::Callback callback;
	void *callback_data;

	bool _hook_omnipresent;
	bool hook_draw_last;
	int count;
	GUI_Drawing_Hook_Step *drawing_hook;
};

#endif // GUI_H
