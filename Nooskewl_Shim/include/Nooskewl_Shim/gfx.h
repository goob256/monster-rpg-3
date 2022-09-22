#ifndef NOO_GFX_H
#define NOO_GFX_H

#include "Nooskewl_Shim/main.h"
#include "Nooskewl_Shim/model.h"

namespace noo {

namespace gfx {

class Image;

enum Compare_Func {
	COMPARE_NEVER = 1,
	COMPARE_LESS,
	COMPARE_EQUAL,
	COMPARE_LESSEQUAL,
	COMPARE_GREATER,
	COMPARE_NOTEQUAL,
	COMPARE_GREATEREQUAL,
	COMPARE_ALWAYS
};

enum Stencil_Op {
	STENCILOP_KEEP = 1,
	STENCILOP_ZERO,
	STENCILOP_REPLACE,
	STENCILOP_INCRSAT,
	STENCILOP_DECRSAT,
	STENCILOP_INVERT,
	STENCILOP_INCR,
	STENCILOP_DECR
};

enum Faces {
	NO_FACE = 0,
	FRONT_FACE,
	BACK_FACE
};

enum Blend_Mode {
	BLEND_ZERO = 0,
	BLEND_ONE,
	BLEND_SRCCOLOR,
	BLEND_INVSRCCOLOR,
	BLEND_SRCALPHA,
	BLEND_INVSRCALPHA
};

// See comments in shim.h for what these parameters are.
void NOOSKEWL_SHIM_EXPORT static_start();
void NOOSKEWL_SHIM_EXPORT static_end();

bool NOOSKEWL_SHIM_EXPORT start(int scaled_w = -1, int scaled_h = -1, bool force_integer_scaling = false, int window_w = -1, int window_h = -1);
bool NOOSKEWL_SHIM_EXPORT restart(int scaled_w = -1, int scaled_h = -1, bool force_integer_scaling = false, int window_w = -1, int window_h = -1);
void NOOSKEWL_SHIM_EXPORT end();

void NOOSKEWL_SHIM_EXPORT update_animations();

void NOOSKEWL_SHIM_EXPORT draw_guis();
void NOOSKEWL_SHIM_EXPORT draw_notifications();
void NOOSKEWL_SHIM_EXPORT flip();

void NOOSKEWL_SHIM_EXPORT clear(SDL_Colour colour);
void NOOSKEWL_SHIM_EXPORT clear_depth_buffer(float value);
void NOOSKEWL_SHIM_EXPORT clear_stencil_buffer(int value);
void NOOSKEWL_SHIM_EXPORT clear_buffers();

void NOOSKEWL_SHIM_EXPORT enable_depth_test(bool onoff);
void NOOSKEWL_SHIM_EXPORT enable_depth_write(bool onoff);
void NOOSKEWL_SHIM_EXPORT set_depth_mode(Compare_Func func);

void NOOSKEWL_SHIM_EXPORT enable_stencil(bool onoff);
void NOOSKEWL_SHIM_EXPORT enable_two_sided_stencil(bool onoff);
void NOOSKEWL_SHIM_EXPORT set_stencil_mode(Compare_Func func, Stencil_Op fail, Stencil_Op zfail, Stencil_Op pass, int reference, int mask);
void NOOSKEWL_SHIM_EXPORT set_stencil_mode_backfaces(Compare_Func func, Stencil_Op fail, Stencil_Op zfail, Stencil_Op pass, int reference, int mask);

void NOOSKEWL_SHIM_EXPORT set_cull_mode(Faces cull);

void NOOSKEWL_SHIM_EXPORT enable_blending(bool onoff);
void NOOSKEWL_SHIM_EXPORT set_blend_mode(Blend_Mode source, Blend_Mode dest);
bool NOOSKEWL_SHIM_EXPORT is_blending_enabled();

void NOOSKEWL_SHIM_EXPORT enable_colour_write(bool onoff);

void NOOSKEWL_SHIM_EXPORT set_default_projection(util::Size<int> screen_size, util::Point<int> screen_offset, float scale);
void NOOSKEWL_SHIM_EXPORT get_matrices(glm::mat4 &modelview, glm::mat4 &proj);
void NOOSKEWL_SHIM_EXPORT set_matrices(glm::mat4 &modelview, glm::mat4 &proj);
void NOOSKEWL_SHIM_EXPORT update_projection();

void NOOSKEWL_SHIM_EXPORT set_scissor(int x, int y, int w, int h);
void NOOSKEWL_SHIM_EXPORT unset_scissor();

Image NOOSKEWL_SHIM_EXPORT *get_target_image();
void NOOSKEWL_SHIM_EXPORT set_target_image(Image *image);
void NOOSKEWL_SHIM_EXPORT set_target_backbuffer();

// warning: lost can be called multiple times consecutively without a found (if delete'ing stuff, set it to NULL)
void NOOSKEWL_SHIM_EXPORT register_lost_device_callbacks(void (*lost)(), void (*found)());

void NOOSKEWL_SHIM_EXPORT set_minimum_window_size(util::Size<int> size);
void NOOSKEWL_SHIM_EXPORT set_maximum_window_size(util::Size<int> size);
void NOOSKEWL_SHIM_EXPORT set_min_aspect_ratio(float min); // anything over/under gets black bars. default: 4.0f / 3.0f.
void NOOSKEWL_SHIM_EXPORT set_max_aspect_ratio(float max); // anything over/under gets black bars. default: 16.0f / 9.0f.

util::Size<int> NOOSKEWL_SHIM_EXPORT get_desktop_resolution();
std::vector< util::Size<int> > NOOSKEWL_SHIM_EXPORT get_supported_video_modes();

void NOOSKEWL_SHIM_EXPORT show_mouse_cursor(bool show);

void NOOSKEWL_SHIM_EXPORT load_palette(std::string name);

void NOOSKEWL_SHIM_EXPORT add_notification(std::string text);
std::string NOOSKEWL_SHIM_EXPORT get_current_notification();
void NOOSKEWL_SHIM_EXPORT cancel_current_notification();

void NOOSKEWL_SHIM_EXPORT draw_9patch_tinted(SDL_Colour tint, Image *image, util::Point<int> dest_position, util::Size<int> dest_size);
void NOOSKEWL_SHIM_EXPORT draw_9patch(Image *image, util::Point<int> dest_position, util::Size<int> dest_size);

void NOOSKEWL_SHIM_EXPORT reset_fancy_draw();
void NOOSKEWL_SHIM_EXPORT fancy_draw(SDL_Colour colour, std::string text, util::Point<int> position);

void NOOSKEWL_SHIM_EXPORT set_custom_mouse_cursor(); // can be needed in some cases but rarely

void NOOSKEWL_SHIM_EXPORT set_screen_size(util::Size<int> size);

int NOOSKEWL_SHIM_EXPORT get_max_comfortable_scale(util::Size<int> scaled_size);

bool NOOSKEWL_SHIM_EXPORT enable_press_and_hold(bool enable);

#ifdef _WIN32
bool NOOSKEWL_SHIM_EXPORT is_d3d_lost();
#endif

bool NOOSKEWL_SHIM_EXPORT is_fullscreen();
bool NOOSKEWL_SHIM_EXPORT is_real_fullscreen();
bool NOOSKEWL_SHIM_EXPORT is_fullscreen_window();

void NOOSKEWL_SHIM_EXPORT resize_window(int width, int height);

void NOOSKEWL_SHIM_EXPORT load_fonts();
void NOOSKEWL_SHIM_EXPORT destroy_fonts();

} // End namespace gfx

} // End namespace noo

#endif // NOO_GFX_H
