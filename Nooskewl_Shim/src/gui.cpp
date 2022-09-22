#include "Nooskewl_Shim/font.h"
#include "Nooskewl_Shim/gfx.h"
#include "Nooskewl_Shim/gui.h"
#include "Nooskewl_Shim/image.h"
#include "Nooskewl_Shim/mml.h"
#include "Nooskewl_Shim/shim.h"
#include "Nooskewl_Shim/sprite.h"
#include "Nooskewl_Shim/translation.h"
#include "Nooskewl_Shim/util.h"

#include "Nooskewl_Shim/internal/gfx.h"

using namespace noo;

#if defined __APPLE__ && !defined IOS
#include "Nooskewl_Shim/macosx.h"
#endif

#ifdef __linux__
#include "Nooskewl_Shim/x.h"
#endif

namespace noo {

namespace gui {

GUI::GUI() :
	gui(0),
	focus(0),
	started_transition_timer(false)
{
	transition = false; // set this true to do transitions
	transitioning_in = true;
	transitioning_out = false;
	transition_is_enlarge = false;
	transition_is_shrink = false;
}

GUI::~GUI()
{
	delete gui;
}

void GUI::handle_event(TGUI_Event *event) {
	if (gui) {
		TGUI_Widget *focus = gui->get_focus();
		gui->handle_event(event);
		if (event->type == TGUI_FOCUS && focus != gui->get_focus()) {
			if (shim::widget_mml != 0) {
				shim::widget_mml->play(false);
			}
		}
	}
}

void GUI::draw_back()
{
	if (transition == false) {
		transitioning_in = false; // for guis that don't transition in (otherwise out looks like in)
		return;
	}

	float p = (SDL_GetTicks() - transition_start_time) / 200.0f;
	if (p >= 1.0f) {
		p = 1.0f;
	}

	if (transitioning_in) {
		if (p >= 1.0f) {
			p = 1.0f;
			transitioning_in = false;
			transition_done(true);
			transition_in_done();
		}
		else {
			transition_start(p);
		}
	}
	else if (transitioning_out) {
		transition_start(p);
	}
}

void GUI::draw()
{
	if (gui) {
		gui->draw();
	}
}

void GUI::draw_fore()
{
	if (transition == false) {
		return;
	}

	if (transitioning_out || transitioning_in) {
		transition_end();
	}
}

void GUI::resize(util::Size<int> size)
{
	gui->resize(size.w, size.h);
}

bool GUI::is_fullscreen()
{
	return false;
}

bool GUI::is_transition_out_finished() {
	if (transitioning_out) {
		if (transition == false) {
			return true;
		}
		else if ((SDL_GetTicks() - transition_start_time) >= 200) {
			if (transition_done(false) == false) {
				return true;
			}
			else {
				transitioning_out = false;
			}
		}
	}

	return false;
}

void GUI::exit()
{
	transitioning_out = true;

	if (transition) {
		transition_start_time = SDL_GetTicks();
	}
}

void GUI::transition_start(float p)
{
	if (transitioning_in) {
		if (transition_is_enlarge || transition_is_shrink) {
			float scale;
			if (transition_is_enlarge) {
				scale = 1.0f + (1.0f - p) * (MAX_FADE_SCALE-1);
			}
			else {
				scale = p;
			}
			scale_transition(scale);
		}
		else {
			fade_transition(p);
		}
	}
	else {
		if (transition_is_enlarge || transition_is_shrink) {
			float scale;
			if (transition_is_enlarge) {
				scale = 1.0f + p * (MAX_FADE_SCALE-1);
			}
			else {
				scale = 1.0f - p;
			}
			scale_transition(scale);
		}
		else {
			p = 1.0f - p;
			fade_transition(p);
		}
	}
}

void GUI::transition_end()
{
	gfx::Font::end_batches();

	if (transition_is_enlarge || transition_is_shrink) {
		glm::mat4 mv, p;
		gfx::get_matrices(mv, p);
		gfx::set_matrices(mv_backup, p);
		gfx::update_projection();
	}
	else {
		gfx::set_target_backbuffer();
		Uint8 c = Uint8(last_transition_p * 255);
		SDL_Colour whitish = { c, c, c, c };
		glm::mat4 mv_backup, proj_backup, mv;
		gfx::get_matrices(mv_backup, proj_backup);
		mv = glm::mat4();
		gfx::set_matrices(mv, proj_backup);
		gfx::update_projection();
		gfx::internal::gfx_context.work_image->draw_tinted(whitish, util::Point<int>(0, 0));
		gfx::set_matrices(mv_backup, proj_backup);
		gfx::update_projection();
	}
}

void GUI::fade_transition(float p)
{
	last_transition_p = p;
	gfx::set_target_image(gfx::internal::gfx_context.work_image);
	SDL_Colour transparent = { 0, 0, 0, 0 };
	gfx::clear(transparent);
}

void GUI::scale_transition(float scale)
{
	scale *= shim::scale;
	int new_w = int(shim::screen_size.w * scale);
	int new_h = int(shim::screen_size.h * scale);
	int w_diff = (new_w - shim::real_screen_size.w) / 2;
	int h_diff = (new_h - shim::real_screen_size.h) / 2;
	if (shim::allow_dpad_below) {
		int o = (shim::real_screen_size.h - (shim::screen_size.h*shim::scale)) / 2 - shim::screen_offset.y;
		h_diff += o;
	}
	glm::mat4 mv, p;
	gfx::get_matrices(mv_backup, p);
	mv = glm::mat4();
	mv = glm::translate(mv, glm::vec3(-w_diff, -h_diff, 0.0f));
	mv = glm::scale(mv, glm::vec3(scale, scale, 1.0f));
	gfx::set_matrices(mv, p);
	gfx::update_projection();
}

void GUI::use_enlarge_transition(bool onoff)
{
	transition_is_enlarge = onoff;
}

void GUI::use_shrink_transition(bool onoff)
{
	transition_is_shrink = onoff;
}

//--

int popup(std::string caption, std::string text, Popup_Type type)
{
#ifdef _WIN32
	UINT native_type;
	if (type == OK) {
		native_type = MB_OK;
	}
	else if (type == YESNO) {
		native_type = MB_YESNO;
	}
	else {
		return -1;
	}
	int ret = MessageBox(gfx::internal::gfx_context.hwnd, text.c_str(), caption.c_str(), native_type);
	int result;
	if (type == OK) {
		result = 0;
	}
	else if (type == YESNO) {
		if (ret == IDYES) {
			result = 1;
		}
		else {
			result = 0;
		}
	}
	else {
		result = -1;
	}
	return result;
#elif defined __APPLE__ && !defined IOS
	return macosx_popup(caption, text, type);
#elif !defined ANDROID && !defined IOS && !defined RASPBERRYPI
	return x_popup(caption, text, type);
#else
	return -1;
#endif
}
	
void GUI::lost_device()
{
}

void GUI::found_device()
{
}

void GUI::transition_in_done()
{
}

void GUI::set_transition(bool transition)
{
	this->transition = transition;
}

void GUI::pre_draw()
{
	if (started_transition_timer == false) {
		transition_start_time = SDL_GetTicks();
		started_transition_timer = true;
	}
}

bool GUI::is_transitioning_out()
{
	return transitioning_out;
}

void GUI::update()
{
}

void GUI::update_background()
{
}

bool GUI::transition_done(bool transition_in)
{
	return false;
}

#ifdef _WIN32
void *GUI::operator new(size_t i)
{
	return _mm_malloc(i,16);
}

void GUI::operator delete(void* p)
{
	_mm_free(p);
}
#endif

} // End namespace gui

} // End namespace noo
