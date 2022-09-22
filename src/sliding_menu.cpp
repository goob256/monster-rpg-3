#include "general.h"
#include "globals.h"
#include "sliding_menu.h"

Sliding_Menu_GUI::Sliding_Menu_GUI() :
	fading_in(true),
	fading_out(false)
{
	fade_start = GET_TICKS();
	clear_colour = shim::palette[24];
}

Sliding_Menu_GUI::~Sliding_Menu_GUI()
{
}

void Sliding_Menu_GUI::update()
{
	if (fading_in || fading_out) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - fade_start;
		if (elapsed >= FADE_TIME) {
			if (fading_out) {
				exit();
			}
			else {
				fading_in = false;
			}
		}
	}
}

void Sliding_Menu_GUI::draw_fore()
{
	if (GLOBALS->bold_font->get_text_width(caption) >= shim::screen_size.w) {
		shim::font->enable_shadow(shim::black, gfx::Font::DROP_SHADOW);
		shim::font->draw(shim::white, caption, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
		shim::font->disable_shadow();
	}
	else {
		GLOBALS->bold_font->enable_shadow(shim::black, gfx::Font::DROP_SHADOW);
		GLOBALS->bold_font->draw(shim::white, caption, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
		GLOBALS->bold_font->disable_shadow();
	}

	gfx::Font::end_batches();
	
	if (fading_in || fading_out) {
		gfx::set_matrices(modelview, proj);
		gfx::update_projection();
	}
	
	gui::GUI::draw_fore();
}

void Sliding_Menu_GUI::draw()
{
	if (fading_in || fading_out) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - fade_start;
		float p = elapsed / (float)FADE_TIME;
		if (p > 1.0f) {
			p = 1.0f;
		}
		if (fading_in) {
			p = 1.0f - p;
		}

		p = p * p;

		gfx::get_matrices(modelview, proj);

		glm::mat4 mv = glm::translate(modelview, glm::vec3(0.0f, -(shim::screen_size.h+4+M3_GLOBALS->bottom_shadow->size.h)*p, 0.0f));

		gfx::set_matrices(mv, proj);

		gfx::update_projection();
	}

	gfx::draw_filled_rectangle(clear_colour, util::Point<int>(0, 0), shim::screen_size);

	for (int i = 0; i < 4; i++) {
		gfx::draw_filled_rectangle(shim::palette[13+i], util::Point<int>(0, shim::screen_size.h+i), util::Size<int>(shim::screen_size.w, 1));
	}

	M3_GLOBALS->bottom_shadow->stretch_region(util::Point<int>(0, 0), M3_GLOBALS->bottom_shadow->size, util::Point<int>(0, shim::screen_size.h+4), util::Size<int>(shim::screen_size.w, M3_GLOBALS->bottom_shadow->size.h));

	int x = list->get_x();
	int y = list->get_y();
	int w = list->get_width();
	int h = list->get_height();
	gfx::draw_filled_rectangle(shim::palette[25], util::Point<int>(x, y), util::Size<int>(w, h));
	gui::GUI::draw();
}

void Sliding_Menu_GUI::set_caption(std::string caption)
{
	this->caption = caption;
}

void Sliding_Menu_GUI::set_fading(bool in, bool out)
{
	fading_in = in;
	fading_out = out;
}
