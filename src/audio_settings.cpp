#include "Nooskewl_Wedge/area_game.h"

#include "audio_settings.h"
#include "general.h"
#include "globals.h"
#include "monster_rpg_3.h"
#include "widgets.h"

Audio_Settings_GUI::Audio_Settings_GUI()
{
	transition = true;
	transition_is_enlarge = true;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	modal_main_widget->set_background_colour(shim::palette[24]);

	Widget *container = new Widget();
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_parent(modal_main_widget);

	Widget_Label *sfx_label = new Widget_Label(GLOBALS->game_t->translate(1326)/* Originally: SFX Volume: */, -1);
	sfx_label->set_centre_x(true);
	sfx_label->set_shadow_colour(shim::black);
	sfx_label->set_padding_top(10);
	sfx_label->set_parent(container);

	sfx_slider = new Widget_Slider(101, 101, sfx_volume * 100);
	sfx_slider->set_clear_float_x(true);
	sfx_slider->set_centre_x(true);
	sfx_slider->set_padding_top(13+sfx_label->get_height());
	sfx_slider->set_parent(container);

	Widget_Label *music_label = new Widget_Label(GLOBALS->game_t->translate(1327)/* Originally: Music Volume: */, -1);
	music_label->set_clear_float_x(true);
	music_label->set_centre_x(true);
	music_label->set_padding_top(23+sfx_label->get_height()+sfx_slider->get_height());
	music_label->set_shadow_colour(shim::black);
	music_label->set_parent(container);

	music_slider = new Widget_Slider(101, 101, music_volume * 100);
	music_slider->set_clear_float_x(true);
	music_slider->set_centre_x(true);
	music_slider->set_padding_top(26+sfx_label->get_height()+sfx_slider->get_height()+music_label->get_height());
	music_slider->set_parent(container);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Audio_Settings_GUI::~Audio_Settings_GUI()
{
}

void Audio_Settings_GUI::handle_event(TGUI_Event *event)
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

void Audio_Settings_GUI::update()
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	sfx_volume = sfx_slider->get_value() / 100.0f;
	shim::sfx_volume = sfx_volume * sfx_amp;
	float vol = music_slider->get_value() / 100.0f;
	if (vol != music_volume) {
		music_volume = vol;
		shim::music_volume = music_volume * music_amp;
		bool paused = AREA != NULL && AREA->is_paused();
		float maybe_paused = paused ? 0.333f : 1.0f;
		shim::music->set_master_volume(vol * maybe_paused);
	}
}

void Audio_Settings_GUI::draw_fore()
{
	GLOBALS->bold_font->enable_shadow(shim::black, gfx::Font::DROP_SHADOW);
	GLOBALS->bold_font->draw(shim::white, GLOBALS->game_t->translate(1328)/* Originally: Audio Settings */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	GLOBALS->bold_font->disable_shadow();

	gfx::draw_rectangle(shim::palette[13], util::Point<int>(-1, -1), shim::screen_size+util::Size<int>(2, 2));

	GUI::draw_fore();
}

void Audio_Settings_GUI::transition_in_done()
{
	transition_is_enlarge = false;
	transition_is_shrink = true;
}
