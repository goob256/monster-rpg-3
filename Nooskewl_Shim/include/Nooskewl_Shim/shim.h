#ifndef NOO_SHIM_H
#define NOO_SHIM_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace audio {
	class MML;
}
namespace gfx {
	class CPA;
	class Font;
	class Image;
	class Shader;
}
namespace gui {
	class GUI;
}
namespace util {
	class CPA;
	class Translation;
}

namespace shim {

// Must be called first thing
bool NOOSKEWL_SHIM_EXPORT static_start(int sdl_init_flags = 0);
bool NOOSKEWL_SHIM_EXPORT static_start_all(int sdl_init_flags = 0);

// Matches either static_start or static_start_all, call before exiting program
void NOOSKEWL_SHIM_EXPORT static_end();

// Call either start and end (if you init subsystems individually) or start_all and end_all

bool NOOSKEWL_SHIM_EXPORT start();
void NOOSKEWL_SHIM_EXPORT end();

/* scaled_gfx_* is the size you want to work with. For example you may want
 * a pixelated game of 240x160 pixels, but a bigger window where the pixels
 * are scaled up. gfx_window_* provides the window size (or fullscreen
 * resolution if going fullscreen.) force_integer_scaling allows you to get a size as
 * close to scaled_gfx_* as possible while keeping an integer scaling i.e.,
 * perfectly square pixels at each location.  If only gfx_window_* are -1,
 * we try to get a window size in a multiple of scaled_gfx_* at a big size
 * that fits the screen.  If you pass -1 for scaled_gfx_*, no scaling will
 * be done i.e., you work with gfx_window_* resolution. If all parameters
 * are -1/false, a fullscreen mode at the current desktop resolution is
 * used.
 */
bool NOOSKEWL_SHIM_EXPORT start_all(int scaled_gfx_w = -1, int scaled_gfx_h = -1, bool force_integer_scaling = false, int gfx_window_w = -1, int gfx_window_h = -1);
void NOOSKEWL_SHIM_EXPORT end_all();

TGUI_Event NOOSKEWL_SHIM_EXPORT *handle_event(SDL_Event *sdl_event);
// if this returns false, don't draw until it returns true again
bool NOOSKEWL_SHIM_EXPORT update();
void NOOSKEWL_SHIM_EXPORT push_event(TGUI_Event event);
TGUI_Event NOOSKEWL_SHIM_EXPORT *pop_pushed_event();
bool NOOSKEWL_SHIM_EXPORT event_in_queue(TGUI_Event e);

// These are all global to the shim namespace
// graphics
extern NOOSKEWL_SHIM_EXPORT bool opengl;
extern NOOSKEWL_SHIM_EXPORT SDL_Colour palette[256];
extern NOOSKEWL_SHIM_EXPORT SDL_Colour black;
extern NOOSKEWL_SHIM_EXPORT SDL_Colour white;
extern NOOSKEWL_SHIM_EXPORT SDL_Colour magenta;
extern NOOSKEWL_SHIM_EXPORT SDL_Colour transparent;
extern NOOSKEWL_SHIM_EXPORT std::vector<gui::GUI *> guis;
extern NOOSKEWL_SHIM_EXPORT float scale;
extern NOOSKEWL_SHIM_EXPORT std::string window_title; // set this first thing to change it (before call to shim::start)
extern NOOSKEWL_SHIM_EXPORT std::string organization_name; // set this first thing too
extern NOOSKEWL_SHIM_EXPORT std::string game_name; // set this first thing too
extern NOOSKEWL_SHIM_EXPORT gfx::Shader *current_shader;
extern NOOSKEWL_SHIM_EXPORT gfx::Shader *default_shader;
extern NOOSKEWL_SHIM_EXPORT gfx::Shader *model_shader;
extern NOOSKEWL_SHIM_EXPORT int tile_size;
extern NOOSKEWL_SHIM_EXPORT util::Size<int> screen_size; // before scaling
extern NOOSKEWL_SHIM_EXPORT util::Size<int> real_screen_size; // actual window size
extern NOOSKEWL_SHIM_EXPORT gfx::Font *font;
extern NOOSKEWL_SHIM_EXPORT bool create_depth_buffer;
extern NOOSKEWL_SHIM_EXPORT bool create_stencil_buffer;
extern NOOSKEWL_SHIM_EXPORT util::Size<int> depth_buffer_size;
extern NOOSKEWL_SHIM_EXPORT util::Point<int> screen_offset; // begin of where game is drawn after black bars
extern NOOSKEWL_SHIM_EXPORT float black_bar_percent;
extern NOOSKEWL_SHIM_EXPORT float z_add; // added to all z values for drawing images/primitives
extern void NOOSKEWL_SHIM_EXPORT (*user_render)();
extern NOOSKEWL_SHIM_EXPORT int refresh_rate;
extern NOOSKEWL_SHIM_EXPORT bool hide_window;
extern NOOSKEWL_SHIM_EXPORT int adapter;
extern NOOSKEWL_SHIM_EXPORT bool use_hires_font;
#ifdef _WIN32
extern NOOSKEWL_SHIM_EXPORT IDirect3DDevice9 *d3d_device;
#endif
// audio
extern NOOSKEWL_SHIM_EXPORT audio::MML *music;
extern NOOSKEWL_SHIM_EXPORT audio::MML *widget_mml;
extern NOOSKEWL_SHIM_EXPORT float music_volume;
extern NOOSKEWL_SHIM_EXPORT float sfx_volume;
extern NOOSKEWL_SHIM_EXPORT int samplerate;
// input
extern NOOSKEWL_SHIM_EXPORT bool convert_xbox_dpad_to_arrows;
extern NOOSKEWL_SHIM_EXPORT int xbox_l;
extern NOOSKEWL_SHIM_EXPORT int xbox_r;
extern NOOSKEWL_SHIM_EXPORT int xbox_u;
extern NOOSKEWL_SHIM_EXPORT int xbox_d;
extern NOOSKEWL_SHIM_EXPORT int key_l;
extern NOOSKEWL_SHIM_EXPORT int key_r;
extern NOOSKEWL_SHIM_EXPORT int key_u;
extern NOOSKEWL_SHIM_EXPORT int key_d;
extern NOOSKEWL_SHIM_EXPORT int fullscreen_key; // key toggles fullscreen window if set
extern NOOSKEWL_SHIM_EXPORT bool convert_directions_to_focus_events;
extern NOOSKEWL_SHIM_EXPORT bool ignore_hat_diagonals;
extern NOOSKEWL_SHIM_EXPORT float joystick_activate_threshold;
extern NOOSKEWL_SHIM_EXPORT float joystick_deactivate_threshold;
extern NOOSKEWL_SHIM_EXPORT bool mouse_button_repeats;
extern NOOSKEWL_SHIM_EXPORT int mouse_button_repeat_max_movement;
extern NOOSKEWL_SHIM_EXPORT bool dpad_below;
extern NOOSKEWL_SHIM_EXPORT bool allow_dpad_below;
extern NOOSKEWL_SHIM_EXPORT bool dpad_enabled;
extern NOOSKEWL_SHIM_EXPORT int joy_index;
// other
extern NOOSKEWL_SHIM_EXPORT util::CPA *cpa;
extern NOOSKEWL_SHIM_EXPORT int notification_duration; // in millis
extern NOOSKEWL_SHIM_EXPORT int notification_fade_duration; // in millis
extern NOOSKEWL_SHIM_EXPORT Uint32 timer_event_id;
extern NOOSKEWL_SHIM_EXPORT int argc;
extern NOOSKEWL_SHIM_EXPORT char **argv;
// this is for loading data from the EXE
extern NOOSKEWL_SHIM_EXPORT int cpa_extra_bytes_after_exe_data;
// these two are for loading data from a memory buffer
extern NOOSKEWL_SHIM_EXPORT Uint8 *cpa_pointer_to_data;
extern NOOSKEWL_SHIM_EXPORT int cpa_data_size;
extern NOOSKEWL_SHIM_EXPORT bool logging;
// game should always set this to logic ticks per second (default is 60)
// Note that this doesn't control execution speed (that is up to your main loop) but it tells some things how fast your main loop is running
extern NOOSKEWL_SHIM_EXPORT int logic_rate;
extern NOOSKEWL_SHIM_EXPORT bool use_cwd;
extern NOOSKEWL_SHIM_EXPORT bool log_tags;
extern NOOSKEWL_SHIM_EXPORT int error_level; // 0=none, 1=errors, 2=info, 3=debug/opengl
#ifdef TVOS
extern NOOSKEWL_SHIM_EXPORT bool pass_menu_to_os;
#endif

} // End namespace shim

} // End namespace noo

#endif // NOO_SHIM_H
