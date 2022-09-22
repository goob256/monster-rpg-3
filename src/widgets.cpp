#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>

#include "general.h"
#include "globals.h"
#include "gui.h"
#include "widgets.h"

SDL_Colour Widget::default_background_colour;

void Widget::static_start()
{
	default_background_colour.r = 0;
	default_background_colour.g = 0;
	default_background_colour.b = 0;
	default_background_colour.a = 0;

	Widget_Image::static_start();
	Widget_Label::static_start();
	Widget_List::static_start();
}

void Widget::set_default_background_colour(SDL_Colour colour)
{
	default_background_colour = colour;
}

void Widget::set_background_colour(SDL_Colour colour)
{
	background_colour = colour;
}

void Widget::start()
{
	background_colour = default_background_colour;
}

Widget::Widget(int w, int h) :
	TGUI_Widget(w, h)
{
	start();
}

Widget::Widget(float percent_w, float percent_h) :
	TGUI_Widget(percent_w, percent_h)
{
	start();
}

Widget::Widget(int w, float percent_h) :
	TGUI_Widget(w, percent_h)
{
	start();
}

Widget::Widget(float percent_w, int h) :
	TGUI_Widget(percent_w, h)
{
	start();
}

Widget::Widget(TGUI_Widget::Fit fit, int other) :
	TGUI_Widget(fit, other)
{
	start();
}

Widget::Widget(TGUI_Widget::Fit fit, float percent_other) :
	TGUI_Widget(fit, percent_other)
{
	start();
}

Widget::Widget() :
	TGUI_Widget()
{
	start();
}

Widget::~Widget()
{
}

void Widget::draw()
{
	// This is used to clear the background to a darker colour, so don't do it unless this widget is part of
	// the topmost gui because it could happen twice giving a darker colour

	if (shim::guis.size() > 0) {
		TGUI_Widget *root = shim::guis.back()->gui->get_main_widget();
		TGUI_Widget *w = this;
		TGUI_Widget *parent;
		while ((parent = w->get_parent()) != NULL) {
			w = parent;
		}
		if (root != w) {
			return;
		}
	}

	if (background_colour.a != 0) {
		// Need to clear transforms temporarily because it might be part of a transition
		glm::mat4 old_mv, old_p;
		gfx::get_matrices(old_mv, old_p);
		gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
		gfx::update_projection();
		gfx::draw_filled_rectangle(background_colour, util::Point<int>(calculated_x, calculated_y), util::Size<int>(calculated_w, calculated_h));
		gfx::set_matrices(old_mv, old_p);
		gfx::update_projection();
	}
}

bool Widget::is_focussed()
{
	if (shim::guis.size() == 0) {
		return false;
	}
	if (gui != shim::guis.back()->gui) {
		return false;
	}
	return gui->get_focus() == this;
}

//--

Widget_Window::Widget_Window(int w, int h) :
	Widget(w, h)
{
}

Widget_Window::Widget_Window(float percent_w, float percent_h) :
	Widget(percent_w, percent_h)
{
}

Widget_Window::Widget_Window(int w, float percent_h) :
	Widget(w, percent_h)
{
}

Widget_Window::Widget_Window(float percent_w, int h) :
	Widget(percent_w, h)
{
}

Widget_Window::Widget_Window(TGUI_Widget::Fit fit, int other) :
	Widget(fit, other)
{
}

Widget_Window::Widget_Window(TGUI_Widget::Fit fit, float percent_other) :
	Widget(fit, percent_other)
{
}

Widget_Window::~Widget_Window()
{
}

void Widget_Window::draw()
{
	float little = get_little_bander_offset();

	SDL_Colour *colours = start_bander(num_bands(calculated_h-4), shim::palette[24], shim::palette[22]);
	gfx::draw_filled_rectangle(colours, util::Point<float>(calculated_x+2-little, calculated_y+2-little), util::Size<float>(calculated_w-4+little*2.0f, calculated_h-4+little*2.0f));
	end_bander();
	gfx::draw_primitives_start();
	gfx::draw_rectangle(shim::palette[24], util::Point<int>(calculated_x, calculated_y)+util::Point<int>(1, 1), util::Size<int>(calculated_w, calculated_h)-util::Size<int>(2, 2));
	gfx::draw_rectangle(shim::palette[27], util::Point<int>(calculated_x, calculated_y), util::Size<int>(calculated_w, calculated_h));
	gfx::draw_primitives_end();
}

//--

SDL_Colour Widget_Image::default_shadow_colour;

void Widget_Image::static_start()
{
	default_shadow_colour.r = 0;
	default_shadow_colour.g = 0;
	default_shadow_colour.b = 0;
	default_shadow_colour.a = 0;
}

void Widget_Image::set_default_shadow_colour(SDL_Colour default_colour)
{
	default_shadow_colour = default_colour;
}

Widget_Image::Widget_Image(gfx::Image *image, bool destroy) :
	Widget(image->size.w, image->size.h),
	image(image),
	destroy(destroy)
{
	shadow_colour = default_shadow_colour;
}

Widget_Image::~Widget_Image()
{
	if (destroy) {
		delete image;
	}
}

void Widget_Image::draw()
{
	if (shadow_colour.a != 0) {
		draw_shadowed_image(shadow_colour, image, util::Point<int>(calculated_x, calculated_y), gfx::Font::DROP_SHADOW);
	}
	else {
		image->draw(util::Point<int>(calculated_x, calculated_y));
	}
}

void Widget_Image::set_shadow_colour(SDL_Colour colour)
{
	shadow_colour = colour;
}

//--

SDL_Colour Widget_Label::default_text_colour;
SDL_Colour Widget_Label::default_shadow_colour;

void Widget_Label::static_start()
{
	default_text_colour.r = 0;
	default_text_colour.g = 0;
	default_text_colour.b = 0;
	default_text_colour.a = 255;
	default_shadow_colour.r = 0;
	default_shadow_colour.g = 0;
	default_shadow_colour.b = 0;
	default_shadow_colour.a = 0;
}

void Widget_Label::set_default_text_colour(SDL_Colour colour)
{
	default_text_colour = colour;
}

void Widget_Label::set_default_shadow_colour(SDL_Colour colour)
{
	default_shadow_colour = colour;
}

void Widget_Label::set_text_colour(SDL_Colour colour)
{
	text_colour = colour;
}

void Widget_Label::set_shadow_colour(SDL_Colour colour)
{
	shadow_colour = colour;
}

void Widget_Label::set_shadow_type(gfx::Font::Shadow_Type shadow_type)
{
	this->shadow_type = shadow_type;
}

void Widget_Label::start()
{
	text_colour = default_text_colour;
	shadow_colour = default_shadow_colour;
	shadow_type = gfx::Font::DROP_SHADOW;
}

Widget_Label::Widget_Label(std::string text, int max_w, bool bold) :
	Widget(0, 0),
	bold(bold)
{
	start();

	if (max_w < 0) {
		this->max_w = INT_MAX;
	}
	else {
		this->max_w = max_w;
	}

	set_text(text);
}

Widget_Label::~Widget_Label()
{
}

void Widget_Label::draw()
{
	bool full;
	int num_lines, width;

	gfx::Font *font = bold ? GLOBALS->bold_font : shim::font;

	if (shadow_type != gfx::Font::NO_SHADOW) {
		font->enable_shadow(shadow_colour, shadow_type);
	}

	font->draw_wrapped(text_colour, text, util::Point<int>(calculated_x, calculated_y), max_w, font->get_height()+1, -1, -1, 0, false, full, num_lines, width);

	if (shadow_type != gfx::Font::NO_SHADOW) {
		font->disable_shadow();
	}
}

void Widget_Label::set_text(std::string text)
{
	gfx::Font *font = bold ? GLOBALS->bold_font : shim::font;
	this->text = text;
	bool full;
	int num_lines, width;
	int line_height = font->get_height()-1;
	font->draw_wrapped(text_colour, text, util::Point<int>(calculated_x, calculated_y), max_w, line_height, -1, -1, 0, true, full, num_lines, width);
	w = width;
	h = line_height * num_lines;
}

void Widget_Label::set_max_width(int width)
{
	max_w = width;
	set_text(text); // readjust w
}

std::string Widget_Label::get_text()
{
	return text;
}

//--

Widget_Button::Widget_Button(int w, int h) :
	Widget(w, h),
	_pressed(false),
	_released(false),
	_hover(false),
	sound_enabled(true),
	gotten(true)
{
	accepts_focus = true;
}

Widget_Button::Widget_Button(float w, float h) :
	Widget(w, h),
	_pressed(false),
	_released(false),
	_hover(false),
	sound_enabled(true),
	gotten(true)
{
	accepts_focus = true;
}

Widget_Button::Widget_Button(int w, float h) :
	Widget(w, h),
	_pressed(false),
	_released(false),
	_hover(false),
	sound_enabled(true),
	gotten(true)
{
	accepts_focus = true;
}

Widget_Button::Widget_Button(float w, int h) :
	Widget(w, h),
	_pressed(false),
	_released(false),
	_hover(false),
	sound_enabled(true),
	gotten(true)
{
	accepts_focus = true;
}

Widget_Button::~Widget_Button()
{
}

void Widget_Button::handle_event(TGUI_Event *event)
{
	int x, y;

	if (use_relative_position) {
		x = relative_x;
		y = relative_y;
	}
	else {
		x = calculated_x;
		y = calculated_y;
	}

	if (event->type == TGUI_MOUSE_AXIS) {
		if (event->mouse.x >= x && event->mouse.x < x+calculated_w && event->mouse.y >= y && event->mouse.y < y+calculated_h) {
			_hover = true;
		}
		else {
			_hover = false;
		}
	}
	if (accepts_focus && gui->get_event_owner(event) == this) {
		if (event->type == TGUI_KEY_DOWN && event->keyboard.is_repeat == false) {
			if (event->keyboard.code == GLOBALS->key_b1) {
				if (gotten) {
					_pressed = true;
					_hover = true;
				}
			}
			else {
				_pressed = false;
				_hover = false;
			}
		}
		else if (event->type == TGUI_JOY_DOWN && event->joystick.is_repeat == false) {
			if (event->joystick.button == GLOBALS->joy_b1) {
				if (gotten) {
					_pressed = true;
					_hover = true;
				}
			}
			else {
				_pressed = false;
				_hover = false;
			}
		}
		else if (event->type == TGUI_MOUSE_DOWN && event->mouse.is_repeat == false) {
			if (event->mouse.button == 1) {
				if (gotten) {
					_pressed = true;
				}
			}
			else {
				_pressed = false;
			}
			_hover = true;
		}
		else if (event->type == TGUI_KEY_UP && event->keyboard.is_repeat == false) {
			if (_pressed && event->keyboard.code == GLOBALS->key_b1) {
				if (gotten) {
					gotten = false;
					_released = true;
					_hover = false;
					if (sound_enabled) {
						if (M3_GLOBALS->button != 0) {
							M3_GLOBALS->button->play(false);
						}
					}
				}
			}
			else {
				_pressed = false;
				_hover = false;
			}
		}
		else if (event->type == TGUI_JOY_UP && event->joystick.is_repeat == false) {
			if (_pressed && (event->joystick.button == GLOBALS->joy_b1)) {
				if (gotten) {
					gotten = false;
					_released = true;
					_hover = false;
					if (sound_enabled) {
						if (M3_GLOBALS->button != 0) {
							M3_GLOBALS->button->play(false);
						}
					}
				}
			}
			else {
				_pressed = false;
				_hover = false;
			}
		}
		else if (event->type == TGUI_MOUSE_UP && event->mouse.is_repeat == false) {
			if (_pressed && (event->mouse.button == 1)) {
				if (gotten) {
					gotten = false;
					_released = true;
					if (sound_enabled) {
						if (M3_GLOBALS->button != 0) {
							M3_GLOBALS->button->play(false);
						}
					}
				}
			}
			else {
				_pressed = false;
			}
		}
	}
	else {
		if (event->type == TGUI_KEY_UP) {
			_pressed = false;
			_hover = false;
		}
		else if (event->type == TGUI_JOY_UP) {
			_pressed = false;
			_hover = false;
		}
		else if (event->type == TGUI_MOUSE_UP && event->mouse.is_repeat == false) {
			_pressed = false;
			_hover = false;
		}
		else if ((event->type == TGUI_MOUSE_DOWN || event->type == TGUI_MOUSE_AXIS) && event->mouse.is_repeat == false) {
			_hover = false;
		}
	}
}

bool Widget_Button::pressed()
{
	bool r = _released;
	if (_released) {
		_pressed = _released = false;
	}
	gotten = true;
	return r;
}

void Widget_Button::set_sound_enabled(bool enabled)
{
	sound_enabled = enabled;
}

//--

Widget_Text_Button::Widget_Text_Button(std::string text) :
	Widget_Button(0, 0),
	text(text),
	icon(NULL),
	destroy(false)
{
	enabled = true;
	padding = 3;
	set_size();

	focussed_button_colour = shim::palette[14];
	hover_button_colour = shim::palette[23];
	normal_button_top_colour = shim::palette[24];
	normal_button_bottom_colour = shim::palette[22];
	focussed_inner_border_colour = shim::palette[15];
	focussed_outer_border_colour = shim::palette[16];
	normal_inner_border_colour = shim::palette[24];
	normal_outer_border_colour = shim::palette[27];
	disabled_text_colour = shim::black;
	focussed_text_colour = shim::white;
	normal_text_colour = shim::white;
	focussed_text_shadow_colour = shim::palette[15];
	normal_text_shadow_colour = shim::palette[24];
}

Widget_Text_Button::Widget_Text_Button(gfx::Image *icon, bool destroy) :
	Widget_Button(0, 0),
	icon(icon),
	destroy(destroy)
{
	enabled = true;
	padding = 3;
	set_size();

	focussed_button_colour = shim::palette[14];
	hover_button_colour = shim::palette[23];
	normal_button_top_colour = shim::palette[24];
	normal_button_bottom_colour = shim::palette[22];
	focussed_inner_border_colour = shim::palette[15];
	focussed_outer_border_colour = shim::palette[16];
	normal_inner_border_colour = shim::palette[24];
	normal_outer_border_colour = shim::palette[27];
	disabled_text_colour = shim::black;
	focussed_text_colour = shim::white;
	normal_text_colour = shim::white;
	focussed_text_shadow_colour = shim::palette[15];
	normal_text_shadow_colour = shim::palette[24];
}

Widget_Text_Button::~Widget_Text_Button()
{
	if (destroy) {
		delete icon;
	}
}

void Widget_Text_Button::draw()
{
	bool focussed = is_focussed();
	util::Point<int> offset(0, 0);

	//bool pressed = false;
	bool hover = false;

	if (focussed) {
		if (_pressed && _hover) {
			offset = util::Point<int>(1, 1);
			//pressed = true;
		}
		else if (_hover) {
			hover = true;
		}
	}
	else {
		if (_hover) {
			hover = true;
		}
	}

	int x, y;

	if (use_relative_position) {
		x = relative_x;
		y = relative_y;
	}
	else {
		x = calculated_x;
		y = calculated_y;
	}

	if (focussed) {
		start_pulse_brighten(0.25f, true, false);
		gfx::draw_filled_rectangle(focussed_button_colour, util::Point<int>(x+offset.x, y+offset.y), util::Size<int>(calculated_w, calculated_h));
		end_pulse_brighten();
	}
	else if (hover) {
		start_pulse_brighten(0.25f, true, false);
		gfx::draw_filled_rectangle(hover_button_colour, util::Point<int>(x+offset.x, y+offset.y), util::Size<int>(calculated_w, calculated_h));
		end_pulse_brighten();
	}
	else {
		SDL_Colour *colours = start_bander(num_bands(calculated_h-4), normal_button_top_colour, normal_button_bottom_colour);
		float little = get_little_bander_offset();
		gfx::draw_filled_rectangle(colours, util::Point<float>(x+offset.x+2-little, y+offset.y+2-little), util::Size<float>(calculated_w-4+little*2.0f, calculated_h-4+little*2.0f));
		end_bander();
	}
	SDL_Colour inner_border;
	SDL_Colour outer_border;
	if (focussed) {
		inner_border = focussed_inner_border_colour;
		outer_border = focussed_outer_border_colour;
	}
	else {
		inner_border = normal_inner_border_colour;
		outer_border = normal_outer_border_colour;
	}
	if (focussed || hover) {
		start_pulse_brighten(0.25f, true, false);
	}
	gfx::draw_rectangle(inner_border, util::Point<int>(x+offset.x, y+offset.y)+util::Point<int>(1, 1), util::Size<int>(calculated_w, calculated_h)-util::Size<int>(2, 2));
	gfx::draw_rectangle(outer_border, util::Point<int>(x+offset.x, y+offset.y), util::Size<int>(calculated_w, calculated_h));
	if (focussed || hover) {
		end_pulse_brighten();
	}

	SDL_Colour colour;
	if (enabled == false) {
		colour = disabled_text_colour;
	}
	else {
		if (focussed) {
			colour = focussed_text_colour;
		}
		else {
			colour = normal_text_colour;
		}
	}

	if (icon) {
		icon->draw_tinted(colour, util::Point<float>(x+padding+offset.x, y+padding+offset.y+(shim::font->get_height()-icon->size.h)/2));
	}
	else {
		if (focussed) {
			shim::font->enable_shadow(focussed_text_shadow_colour, gfx::Font::DROP_SHADOW);
		}
		else {
			shim::font->enable_shadow(normal_text_shadow_colour, gfx::Font::DROP_SHADOW);
		}
		shim::font->draw(colour, text, util::Point<float>(x+offset.x+padding, (float)y+padding+offset.y));
		shim::font->disable_shadow();
	}
}

void Widget_Text_Button::set_size()
{
	if (icon != NULL) {
		w = icon->size.w + padding * 2;
	}
	else {
		w = shim::font->get_text_width(text) + padding * 2 + 1; // +1 for shadow
	}
	h = shim::font->get_height() + padding * 2;
}

void Widget_Text_Button::set_enabled(bool enabled)
{
	this->enabled = enabled;
	if (enabled == true) {
		accepts_focus = true;
	}
	else {
		accepts_focus = false;
	}
}

bool Widget_Text_Button::is_enabled()
{
	return enabled;
}

void Widget_Text_Button::set_text(std::string text)
{
	this->text = text;
	set_size();
}

//--

Widget_Main_Menu_Text_Button::Widget_Main_Menu_Text_Button(std::string text) :
	Widget_Text_Button(text)
{
	w--; // No shadow on these buttons, that's an extra pixel

	focussed_button_colour = shim::palette[25];
	/*
	focussed_button_colour.r *= 0.5f;
	focussed_button_colour.g *= 0.5f;
	focussed_button_colour.b *= 0.5f;
	focussed_button_colour.a *= 0.5f;
	*/
	hover_button_colour = shim::black;
	/*
	hover_button_colour.r *= 0.5f;
	hover_button_colour.g *= 0.5f;
	hover_button_colour.b *= 0.5f;
	hover_button_colour.a *= 0.5f;
	*/
	normal_button_top_colour = shim::black;
	/*
	normal_button_top_colour.r *= 0.5f;
	normal_button_top_colour.g *= 0.5f;
	normal_button_top_colour.b *= 0.5f;
	normal_button_top_colour.a *= 0.5f;
	*/
	normal_button_bottom_colour = shim::black;
	/*
	normal_button_bottom_colour.r *= 0.5f;
	normal_button_bottom_colour.g *= 0.5f;
	normal_button_bottom_colour.b *= 0.5f;
	normal_button_bottom_colour.a *= 0.5f;
	*/
	focussed_outer_border_colour = shim::black;
	focussed_inner_border_colour = shim::palette[20];
	normal_outer_border_colour = shim::black;
	normal_inner_border_colour = shim::palette[9];
	disabled_text_colour = shim::black;
	focussed_text_colour = shim::palette[20];
	normal_text_colour = shim::palette[9];
	focussed_text_shadow_colour = shim::transparent;
	normal_text_shadow_colour = shim::transparent;
}

Widget_Main_Menu_Text_Button::~Widget_Main_Menu_Text_Button()
{
}

//--

Widget_Main_Menu_Icon_Button::Widget_Main_Menu_Icon_Button(gfx::Image *icon, bool destroy) :
	Widget_Text_Button(icon, destroy)
{
	focussed_button_colour = shim::palette[25];
	/*
	focussed_button_colour.r *= 0.5f;
	focussed_button_colour.g *= 0.5f;
	focussed_button_colour.b *= 0.5f;
	focussed_button_colour.a *= 0.5f;
	*/
	hover_button_colour = shim::black;
	/*
	hover_button_colour.r *= 0.5f;
	hover_button_colour.g *= 0.5f;
	hover_button_colour.b *= 0.5f;
	hover_button_colour.a *= 0.5f;
	*/
	normal_button_top_colour = shim::black;
	/*
	normal_button_top_colour.r *= 0.5f;
	normal_button_top_colour.g *= 0.5f;
	normal_button_top_colour.b *= 0.5f;
	normal_button_top_colour.a *= 0.5f;
	*/
	normal_button_bottom_colour = shim::black;
	/*
	normal_button_bottom_colour.r *= 0.5f;
	normal_button_bottom_colour.g *= 0.5f;
	normal_button_bottom_colour.b *= 0.5f;
	normal_button_bottom_colour.a *= 0.5f;
	*/
	focussed_outer_border_colour = shim::black;
	focussed_inner_border_colour = shim::palette[20];
	normal_outer_border_colour = shim::black;
	normal_inner_border_colour = shim::palette[9];
	disabled_text_colour = shim::black;
	focussed_text_colour = shim::palette[20];
	normal_text_colour = shim::palette[9];
	focussed_text_shadow_colour = shim::transparent;
	normal_text_shadow_colour = shim::transparent;
}

Widget_Main_Menu_Icon_Button::~Widget_Main_Menu_Icon_Button()
{
}

//--

Widget_Slider::Widget_Slider(int width, int stops, int initial_value) :
	Widget(width, 0),
	stops(stops),
	value(initial_value),
	mouse_down(false)
{
	h = TAB_SIZE;

	accepts_focus = true;
}

Widget_Slider::~Widget_Slider()
{
}

void Widget_Slider::handle_event(TGUI_Event *event)
{
	util::Size<int> tab_size(h, h);

	bool focussed = is_focussed();

	if (focussed && event->type == TGUI_FOCUS) {
		if (event->focus.type == TGUI_FOCUS_LEFT) {
			if (value > 0) {
				if (shim::widget_mml != 0) {
					shim::widget_mml->play(false);
				}
				value--;
			}
		}
		else if (event->focus.type == TGUI_FOCUS_RIGHT) {
			if (value < stops-1) {
				if (shim::widget_mml != 0) {
					shim::widget_mml->play(false);
				}
				value++;
			}
		}
	}
	else if (focussed && ((event->type == TGUI_MOUSE_DOWN && event->mouse.is_repeat == false) || (mouse_down && event->type == TGUI_MOUSE_AXIS))) {
		int old_value = value;
		if (mouse_down) {
			float p = (event->mouse.x - calculated_x) / calculated_w;
			p = MAX(0.0f, MIN(0.999f, p));
			p *= stops;
			value = (int)p;
		}
		else {
			TGUI_Event e = tgui_get_relative_event(this, event);
			if (e.mouse.x >= 0) {
				mouse_down = true;
				float p = (event->mouse.x - calculated_x) / calculated_w;
				p = MAX(0.0f, MIN(0.999f, p));
				p *= stops;
				value = (int)p;
			}
		}
		if (value != old_value) {
			if (shim::widget_mml != 0) {
				shim::widget_mml->play(false);
			}
		}
	}
	else if (event->type == TGUI_MOUSE_UP && event->mouse.is_repeat == false) {
		mouse_down = false;
	}
}

void Widget_Slider::draw()
{
	bool focussed = is_focussed();

	if (focussed) {
		start_pulse_brighten(0.25f, true, false);
	}

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(calculated_x, calculated_y), util::Size<int>(calculated_w, TAB_SIZE));

	if (focussed) {
		gfx::draw_rectangle(shim::palette[14], util::Point<int>(calculated_x-1, calculated_y-1), util::Size<int>(calculated_w+2, TAB_SIZE+2));
		end_pulse_brighten();
	}

	int x = int((float)value / (stops-1) * (calculated_w-2));

	if (x != 0) {
		float little = get_little_bander_offset();

		SDL_Colour *colours = start_bander(3, shim::palette[13], shim::palette[16]);
		gfx::draw_filled_rectangle(colours, util::Point<float>(calculated_x + 1-little, calculated_y + 1-little), util::Size<float>(x+little*2.0f, TAB_SIZE-2+little*2.0f));
		end_bander();
	}
}

int Widget_Slider::get_value()
{
	return value;
}

//--

SDL_Colour Widget_List::default_focussed_bar_colour;
SDL_Colour Widget_List::default_bar_colour;
SDL_Colour Widget_List::default_normal_text_colour;
SDL_Colour Widget_List::default_selected_text_colour;
SDL_Colour Widget_List::default_highlight_text_colour;
SDL_Colour Widget_List::default_disabled_text_colour;
SDL_Colour Widget_List::default_text_shadow_colour;
SDL_Colour Widget_List::default_selected_text_shadow_colour;
SDL_Colour Widget_List::default_unfocussed_selected_text_shadow_colour;
SDL_Colour Widget_List::default_highlight_text_shadow_colour;
SDL_Colour Widget_List::default_disabled_text_shadow_colour;

void Widget_List::static_start()
{
	default_focussed_bar_colour.r = 0;
	default_focussed_bar_colour.g = 255;
	default_focussed_bar_colour.b = 255;
	default_focussed_bar_colour.a = 255;
	default_bar_colour.r = 0;
	default_bar_colour.g = 0;
	default_bar_colour.b = 255;
	default_bar_colour.a = 255;
	default_normal_text_colour.r = 192;
	default_normal_text_colour.g = 192;
	default_normal_text_colour.b = 192;
	default_normal_text_colour.a = 255;
	default_selected_text_colour.r = 255;
	default_selected_text_colour.g = 255;
	default_selected_text_colour.b = 255;
	default_selected_text_colour.a = 255;
	default_highlight_text_colour.r = 255;
	default_highlight_text_colour.g = 255;
	default_highlight_text_colour.b = 0;
	default_highlight_text_colour.a = 255;
	default_disabled_text_colour.r = 255;
	default_disabled_text_colour.g = 0;
	default_disabled_text_colour.b = 0;
	default_disabled_text_colour.a = 255;
	default_text_shadow_colour.r = 0;
	default_text_shadow_colour.g = 0;
	default_text_shadow_colour.b = 0;
	default_text_shadow_colour.a = 0;
	default_selected_text_shadow_colour.r = 0;
	default_selected_text_shadow_colour.g = 0;
	default_selected_text_shadow_colour.b = 0;
	default_selected_text_shadow_colour.a = 0;
	default_unfocussed_selected_text_shadow_colour.r = 0;
	default_unfocussed_selected_text_shadow_colour.g = 0;
	default_unfocussed_selected_text_shadow_colour.b = 0;
	default_unfocussed_selected_text_shadow_colour.a = 0;
	default_highlight_text_shadow_colour.r = 0;
	default_highlight_text_shadow_colour.g = 0;
	default_highlight_text_shadow_colour.b = 0;
	default_highlight_text_shadow_colour.a = 0;
	default_disabled_text_shadow_colour.r = 0;
	default_disabled_text_shadow_colour.g = 0;
	default_disabled_text_shadow_colour.b = 0;
	default_disabled_text_shadow_colour.a = 0;
}

void Widget_List::set_default_focussed_bar_colour(SDL_Colour colour)
{
	default_focussed_bar_colour = colour;
}

void Widget_List::set_default_bar_colour(SDL_Colour colour)
{
	default_bar_colour = colour;
}

void Widget_List::set_default_normal_text_colour(SDL_Colour colour)
{
	default_normal_text_colour = colour;
}

void Widget_List::set_default_selected_text_colour(SDL_Colour colour)
{
	default_selected_text_colour = colour;
}

void Widget_List::set_default_highlight_text_colour(SDL_Colour colour)
{
	default_highlight_text_colour = colour;
}

void Widget_List::set_default_disabled_text_colour(SDL_Colour colour)
{
	default_disabled_text_colour = colour;
}

void Widget_List::set_default_text_shadow_colour(SDL_Colour colour)
{
	default_text_shadow_colour = colour;
}

void Widget_List::set_default_selected_text_shadow_colour(SDL_Colour colour)
{
	default_selected_text_shadow_colour = colour;
}

void Widget_List::set_default_unfocussed_selected_text_shadow_colour(SDL_Colour colour)
{
	default_unfocussed_selected_text_shadow_colour = colour;
}

void Widget_List::set_default_highlight_text_shadow_colour(SDL_Colour colour)
{
	default_highlight_text_shadow_colour = colour;
}

void Widget_List::set_default_disabled_text_shadow_colour(SDL_Colour colour)
{
	default_disabled_text_shadow_colour = colour;
}

void Widget_List::set_focussed_bar_colour(SDL_Colour colour)
{
	focussed_bar_colour = colour;
}

void Widget_List::set_bar_colour(SDL_Colour colour)
{
	bar_colour = colour;
}

void Widget_List::set_normal_text_colour(SDL_Colour colour)
{
	normal_text_colour = colour;
}

void Widget_List::set_selected_text_colour(SDL_Colour colour)
{
	selected_text_colour = colour;
}

void Widget_List::set_highlight_text_colour(SDL_Colour colour)
{
	highlight_text_colour = colour;
}

void Widget_List::set_disabled_text_colour(SDL_Colour colour)
{
	disabled_text_colour = colour;
}

void Widget_List::set_text_shadow_colour(SDL_Colour colour)
{
	text_shadow_colour = colour;
}

void Widget_List::set_selected_text_shadow_colour(SDL_Colour colour)
{
	selected_text_shadow_colour = colour;
}

void Widget_List::set_unfocussed_selected_text_shadow_colour(SDL_Colour colour)
{
	unfocussed_selected_text_shadow_colour = colour;
}

void Widget_List::set_highlight_text_shadow_colour(SDL_Colour colour)
{
	highlight_text_shadow_colour = colour;
}

void Widget_List::set_disabled_text_shadow_colour(SDL_Colour colour)
{
	disabled_text_shadow_colour = colour;
}

void Widget_List::start()
{
	h += 4; // more space for green bar

	accepts_focus = true;
	top = 0;
	selected = -1;
	row_h = shim::font->get_height();
	pressed_item = -1;
	mouse_down = false;
	scrollbar_down = false;
	arrow_size = util::Size<int>(5, 3);
	has_scrollbar = true;

	focussed_bar_colour = default_focussed_bar_colour;
	bar_colour = default_bar_colour;
	normal_text_colour = default_normal_text_colour;
	selected_text_colour = default_selected_text_colour;
	highlight_text_colour = default_highlight_text_colour;
	disabled_text_colour = default_disabled_text_colour;
	text_shadow_colour = default_text_shadow_colour;
	selected_text_shadow_colour = default_selected_text_shadow_colour;
	unfocussed_selected_text_shadow_colour = default_unfocussed_selected_text_shadow_colour;
	highlight_text_shadow_colour = default_highlight_text_shadow_colour;
	disabled_text_shadow_colour = default_disabled_text_shadow_colour;

	arrow_colour = shim::white;
	arrow_shadow_colour = shim::palette[24];
	
	down_selected = -1;

	reserved_size = -1;

	selected_time = GET_TICKS();
}

Widget_List::Widget_List(int w, int h) :
	Widget(w, h)
{
	start();
}

Widget_List::Widget_List(float w, float h) :
	Widget(w, h)
{
	start();
}

Widget_List::Widget_List(int w, float h) :
	Widget(w, h)
{
	start();
}

Widget_List::Widget_List(float w, int h) :
	Widget(w, h)
{
	start();
}

Widget_List::~Widget_List()
{
}

void Widget_List::handle_event(TGUI_Event *event)
{
	bool focussed = is_focussed();

	int scrollbar_x = 0, scrollbar_x2 = 0;

	if (has_scrollbar && visible_rows() < (int)items.size()) {
		scrollbar_x = calculated_x + calculated_w - arrow_size.w - 2;
		scrollbar_x2 = calculated_x + calculated_w;
	}
	else {
		scrollbar_x = scrollbar_x2 = calculated_x + calculated_w;
	}

	int scrollbar_y = calculated_y+scrollbar_pos()+M3_GLOBALS->up_arrow->size.h+1;

	if (focussed && event->type == TGUI_FOCUS) {
		if (event->focus.type == TGUI_FOCUS_UP) {
			up();
		}
		else if (event->focus.type == TGUI_FOCUS_DOWN) {
			down();
		}
	}
	else if (focussed && event->type == TGUI_KEY_DOWN && event->keyboard.is_repeat == false) {
		if (event->keyboard.code == GLOBALS->key_b1) {
			down_selected = selected;
			down_time = GET_TICKS();
		}
	}
	else if (focussed && event->type == TGUI_JOY_DOWN && event->joystick.is_repeat == false) {
		if (event->joystick.button == GLOBALS->joy_b1) {
			down_selected = selected;
			down_time = GET_TICKS();
		}
	}
	else if (focussed && event->type == TGUI_KEY_UP && is_disabled(selected) == false && event->keyboard.is_repeat == false) {
		if (event->keyboard.code == GLOBALS->key_b1) {
			if (selected == down_selected) {
				pressed_item = selected;
				if (M3_GLOBALS->button != 0) {
					M3_GLOBALS->button->play(false);
				}
			}
			down_selected = -1;
		}
	}
	else if (focussed && event->type == TGUI_JOY_UP && is_disabled(selected) == false && event->joystick.is_repeat == false) {
		if (event->joystick.button == GLOBALS->joy_b1) {
			if (selected == down_selected) {
				pressed_item = selected;
				if (M3_GLOBALS->button != 0) {
					M3_GLOBALS->button->play(false);
				}
			}
			down_selected = -1;
		}
	}
	else if (event->type == TGUI_MOUSE_DOWN) {
		int mx = (int)event->mouse.x;
		int my = (int)event->mouse.y;
		// Check for clicks on arrows  first
		bool top_arrow = top > 0;
		bool bottom_arrow;
		int vr = visible_rows();
		if ((int)items.size() > vr && top < (int)items.size() - vr) {
			bottom_arrow = true;
		}
		else {
			bottom_arrow = false;
		}
		int height = used_height();
		if (scrollbar_down == false && has_scrollbar && visible_rows() < (int)items.size() && top_arrow && mx >= scrollbar_x && mx < scrollbar_x2 && my >= calculated_y && my <= calculated_y+arrow_size.h) {
			change_top(-1);
		}
		else if (scrollbar_down == false && has_scrollbar && visible_rows() < (int)items.size() && bottom_arrow && mx >= scrollbar_x && mx < scrollbar_x2 && my >= calculated_y+height-arrow_size.h && my <= calculated_y+height) {
			change_top(1);
		}
		else if (event->mouse.is_repeat == false && has_scrollbar && visible_rows() < (int)items.size() && (top_arrow || bottom_arrow) && mx >= scrollbar_x && mx < scrollbar_x2 && my >= scrollbar_y && my < scrollbar_y+scrollbar_height()) {
			scrollbar_down = true;
			mouse_down_point = util::Point<int>(mx, my);
			scrollbar_pos_mouse_down = calculated_y+scrollbar_pos()+arrow_size.h+1;
		}
		else if (scrollbar_down == false && has_scrollbar && visible_rows() < (int)items.size() && (top_arrow || bottom_arrow) && mx >= scrollbar_x && mx < scrollbar_x2 && ((my >= calculated_y+M3_GLOBALS->up_arrow->size.h && my < calculated_y+calculated_h-M3_GLOBALS->down_arrow->size.h) || (my >= scrollbar_y+scrollbar_height() && my < calculated_y+calculated_h-M3_GLOBALS->down_arrow->size.h))) {
			if (my < calculated_y+scrollbar_pos()+arrow_size.h+1) {
				change_top(-visible_rows());
			}
			else if (my >= calculated_y+scrollbar_pos()+scrollbar_height()+arrow_size.h+1) {
				change_top(visible_rows());
			}
		}
		else if (event->mouse.is_repeat == false && mx >= calculated_x && mx < scrollbar_x && my >= calculated_y && my < calculated_y+calculated_h) {
			int r = get_click_row(my);
			if (r >= top && r < (int)items.size() && r < top+visible_rows()) {
				selected = get_click_row(my);
				selected_time = GET_TICKS();
				mouse_down = true;
				clicked = true;
				mouse_down_point = util::Point<int>(mx, my);
				mouse_down_row = selected;
			}
		}
	}
	else if (event->type == TGUI_MOUSE_UP && event->mouse.is_repeat == false) {
		if (mouse_down && clicked && event->mouse.x >= calculated_x && event->mouse.x < calculated_x+calculated_w && event->mouse.y >= calculated_y && event->mouse.y < calculated_y+calculated_h) {
			int row = get_click_row((int)event->mouse.y);
			if (row == mouse_down_row && is_disabled(selected) == false) {
				pressed_item = selected;
				if (M3_GLOBALS->button != 0) {
					M3_GLOBALS->button->play(false);
				}
			}
		}
		mouse_down = false;
		scrollbar_down = false;
	}
	else if (event->type == TGUI_MOUSE_AXIS) {
		int mx = (int)event->mouse.x;
		int my = (int)event->mouse.y;
		if (mouse_down) {
			util::Point<int> p(mx, my);
			util::Point<int> d = p - mouse_down_point;
			if (abs(d.y) >= row_h) {
				clicked = false;
			}
			if (clicked == false) {
				int rows = -d.y / row_h;
				if (rows != 0) {
					change_top(rows);
					mouse_down_point.y -= rows * row_h;
				}
			}
		}
		else if (scrollbar_down) {
			int diffy = scrollbar_pos_mouse_down - mouse_down_point.y;
			float _my = my + diffy;
			int total_rows = (int)items.size();
			int displayed_rows = visible_rows();
			int extra_rows = total_rows - displayed_rows;
			float half = 0.5f / extra_rows;
			_my -= half;
			int extra_h = used_height() - (arrow_size.h * 2 + 2) - scrollbar_height();
			int off_y = _my - (calculated_y + arrow_size.h + 1);
			float p = off_y / (float)extra_h;
			int old_top = top;
			top = p * extra_rows;
			if (top < 0) {
				top = 0;
			}
			if (top > extra_rows) {
				top = extra_rows;
			}
			if (selected < top) {
				selected = top;
				selected_time = GET_TICKS();
			}
			if (selected > top+displayed_rows-1) {
				selected = top+displayed_rows-1;
				selected_time = GET_TICKS();
			}
			if (old_top != top) {
				shim::widget_mml->play(false);
			}
		}
	}
	else if (event->type == TGUI_MOUSE_WHEEL) {
		util::Point<int> mouse_pos = wedge::get_mouse_position();
		if (mouse_pos.x >= calculated_x && mouse_pos.x < calculated_x+calculated_w && mouse_pos.y >= calculated_y && mouse_pos.y < calculated_y+used_height()) {
			change_top(-event->mouse.y);
		}
	}
}

void Widget_List::draw()
{
	int arrow_w;
	
	if (has_scrollbar && visible_rows() < (int)items.size()) {
		arrow_w = M3_GLOBALS->up_arrow->size.w;
	}
	else {
		arrow_w = -2;// counter the -2
	}
	
	bool focussed = is_focussed();

	for (int i = top; i < (int)items.size() && i < top+visible_rows(); i++) {
		int y = calculated_y + ((i - top) * row_h) + 2;
		if (i == selected) {
			SDL_Colour colour;
			if (focussed) {
				colour = focussed_bar_colour;
				start_pulse_brighten(0.25f, true, false);
			}
			else {
				colour = bar_colour;
			}
			gfx::draw_filled_rectangle(colour, util::Point<int>(calculated_x, y-2), util::Size<int>(calculated_w-arrow_w-2, row_h+4));
			gfx::draw_rectangle(shim::palette[16], util::Point<int>(calculated_x, y-2), util::Size<int>(calculated_w-arrow_w-2, row_h+4));
			gfx::draw_rectangle(shim::palette[15], util::Point<int>(calculated_x, y-2)+util::Point<int>(1, 1), util::Size<int>(calculated_w-arrow_w-2, row_h+4)-util::Size<int>(2, 2));
			if (focussed) {
				end_pulse_brighten();
			}
		}
		
		SDL_Colour colour;
		SDL_Colour shadow_colour;
		if (is_disabled(i)) {
			colour = disabled_text_colour;
			shadow_colour = disabled_text_shadow_colour;
		}
		else if (is_highlighted(i)) {
			colour = highlight_text_colour;
			shadow_colour = highlight_text_shadow_colour;
		}
		else if (i == selected) {
			colour = selected_text_colour;
			if (focussed) {
				shadow_colour = selected_text_shadow_colour;
			}
			else {
				shadow_colour = unfocussed_selected_text_shadow_colour;
			}
		}
		else {
			colour = normal_text_colour;
			shadow_colour = text_shadow_colour;
		}

		bool use_shadow;
		if (shadow_colour.r != 0 || shadow_colour.g != 0 || shadow_colour.b != 0 || shadow_colour.a != 0) {
			use_shadow = true;
		}
		else {
			use_shadow = false;
		}

		if (use_shadow) {
			shim::font->enable_shadow(shadow_colour, gfx::Font::DROP_SHADOW);
		}

		draw_row(i, colour, shadow_colour, y);

		if (use_shadow) {
			shim::font->disable_shadow();
		}
	}

	if (has_scrollbar && visible_rows() < (int)items.size()) {
		draw_scrollbar();
	}
}

void Widget_List::draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y)
{
	int xx = calculated_x+3;
	int have = calculated_w - (xx-calculated_x) - 5 - (visible_rows() < (int)items.size() ? arrow_size.w : 0);
	int totlen = shim::font->get_text_width(items[index]);
	int scroll = totlen-have + 2;
	if (totlen > have) {
		const int wait = 1000;
		Uint32 ticks = GET_TICKS() - selected_time;
		int m = scroll * 200;
		Uint32 t = ticks % (wait*2 + m);
		float o;
		if (t < wait || index != selected) {
			o = 0.0f;
		}
		else if ((int)t >= wait+m) {
			o = -scroll;
		}
		else {
			o = -((t - wait) / (float)m * scroll);
		}
		gfx::Font::end_batches();
		// These lists can be transformed, scissor doesn't understand transforms though so we have to calculate
		// However scissor already accounts for black bars, so reverse that
		glm::mat4 mv, _p;
		gfx::get_matrices(mv, _p);
		glm::vec3 zero(0.0f, 0.0f, 0.0f);
		zero = glm::project(zero, mv, _p, glm::vec4(0.0f, 0.0f, (float)shim::real_screen_size.w, (float)shim::real_screen_size.h));
		// don't just use shim::screen_size above as it has to account for black bars
		zero.x -= shim::screen_offset.x;
		zero.y -= shim::screen_offset.y;
		zero.x /= shim::scale;
		zero.y /= shim::scale;
		zero.y = shim::screen_size.h - zero.y;
		gfx::set_scissor(int(zero.x+xx), 0, have, shim::screen_size.h-1);
		shim::font->draw(colour, items[index], util::Point<float>(xx+o, y));
		gfx::Font::end_batches();
		gfx::unset_scissor();
	}
	else {
		shim::font->draw(colour, items[index], util::Point<int>(xx, y));
	}
}

void Widget_List::draw_scrollbar()
{
	if (visible_rows() >= (int)items.size()) {
		return;
	}
	
	int arrow_w = M3_GLOBALS->up_arrow->size.w;
	int height = used_height();

	if (arrow_shadow_colour.a != 0) {
		draw_tinted_shadowed_image(arrow_colour, arrow_shadow_colour, M3_GLOBALS->up_arrow, util::Point<int>(calculated_x+calculated_w-arrow_w-1, calculated_y), gfx::Font::DROP_SHADOW);
		draw_tinted_shadowed_image(arrow_colour, arrow_shadow_colour, M3_GLOBALS->down_arrow, util::Point<int>(calculated_x+calculated_w-arrow_w-1, calculated_y+height-M3_GLOBALS->down_arrow->size.h), gfx::Font::DROP_SHADOW);
		gfx::draw_filled_rectangle(arrow_shadow_colour, util::Point<int>(calculated_x+calculated_w-arrow_w, calculated_y+scrollbar_pos()+M3_GLOBALS->up_arrow->size.h+1), util::Size<int>(M3_GLOBALS->up_arrow->size.w-1, scrollbar_height()+1));
	}
	else {
		M3_GLOBALS->up_arrow->draw_tinted(arrow_colour, util::Point<int>(calculated_x+calculated_w-arrow_w-1, calculated_y));
		M3_GLOBALS->down_arrow->draw_tinted(arrow_colour, util::Point<int>(calculated_x+calculated_w-arrow_w-1, calculated_y+height-M3_GLOBALS->down_arrow->size.h));
	}

	gfx::draw_filled_rectangle(arrow_colour, util::Point<int>(calculated_x+calculated_w-arrow_w, calculated_y+scrollbar_pos()+M3_GLOBALS->up_arrow->size.h+1), util::Size<int>(M3_GLOBALS->up_arrow->size.w-2, scrollbar_height()));
}

int Widget_List::scrollbar_height()
{
	int total_rows = (int)items.size();
	int displayed_rows = visible_rows();
	int extra_rows = total_rows - displayed_rows;
	float p = 1.0f - ((float)extra_rows / total_rows);
	int arrow_h = M3_GLOBALS->up_arrow->size.h + M3_GLOBALS->down_arrow->size.h + 2;
	return (used_height() - arrow_h) * p;
}

int Widget_List::scrollbar_pos()
{
	int arrow_h = M3_GLOBALS->up_arrow->size.h + M3_GLOBALS->down_arrow->size.h + 2;
	int extra_h = used_height() - arrow_h - scrollbar_height();
	int total_rows = (int)items.size();
	int displayed_rows = visible_rows();
	int extra_rows = total_rows - displayed_rows;
	float p = (float)top / extra_rows;
	return p * extra_h;
}

void Widget_List::set_items(std::vector<std::string> new_items)
{
	items.clear();
	items.insert(items.begin(), new_items.begin(), new_items.end());
	accepts_focus = items.size() != 0;

	if (items.size() == 0) {
		selected = -1;
		if (gui != 0 && gui->get_focus() == this) {
			gui->focus_something();
		}
	}
	else {
		if (selected < 0) {
			selected = 0;
			selected_time = GET_TICKS();
		}
	}
}

int Widget_List::pressed()
{
	int ret = pressed_item;
	pressed_item = -1;
	return ret;
}

int Widget_List::get_selected()
{
	return selected;
}

void Widget_List::set_selected(int selected)
{
	this->selected = selected;
	selected_time = GET_TICKS();
}

int Widget_List::get_size()
{
	return (int)items.size();
}

int Widget_List::get_top()
{
	return top;
}

void Widget_List::set_top(int top)
{
	this->top = top;
}

void Widget_List::set_highlight(int index, bool onoff)
{
	std::vector<int>::iterator it = std::find(highlight.begin(), highlight.end(), index);
	if (it != highlight.end()) {
		highlight.erase(it);
	}
	if (onoff) {
		highlight.push_back(index);
	}
}

bool Widget_List::is_highlighted(int index)
{
	return std::find(highlight.begin(), highlight.end(), index) != highlight.end();
}

void Widget_List::up()
{
	if (selected > 0) {
		if (shim::widget_mml != 0) {
			shim::widget_mml->play(false);
		}
		selected--;
		if (selected < top) {
			top--;
		}
		selected_time = GET_TICKS();
	}
}

void Widget_List::down()
{
	if (selected < (int)items.size()-1) {
		if (shim::widget_mml != 0) {
			shim::widget_mml->play(false);
		}
		selected++;
		if (top + visible_rows() <= selected) {
			top++;
		}
		selected_time = GET_TICKS();
	}
}

int Widget_List::get_click_row(int y)
{
	int row = (y - calculated_y) / row_h + top;
	if (row < 0) {
		row = 0;
	}
	else if (row >= (int)items.size()) {
		row = (int)items.size()-1;
	}
	return row;
}

void Widget_List::change_top(int rows)
{
	int vr = visible_rows();
	int old = top;
	top += rows;
	if (top < 0) {
		top = 0;
	}
	else if ((int)items.size() <= vr) {
		top = 0;
	}
	else if (top > (int)items.size() - vr) {
		top = (int)items.size() - vr;
	}
	if (selected < top) {
		selected = top;
		selected_time = GET_TICKS();
	}
	else if (selected >= top + vr) {
		selected = MIN((int)items.size()-1, top + vr - 1);
		selected_time = GET_TICKS();
	}
	if (top != old) {
		if (shim::widget_mml != 0) {
			shim::widget_mml->play(false);
		}
	}
}

int Widget_List::visible_rows()
{
	return (calculated_h-4) / row_h;
}

int Widget_List::used_height()
{
	return calculated_h - 1;
}

void Widget_List::set_disabled(int index, bool onoff)
{
	std::vector<int>::iterator it = std::find(disabled.begin(), disabled.end(), index);
	if (it != disabled.end()) {
		disabled.erase(it);
	}
	if (onoff) {
		disabled.push_back(index);
	}
}

bool Widget_List::is_disabled(int index)
{
	return std::find(disabled.begin(), disabled.end(), index) != disabled.end();
}

void Widget_List::set_arrow_colour(SDL_Colour colour)
{
	arrow_colour = colour;
}

void Widget_List::set_arrow_shadow_colour(SDL_Colour colour)
{
	arrow_shadow_colour = colour;
}

std::vector<std::string> Widget_List::get_items()
{
	return items;
}

std::string Widget_List::get_item(int index)
{
	return items[index];
}

void Widget_List::set_reserved_size(int pixels)
{
	reserved_size = pixels;
	resize();
}

void Widget_List::resize()
{
	if (reserved_size >= 0) {
		calculated_h = (((shim::screen_size.h - reserved_size) - 4) / row_h) * row_h + 4;
	}
	TGUI_Widget::resize();
	if (items.size() == 0) {
		top = selected = 0;
		selected_time = GET_TICKS();
	}
	else if (reserved_size >= 0) {
		int vr = visible_rows();
		if (top + vr <= selected) {
			top = selected - (vr-1);
		}
		if (top + vr > (int)items.size()) {
			top = (int)items.size() - vr;
		}
		if (top < 0) {
			top = 0;
		}
	}
}

//--

Widget_Quantity_List::Widget_Quantity_List(int w, int h) :
	Widget_List(w, h)
{
	longpress_started = 0;
}

Widget_Quantity_List::Widget_Quantity_List(float w, float h) :
	Widget_List(w, h)
{
	longpress_started = 0;
}

Widget_Quantity_List::Widget_Quantity_List(int w, float h) :
	Widget_List(w, h)
{
	longpress_started = 0;
}

Widget_Quantity_List::Widget_Quantity_List(float w, int h) :
	Widget_List(w, h)
{
	longpress_started = 0;
}

Widget_Quantity_List::~Widget_Quantity_List()
{
}

void Widget_Quantity_List::handle_event(TGUI_Event *event)
{
	int scrollbar_x = 0, scrollbar_x2 = 0;

	if (has_scrollbar && visible_rows() < (int)items.size()) {
		scrollbar_x = calculated_x + calculated_w - arrow_size.w - 2;
		scrollbar_x2 = calculated_x + calculated_w;
	}
	else {
		scrollbar_x = scrollbar_x2 = calculated_x + calculated_w;
	}

	if (event->type == TGUI_KEY_UP && event->keyboard.code == GLOBALS->key_b3) {
		show_description();
		return;
	}
	else if (event->type == TGUI_JOY_UP && event->joystick.button == GLOBALS->joy_b3) {
		show_description();
		return;
	}
	else if (event->type == TGUI_MOUSE_DOWN && event->mouse.is_repeat == false && gui->get_event_owner(event) == this && event->mouse.x < scrollbar_x) {
		longpress_started = GET_TICKS();
		longpress_pos = util::Point<int>(event->mouse.x, event->mouse.y);
	}
	else if (event->type == TGUI_MOUSE_AXIS && longpress_started != 0) {
		util::Point<int> pos(event->mouse.x, event->mouse.y);
		if ((pos-longpress_pos).length() > 5) {
			longpress_started = 0;
		}
	}
	else if (event->type == TGUI_MOUSE_UP && event->mouse.is_repeat == false) {
		// Widget_List::handle_event (previous line) sets selection, so all we have to do (for disabled (special) items) is show the description
		if (longpress_started != 0 && gui->get_event_owner(event) == this && event->mouse.x < scrollbar_x && is_disabled(selected) == true && selected >= 0) {
			show_description();
		}
		longpress_started = 0;
	}
	else if (event->type == TGUI_TICK) {
		bool is_longpress = longpress_started != 0;
		bool go = is_longpress || down_selected >= 0;
		if (go) {
			Uint32 u = is_longpress ? longpress_started : down_time;
			Uint32 now = GET_TICKS();
			Uint32 elapsed = now - u;
			if (elapsed >= LONGPRESS_TIME) {
				longpress_started = 0;
				down_selected = -1;
				show_description();
				// fudge some events so the item isn't used/list isn't scrolled:
				TGUI_Event ev;
				ev.type = TGUI_MOUSE_AXIS;
				ev.mouse.x = 1000000;
				ev.mouse.y = longpress_pos.y;
				ev.mouse.is_touch = false;
				ev.mouse.is_repeat = false;
				ev.mouse.normalised = false;
				Widget_List::handle_event(&ev);
				ev.type = TGUI_MOUSE_UP;
				ev.mouse.button = 1;
				Widget_List::handle_event(&ev);
				return;
			}
		}
	}

	if (event->type == TGUI_KEY_DOWN && event->keyboard.is_repeat == false && event->keyboard.code == GLOBALS->key_b1 && is_disabled(selected) == true) {
		show_description();
	}
	else if (event->type == TGUI_JOY_DOWN && event->joystick.is_repeat == false && event->joystick.button == GLOBALS->joy_b1 && is_disabled(selected) == true) {
		show_description();
	}
	else {
		Widget_List::handle_event(event);
	}
}

void Widget_Quantity_List::draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y)
{
	std::string qs = util::itos(quantities[index]);
	int len = shim::font->get_text_width(qs);

	shim::font->draw(colour, qs, util::Point<int>(calculated_x+2+max_quantity_width-len, y)); // right justified max_quantity_width pixels

	int xx = calculated_x+2+5+max_quantity_width;
	int have = calculated_w - (xx-calculated_x) - 5 - (visible_rows() < (int)items.size() ? arrow_size.w : 0);
	int totlen = shim::font->get_text_width(items[index]);
	int scroll = totlen-have + 2;
	if (totlen > have) {
		const int wait = 1000;
		Uint32 ticks = GET_TICKS() - selected_time;
		int m = scroll * 200;
		Uint32 t = ticks % (wait*2 + m);
		float o;
		if (t < wait || index != selected) {
			o = 0.0f;
		}
		else if ((int)t >= wait+m) {
			o = -scroll;
		}
		else {
			o = -((t - wait) / (float)m * scroll);
		}
		gfx::Font::end_batches();
		// These lists can be transformed, scissor doesn't understand transforms though so we have to calculate
		// However scissor already accounts for black bars, so reverse that
		glm::mat4 mv, _p;
		gfx::get_matrices(mv, _p);
		glm::vec3 zero(0.0f, 0.0f, 0.0f);
		zero = glm::project(zero, mv, _p, glm::vec4(0.0f, 0.0f, (float)shim::real_screen_size.w, (float)shim::real_screen_size.h));
		// don't just use shim::screen_size above as it has to account for black bars
		zero.x -= shim::screen_offset.x;
		zero.y -= shim::screen_offset.y;
		zero.x /= shim::scale;
		zero.y /= shim::scale;
		zero.y = shim::screen_size.h - zero.y;
		gfx::set_scissor(int(zero.x+xx), 0, have, shim::screen_size.h-1);
		shim::font->draw(colour, items[index], util::Point<float>(xx+o, y));
		gfx::Font::end_batches();
		gfx::unset_scissor();
	}
	else {
		shim::font->draw(colour, items[index], util::Point<int>(xx, y));
	}
}

void Widget_Quantity_List::set_items(std::vector< std::pair<int, std::string> > new_items)
{
	this->items.clear();
	this->quantities.clear();

	std::vector<std::string> items;
	std::vector<int> quantities;

	max_quantity_width = 0;

	for (size_t i = 0; i < new_items.size(); i++) {
		quantities.push_back(new_items[i].first);
		int w = shim::font->get_text_width(util::itos(new_items[i].first));
		if (w > max_quantity_width) {
			max_quantity_width = w;
		}
		//items.push_back(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(new_items[i].second)));
		items.push_back(new_items[i].second);
	}

	max_quantity_width++;

	this->items = items;
	this->quantities = quantities;
	
	accepts_focus = items.size() != 0;

	if (items.size() == 0) {
		selected = -1;
		if (gui != 0 && gui->get_focus() == this) {
			gui->focus_something();
		}
	}
	else {
		if (selected < 0) {
			selected = 0;
			selected_time = GET_TICKS();
		}
	}
}

void Widget_Quantity_List::show_description()
{
	if ((int)descriptions.size() <= selected) {
		return;
	}
	
	M3_GLOBALS->button->play(false);

	Notification_GUI *notification_gui = new Notification_GUI(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(descriptions[selected])));
	shim::guis.push_back(notification_gui);
}

void Widget_Quantity_List::set_descriptions(std::vector<std::string> descriptions)
{
	this->descriptions = descriptions;
}

//--

Widget_Vampire_List::Widget_Vampire_List(int w, int h) :
	Widget_List(w, h)
{
}

Widget_Vampire_List::Widget_Vampire_List(float w, float h) :
	Widget_List(w, h)
{
}

Widget_Vampire_List::Widget_Vampire_List(int w, float h) :
	Widget_List(w, h)
{
}

Widget_Vampire_List::Widget_Vampire_List(float w, int h) :
	Widget_List(w, h)
{
}

Widget_Vampire_List::~Widget_Vampire_List()
{
}

void Widget_Vampire_List::draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y)
{
	std::string s = costs[index];
	int len = shim::font->get_text_width(s);

	shim::font->draw(colour, s, util::Point<int>(calculated_x+2+max_cost_width-len, y)); // right justified max_cost_width pixels
	shim::font->draw(colour, items[index], util::Point<int>(calculated_x+2+5+max_cost_width, y));
}

void Widget_Vampire_List::set_items(std::vector< std::pair<std::string, std::string> > new_items)
{
	this->items.clear();
	this->costs.clear();

	std::vector<std::string> items;
	std::vector<std::string> costs;

	max_cost_width = 0;

	for (size_t i = 0; i < new_items.size(); i++) {
		costs.push_back(new_items[i].first);
		int w = shim::font->get_text_width(new_items[i].first);
		if (w > max_cost_width) {
			max_cost_width = w;
		}
		items.push_back(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(new_items[i].second)));
	}

	max_cost_width++;

	this->items = items;
	this->costs = costs;
	
	accepts_focus = items.size() != 0;

	if (items.size() == 0) {
		selected = -1;
		if (gui != 0 && gui->get_focus() == this) {
			gui->focus_something();
		}
	}
	else {
		if (selected < 0) {
			selected = 0;
			selected_time = GET_TICKS();
		}
	}
}

//--

Widget_Save_Game_List::Widget_Save_Game_List(int w, int h) :
	Widget_List(w, h)
{
	has_scrollbar = false;
}

Widget_Save_Game_List::Widget_Save_Game_List(float w, float h) :
	Widget_List(w, h)
{
	has_scrollbar = false;
}

Widget_Save_Game_List::Widget_Save_Game_List(int w, float h) :
	Widget_List(w, h)
{
	has_scrollbar = false;
}

Widget_Save_Game_List::Widget_Save_Game_List(float w, int h) :
	Widget_List(w, h)
{
	has_scrollbar = false;
}

Widget_Save_Game_List::~Widget_Save_Game_List()
{
	has_scrollbar = false;
}

void Widget_Save_Game_List::draw()
{
	gfx::draw_filled_rectangle(shim::palette[25], util::Point<int>(calculated_x, calculated_y), util::Size<int>(calculated_w, calculated_h));

	Widget_List::draw();
}

void Widget_Save_Game_List::draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y)
{
	std::pair<int, int> p = experience[index];

	if (p.first == -1) {
		shim::font->draw(colour, GLOBALS->game_t->translate(1546)/* Originally: - Empty - */, util::Point<int>(calculated_x+3, y));
	}
	else if (p.second == -2) {
		shim::font->draw(colour, GLOBALS->game_t->translate(1547)/* Originally: *** CORRUPT *** */, util::Point<int>(calculated_x+3, y));
	}
	else {
		shim::font->draw(colour, items[index], util::Point<int>(calculated_x+3, y));

		gfx::Image *eny_image = M3_GLOBALS->mini_profile_images[ENY];
		gfx::Image *tiggy_image = M3_GLOBALS->mini_profile_images[TIGGY];

		int exp_w = (eny_image->size.w + 2) * 2 + (shim::font->get_text_width(util::itos(p.first)) + 2) + (shim::font->get_text_width(util::itos(p.second)) + 2);
		int x = calculated_x + calculated_w - exp_w - 2;

		draw_shadowed_image(shadow_colour, eny_image, util::Point<int>(x, y+2), gfx::Font::DROP_SHADOW);
		x += eny_image->size.w + 2;
		
		shim::font->draw(colour, util::itos(p.first), util::Point<int>(x, y));

		x += shim::font->get_text_width(util::itos(p.first)) + 2;

		draw_shadowed_image(shadow_colour, tiggy_image, util::Point<int>(x, y+2), gfx::Font::DROP_SHADOW);
		x += tiggy_image->size.w + 2;

		shim::font->draw(colour, util::itos(p.second), util::Point<int>(x, y));
	}
}

void Widget_Save_Game_List::set_items(std::vector<std::string> times, std::vector< std::pair<int, int> > experience)
{
	items = times;
	this->experience = experience;
	selected = 0;
	selected_time = GET_TICKS();
}

//--

Widget_Controls_List::Widget_Controls_List(int w, int h) :
	Widget_List(w, h)
{
}

Widget_Controls_List::Widget_Controls_List(float w, float h) :
	Widget_List(w, h)
{
}

Widget_Controls_List::Widget_Controls_List(int w, float h) :
	Widget_List(w, h)
{
}

Widget_Controls_List::Widget_Controls_List(float w, int h) :
	Widget_List(w, h)
{
}

Widget_Controls_List::~Widget_Controls_List()
{
}

void Widget_Controls_List::draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y)
{
	int arrow_w = M3_GLOBALS->up_arrow->size.w;
	shim::font->draw(shim::white, GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(items[index])), util::Point<int>(calculated_x+3, y));
	int w = shim::font->get_text_width(assignments[index]);
	int xo;
	if (has_scrollbar && visible_rows() < (int)items.size()) {
		xo = arrow_w+6;
	}
	else {
		xo = 4;
	}
	shim::font->draw(shim::white, assignments[index], util::Point<int>(calculated_x+calculated_w-w-xo, y));
}

void Widget_Controls_List::set_items(std::vector<std::string> names, std::vector<std::string> assignments)
{
	items = names;
	this->assignments = assignments;
	selected = 0;
	selected_time = GET_TICKS();
}

//--

Widget_Spell_Target_List::Widget_Spell_Target_List(int w, int h) :
	Widget_List(w, h)
{
}

Widget_Spell_Target_List::Widget_Spell_Target_List(float w, float h) :
	Widget_List(w, h)
{
}

Widget_Spell_Target_List::Widget_Spell_Target_List(int w, float h) :
	Widget_List(w, h)
{
}

Widget_Spell_Target_List::Widget_Spell_Target_List(float w, int h) :
	Widget_List(w, h)
{
	mini_profile_pics[GLOBALS->game_t->translate(0)/* Originally: Eny */] = M3_GLOBALS->mini_profile_images[ENY];
	mini_profile_pics[GLOBALS->game_t->translate(1)/* Originally: Tiggy */] = M3_GLOBALS->mini_profile_images[TIGGY];
}

Widget_Spell_Target_List::~Widget_Spell_Target_List()
{
}

void Widget_Spell_Target_List::draw_row(int index, SDL_Colour colour, SDL_Colour shadow_colour, int y)
{
	std::string item = items[index];
	gfx::Image *mini_pic = mini_profile_pics[item];
	if (mini_pic) {
		int x_add = 1 + mini_pic->size.w;
		draw_shadowed_image(shadow_colour, mini_pic, util::Point<int>(calculated_x+calculated_w-3-x_add, y+2), gfx::Font::DROP_SHADOW);
		shim::font->draw(colour, item, util::Point<int>(calculated_x+3, y));
	}
	else {
		gfx::Image *eny_pic = mini_profile_pics[GLOBALS->game_t->translate(0)/* Originally: Eny */];
		gfx::Image *tiggy_pic = mini_profile_pics[GLOBALS->game_t->translate(1)/* Originally: Tiggy */];
		int x_add = 1 + eny_pic->size.w;
		draw_shadowed_image(shadow_colour, tiggy_pic, util::Point<int>(calculated_x+calculated_w-3-x_add, y+2), gfx::Font::DROP_SHADOW);
		draw_shadowed_image(shadow_colour, eny_pic, util::Point<int>(calculated_x+calculated_w-3-x_add*2-1, y+2), gfx::Font::DROP_SHADOW);
		shim::font->draw(colour, item, util::Point<int>(calculated_x+3, y));
	}
}

//--

Widget_Checkbox::Widget_Checkbox(std::string text, bool checked) :
	Widget(0, 0),
	text(text),
	checked(checked)
{
	w = shim::font->get_text_width(text) + 6 + Widget_Slider::TAB_SIZE;
	h = MAX(shim::font->get_height(), Widget_Slider::TAB_SIZE);

	accepts_focus = true;

	mouse_down = false;
}

Widget_Checkbox::~Widget_Checkbox()
{
}

void Widget_Checkbox::handle_event(TGUI_Event *event)
{
	if (accepts_focus == false) {
		return;
	}

	bool focussed = is_focussed();

	int xx = calculated_x;
	int yy = calculated_y;

	bool was_checked = checked;

	if ((event->type == TGUI_MOUSE_DOWN || event->type == TGUI_MOUSE_AXIS) && event->mouse.is_repeat == false) {
		if (event->mouse.x >= xx && event->mouse.x < xx+calculated_w && event->mouse.y >= yy && event->mouse.y < yy+calculated_h) {
			mouse_down = true;
		}
		else {
			mouse_down = false;
		}
	}
	if (event->type == TGUI_MOUSE_UP && event->mouse.is_repeat == false) {
		if (event->mouse.x >= xx && event->mouse.x < xx+calculated_w && event->mouse.y >= yy && event->mouse.y < yy+calculated_h) {
			if (mouse_down) {
				mouse_down = false;
				checked = !checked;
			}
		}
	}
	else if (focussed && event->type == TGUI_KEY_UP && event->keyboard.code == GLOBALS->key_b1) {
		checked = !checked;
	}
	else if (focussed && event->type == TGUI_JOY_UP && event->joystick.button == GLOBALS->joy_b1) {
		checked = !checked;
	}

	if (was_checked != checked) {
		M3_GLOBALS->button->play(false);
	}
}

void Widget_Checkbox::draw()
{
	bool focussed = is_focussed();

	int text_y = calculated_y + h/2 - shim::font->get_height() / 2;

	if (accepts_focus == false) {
		shim::font->draw(shim::black, text, util::Point<int>(calculated_x+1, text_y+1), true, false);
	}
	else {
		shim::font->enable_shadow(shim::black, gfx::Font::DROP_SHADOW);
		shim::font->draw(shim::white, text, util::Point<int>(calculated_x+1, text_y), true, false);
		shim::font->disable_shadow();
	}

	float check_x = calculated_x + 5 + shim::font->get_text_width(text);
	float check_y = calculated_y + h/2.0f - Widget_Slider::TAB_SIZE/2.0f;

	if (focussed) {
		start_pulse_brighten(0.25f, true, false);
	}

	gfx::draw_filled_rectangle(shim::black, util::Point<float>(check_x, check_y), util::Size<int>(Widget_Slider::TAB_SIZE, Widget_Slider::TAB_SIZE));

	if (focussed) {
		gfx::draw_rectangle(shim::palette[14], util::Point<int>(calculated_x-2, calculated_y-2), util::Size<int>(w+4, h+4));
		end_pulse_brighten();
	}

	if (checked) {
		float little = get_little_bander_offset();

		SDL_Colour *colours = start_bander(3, shim::palette[13], shim::palette[16]);
		gfx::draw_filled_rectangle(colours, util::Point<float>(check_x+1-little, check_y+1-little), util::Size<float>(Widget_Slider::TAB_SIZE-2+little*2.0f, Widget_Slider::TAB_SIZE-2+little*2.0f));
		end_bander();
	}
}

bool Widget_Checkbox::is_checked()
{
	return checked;
}

void Widget_Checkbox::set_disabled(bool disabled)
{
	if (disabled) {
		accepts_focus = false;
		// FIXME: if focussed, focus something else
	}
	else {
		accepts_focus = true;
	}
}

//--

Widget_Map::Widget_Map() :
	image(NULL),
	image2(NULL)
{
	w = 1;
	h = 1;
}

Widget_Map::~Widget_Map()
{
}

void Widget_Map::draw()
{
	int dx = calculated_x - image->size.w/2;
	int dy = calculated_y - image->size.h/2;

	if (image != NULL && image2 != NULL) {
		gfx::draw_filled_rectangle(shim::black, util::Point<int>(dx, dy), image->size);
		int t = GET_TICKS() % 1000;
		float p = t / 1000.0f;
		if (p < 0.5f) {
			p = p / 0.5f;
		}
		else {
			p = 1.0f - ((p - 0.5f) / 0.5f);
		}
		SDL_Colour tint = shim::white;
		tint.r *= p;
		tint.g *= p;
		tint.b *= p;
		tint.a *= p;
		image2->draw_tinted(tint, util::Point<int>(dx, dy));
		image->draw(util::Point<int>(dx, dy));
	}
	
	bool focussed = is_focussed();

	if (focussed && image != NULL) {
		start_pulse_brighten(0.25f, true, false);
		gfx::draw_rectangle(shim::palette[14], util::Point<int>(dx-1, dy-1), image->size+util::Size<int>(2, 2));
		end_pulse_brighten();
	}

}

void Widget_Map::set_images(gfx::Image *image, gfx::Image *image2)
{
	this->image = image;
	this->image2 = image2;
}


void Widget_Map::handle_event(TGUI_Event *event)
{
	int w, h;
	if (image) {
		w = image->size.w;
		h = image->size.h;
	}
	else {
		w = h = 0;
	}
	bool is_click = false;
	if (event->type == TGUI_MOUSE_DOWN) {
		if (event->mouse.x >= calculated_x-w/2 && event->mouse.y >= calculated_y-h/2 && event->mouse.x < calculated_x+w/2 && event->mouse.y < calculated_y+h/2) {
			is_click = true;
		}
	}
	if (is_click || (event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b1) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b1)) {
		M3_GLOBALS->button->play(false);
		Notification_GUI *gui = new Notification_GUI(
			"|0a" + GLOBALS->game_t->translate(0) + "$" +
			"|0d" + GLOBALS->game_t->translate(1) + "$" +
			"|00" + GLOBALS->game_t->translate(1740)
		);

		shim::guis.push_back(gui);
	}
}
