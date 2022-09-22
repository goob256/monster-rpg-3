#ifndef NOO_IMAGE_H
#define NOO_IMAGE_H

#include "Nooskewl_Shim/main.h"
#include "Nooskewl_Shim/image_base.h"
#include "Nooskewl_Shim/vertex_cache.h"

namespace noo {

namespace gfx {

class NOOSKEWL_SHIM_EXPORT Image : public Image_Base
{
public:
	Image(std::string filename, bool is_absolute_path = false, bool load_from_filesystem = false);
	Image(Uint8 *data, util::Size<int> size, bool destroy_data = false);
	Image(SDL_Surface *surface);
	Image(util::Size<int> size);
	Image(Image *parent, util::Point<int> offset, util::Size<int> size);
	virtual ~Image();

	void start_batch(bool repeat = false);
	void end_batch();

	void stretch_region_tinted_repeat(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, util::Size<int> dest_size, int flags = 0);
	void stretch_region_tinted(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, util::Size<int> dest_size, int flags = 0);
	void stretch_region(util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, util::Size<int> dest_size, int flags = 0);
	void draw_region_lit_z_range(SDL_Colour colours[4], util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z_top, float z_bottom, int flags = 0);
	void draw_region_lit_z(SDL_Colour colours[4], util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z, int flags = 0);
	void draw_region_tinted_z_range(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z_top, float z_bottom, int flags = 0);
	void draw_region_tinted_z(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z, int flags = 0);
	void draw_region_tinted(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, int flags = 0);
	void draw_region_z_range(util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z_top, float z_bottom, int flags = 0);
	void draw_region_z(util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z, int flags = 0);
	void draw_region(util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, int flags = 0);
	void draw_z(util::Point<float> dest_position, float z, int flags = 0);
	void draw_tinted(SDL_Colour tint, util::Point<float> dest_position, int flags = 0);
	void draw(util::Point<float> dest_position, int flags = 0);
	void draw_tinted_rotated(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, int flags = 0);
	void draw_tinted_rotated_scaledxy_z(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, float scale_x, float scale_y, float z, int flags = 0);
	void draw_tinted_rotated_scaled_z(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, float scale, float z, int flags = 0);
	void draw_rotated_scaled_z(util::Point<float> centre, util::Point<float> dest_position, float angle, float scale, float z, int flags = 0);
	void draw_tinted_rotated_scaled(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, float scale, int flags = 0);
	void draw_tinted_rotated_scaledxy(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, float scale_x, float scale_y, int flags = 0);
	void draw_rotated(util::Point<float> centre, util::Point<float> dest_position, float angle, int flags = 0);
	void draw_rotated_scaled(util::Point<float> centre, util::Point<float> dest_position, float angle, float scale, int flags = 0);
};

} // End namespace gfx

} // End namespace noo

#endif // NOO_IMAGE_H
