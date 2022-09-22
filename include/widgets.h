#ifndef WIDGET_LIST
#define WIDGET_LIST

#include <Nooskewl_Wedge/main.h>

class Widget : public TGUI_Widget {
public:
	static void static_start();

	static void set_default_background_colour(SDL_Colour default_colour);

	Widget(int w, int h);
	Widget(float percent_w, float percent_h);
	Widget(int w, float percent_h);
	Widget(float percent_w, int h);
	Widget(TGUI_Widget::Fit fit, int other);
	Widget(TGUI_Widget::Fit fit, float percent_other);
	Widget(); // Fit both
	virtual ~Widget();

	virtual void draw();

	void set_background_colour(SDL_Colour colour);

	bool is_focussed();

protected:
	static SDL_Colour default_background_colour;

	void start();

	SDL_Colour background_colour;
};

class Widget_Window : public Widget
{
public:
	Widget_Window(int w, int h);
	Widget_Window(float percent_w, float percent_h);
	Widget_Window(int w, float percent_h);
	Widget_Window(float percent_w, int h);
	Widget_Window(TGUI_Widget::Fit fit, int other);
	Widget_Window(TGUI_Widget::Fit fit, float percent_other);
	virtual ~Widget_Window();

	void draw();
};

class Widget_Image : public Widget
{
public:
	static void static_start();
	static void set_default_shadow_colour(SDL_Colour default_colour);

	Widget_Image(gfx::Image *image, bool destroy = true);
	virtual ~Widget_Image();

	void draw();

	void set_shadow_colour(SDL_Colour colour);

private:
	static SDL_Colour default_shadow_colour;

	gfx::Image *image;
	bool destroy;
	SDL_Colour shadow_colour;
};

class Widget_Label : public Widget
{
public:
	static void static_start();
	static void set_default_text_colour(SDL_Colour default_colour);
	static void set_default_shadow_colour(SDL_Colour default_colour);

	Widget_Label(std::string text, int max_w, bool bold = false);
	virtual ~Widget_Label();

	void draw();

	void set_text(std::string text);
	void set_max_width(int width);

	void set_text_colour(SDL_Colour colour);
	void set_shadow_colour(SDL_Colour colour);

	void set_shadow_type(gfx::Font::Shadow_Type shadow_type);

	std::string get_text();

private:
	static SDL_Colour default_text_colour;
	static SDL_Colour default_shadow_colour;

	void start();

	std::string text;
	int max_w;
	
	SDL_Colour text_colour;

	SDL_Colour shadow_colour;
	gfx::Font::Shadow_Type shadow_type;
	bool bold;
};

class Widget_Button : public Widget {
public:
	Widget_Button(int w, int h);
	Widget_Button(float w, float h);
	Widget_Button(int w, float h);
	Widget_Button(float w, int h);
	virtual ~Widget_Button();

	virtual void handle_event(TGUI_Event *event);

	virtual bool pressed();

	void set_sound_enabled(bool enabled);

protected:
	bool _pressed;
	bool _released;
	bool _hover;
	bool sound_enabled;
	bool gotten;
};

class Widget_Text_Button : public Widget_Button
{
public:
	Widget_Text_Button(std::string text);
	Widget_Text_Button(gfx::Image *icon, bool destroy);
	virtual ~Widget_Text_Button();

	void draw();

	void set_enabled(bool enabled);
	bool is_enabled();
	void set_text(std::string text);

protected:
	void set_size();

	std::string text;
	int padding;

	bool enabled;
	
	SDL_Colour focussed_button_colour;
	SDL_Colour hover_button_colour;
	SDL_Colour normal_button_top_colour;
	SDL_Colour normal_button_bottom_colour;
	SDL_Colour focussed_inner_border_colour;
	SDL_Colour focussed_outer_border_colour;
	SDL_Colour normal_inner_border_colour;
	SDL_Colour normal_outer_border_colour;
	SDL_Colour disabled_text_colour;
	SDL_Colour focussed_text_colour;
	SDL_Colour normal_text_colour;
	SDL_Colour focussed_text_shadow_colour;
	SDL_Colour normal_text_shadow_colour;

	gfx::Image *icon;
	bool destroy;
};

class Widget_Main_Menu_Text_Button : public Widget_Text_Button
{
public:
	Widget_Main_Menu_Text_Button(std::string text);
	virtual ~Widget_Main_Menu_Text_Button();
};

class Widget_Main_Menu_Icon_Button : public Widget_Text_Button
{
public:
	Widget_Main_Menu_Icon_Button(gfx::Image *icon, bool destroy);
	virtual ~Widget_Main_Menu_Icon_Button();
};

class Widget_Slider : public Widget
{
public:
	static const int TAB_SIZE = 6;

	Widget_Slider(int width, int stops, int initial_value);
	virtual ~Widget_Slider();

	void handle_event(TGUI_Event *event);
	void draw();

	int get_value();

protected:
	int stops;
	int value;
	bool mouse_down;
};

class Widget_List : public Widget
{
public:
	static const int LONGPRESS_TIME = 500;

	static void static_start();
	static void set_default_focussed_bar_colour(SDL_Colour default_colour);
	static void set_default_bar_colour(SDL_Colour default_colour);
	static void set_default_normal_text_colour(SDL_Colour default_colour);
	static void set_default_selected_text_colour(SDL_Colour default_colour);
	static void set_default_highlight_text_colour(SDL_Colour default_colour);
	static void set_default_disabled_text_colour(SDL_Colour default_colour);
	static void set_default_text_shadow_colour(SDL_Colour default_colour);
	static void set_default_selected_text_shadow_colour(SDL_Colour default_colour);
	static void set_default_unfocussed_selected_text_shadow_colour(SDL_Colour default_colour);
	static void set_default_highlight_text_shadow_colour(SDL_Colour default_colour);
	static void set_default_disabled_text_shadow_colour(SDL_Colour default_colour);

	Widget_List(int w, int h);
	Widget_List(float w, float h);
	Widget_List(int w, float h);
	Widget_List(float w, int h);
	virtual ~Widget_List();

	virtual void draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y); // subclass this to customize

	void handle_event(TGUI_Event *event);
	void draw();

	void set_items(std::vector<std::string> new_items); // you can overload this to set extra values
	int pressed();
	int get_selected();
	void set_selected(int selected);
	int get_size();
	int get_top();
	void set_top(int top);
	std::vector<std::string> get_items();
	std::string get_item(int index);

	void set_highlight(int index, bool onoff);
	bool is_highlighted(int index);

	void set_disabled(int index, bool onoff);
	bool is_disabled(int index);

	void set_focussed_bar_colour(SDL_Colour colour);
	void set_bar_colour(SDL_Colour colour);
	void set_normal_text_colour(SDL_Colour colour);
	void set_selected_text_colour(SDL_Colour colour);
	void set_highlight_text_colour(SDL_Colour colour);
	void set_disabled_text_colour(SDL_Colour colour);
	void set_text_shadow_colour(SDL_Colour colour);
	void set_selected_text_shadow_colour(SDL_Colour colour);
	void set_unfocussed_selected_text_shadow_colour(SDL_Colour colour);
	void set_highlight_text_shadow_colour(SDL_Colour colour);
	void set_disabled_text_shadow_colour(SDL_Colour colour);

	void set_arrow_colour(SDL_Colour colour);
	void set_arrow_shadow_colour(SDL_Colour colour);

	void set_reserved_size(int pixels); // for auto-sizing lists
	virtual void resize();
	
	int visible_rows();

protected:
	static SDL_Colour default_focussed_bar_colour;
	static SDL_Colour default_bar_colour;
	static SDL_Colour default_normal_text_colour;
	static SDL_Colour default_selected_text_colour;
	static SDL_Colour default_highlight_text_colour;
	static SDL_Colour default_disabled_text_colour;
	static SDL_Colour default_text_shadow_colour;
	static SDL_Colour default_selected_text_shadow_colour;
	static SDL_Colour default_unfocussed_selected_text_shadow_colour;
	static SDL_Colour default_highlight_text_shadow_colour;
	static SDL_Colour default_disabled_text_shadow_colour;

	void start();
	void up();
	void down();
	int get_click_row(int y);
	void change_top(int rows);
	int used_height();
	void draw_scrollbar();
	int scrollbar_height();
	int scrollbar_pos();

	std::vector<std::string> items; // this generic class expects this to be filled, or at least filled with the number of empty strings in your list
	int top;
	int selected;
	int row_h;

	int pressed_item;

	bool mouse_down;
	bool scrollbar_down;
	bool clicked;
	util::Point<int> mouse_down_point;
	int mouse_down_row;
	int scrollbar_pos_mouse_down;

	std::vector<int> highlight;
	std::vector<int> disabled;
	
	SDL_Colour focussed_bar_colour;
	SDL_Colour bar_colour;
	SDL_Colour normal_text_colour;
	SDL_Colour selected_text_colour;
	SDL_Colour highlight_text_colour;
	SDL_Colour disabled_text_colour;
	SDL_Colour text_shadow_colour;
	SDL_Colour selected_text_shadow_colour;
	SDL_Colour unfocussed_selected_text_shadow_colour;
	SDL_Colour highlight_text_shadow_colour;
	SDL_Colour disabled_text_shadow_colour;

	util::Size<int> arrow_size;

	SDL_Colour arrow_colour;
	SDL_Colour arrow_shadow_colour;

	bool has_scrollbar;

	Uint32 down_time;
	int down_selected;

	int reserved_size;

	Uint32 selected_time;
};

class Widget_Quantity_List : public Widget_List
{
public:
	static const int LONGPRESS_TIME = 1000;

	Widget_Quantity_List(int w, int h);
	Widget_Quantity_List(float w, float h);
	Widget_Quantity_List(int w, float h);
	Widget_Quantity_List(float w, int h);
	virtual ~Widget_Quantity_List();

	void handle_event(TGUI_Event *event);

	void draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y);

	void set_items(std::vector< std::pair<int, std::string> > new_items); // int is quantity
	void set_descriptions(std::vector<std::string> descriptions);

protected:
	void show_description();

	std::vector<int> quantities;
	int max_quantity_width;
	std::vector<std::string> descriptions;
	Uint32 longpress_started;
	util::Point<int> longpress_pos;
};

class Widget_Vampire_List : public Widget_List
{
public:
	Widget_Vampire_List(int w, int h);
	Widget_Vampire_List(float w, float h);
	Widget_Vampire_List(int w, float h);
	Widget_Vampire_List(float w, int h);
	virtual ~Widget_Vampire_List();

	void draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y);

	void set_items(std::vector< std::pair<std::string, std::string> > new_items); // first std::string is "hp/mp"

protected:
	std::vector<std::string> costs;
	int max_cost_width;
};

class Widget_Save_Game_List : public Widget_List
{
public:
	Widget_Save_Game_List(int w, int h);
	Widget_Save_Game_List(float w, float h);
	Widget_Save_Game_List(int w, float h);
	Widget_Save_Game_List(float w, int h);
	virtual ~Widget_Save_Game_List();

	void draw();
	void draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y);

	void set_items(std::vector<std::string> times, std::vector< std::pair<int, int> > experience); // ints are each player's experience

protected:
	std::vector< std::pair<int, int> > experience;
};

class Widget_Controls_List : public Widget_List
{
public:
	Widget_Controls_List(int w, int h);
	Widget_Controls_List(float w, float h);
	Widget_Controls_List(int w, float h);
	Widget_Controls_List(float w, int h);
	virtual ~Widget_Controls_List();

	void draw_row(int index, SDL_Colour colour, SDL_Colour shadow_Colour, int y);

	void set_items(std::vector<std::string> names, std::vector<std::string> assignments);

protected:
	std::vector<std::string> assignments;
};

class Widget_Spell_Target_List : public Widget_List
{
public:
	Widget_Spell_Target_List(int w, int h);
	Widget_Spell_Target_List(float w, float h);
	Widget_Spell_Target_List(int w, float h);
	Widget_Spell_Target_List(float w, int h);
	virtual ~Widget_Spell_Target_List();

	void draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y);

protected:
	std::map<std::string, gfx::Image *> mini_profile_pics;
};

class Widget_Checkbox : public Widget
{
public:
	Widget_Checkbox(std::string text, bool checked);
	virtual ~Widget_Checkbox();

	void handle_event(TGUI_Event *event);
	void draw();

	bool is_checked();

	void set_disabled(bool disabled);

protected:
	std::string text;
	bool checked;
	bool mouse_down;
};

class Widget_Map : public Widget
{
public:
	Widget_Map();
	virtual ~Widget_Map();

	void draw();

	void set_images(gfx::Image *map, gfx::Image *map2); // with/without portals for blinking

	virtual void handle_event(TGUI_Event *event);

private:
	gfx::Image *image;
	gfx::Image *image2;
};

#endif // WIDGET_LIST
