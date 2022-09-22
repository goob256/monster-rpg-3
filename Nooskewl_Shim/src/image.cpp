#include "Nooskewl_Shim/image.h"

namespace noo {

namespace gfx {

Image::Image(std::string filename, bool is_absolute_path, bool load_from_filesystem) :
	Image_Base(filename, is_absolute_path, load_from_filesystem)
{
}

Image::Image(Uint8 *data, util::Size<int> size, bool destroy_data) :
	Image_Base(data, size, destroy_data)
{
}

Image::Image(SDL_Surface *surface) :
	Image_Base(surface)
{
}

Image::Image(util::Size<int> size) :
	Image_Base(size)
{
}

Image::Image(Image *parent, util::Point<int> offset, util::Size<int> size) :
	Image_Base(parent, offset, size)
{
}

Image::~Image()
{
}

void Image::start_batch(bool repeat)
{
	Image_Base *root = get_root();
	Vertex_Cache::instance()->start(root, repeat);
	root->batching = true;
}

void Image::end_batch()
{
	Vertex_Cache::instance()->end();
	get_root()->batching = false;
}

void Image::stretch_region_tinted_repeat(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, util::Size<int> dest_size, int flags)
{
	SDL_Colour colours[4];
	colours[0] = colours[1] = colours[2] = colours[3] = tint;

	int wt = dest_size.w / source_size.w;
	if (dest_size.w % source_size.w != 0) {
		wt++;
	}
	int ht = dest_size.h / source_size.h;
	if (dest_size.h % source_size.h != 0) {
		ht++;
	}

	bool was_batching = get_root()->batching;
	if (was_batching == false) start_batch();

	int drawn_h = 0;
	for (int y = 0; y < ht; y++) {
		int drawn_w = 0;
		int h = source_size.h;
		if (dest_size.h - drawn_h < h) {
			h = dest_size.h- drawn_h;
		}
		for (int x = 0; x < wt; x++) {
			int w = source_size.w;
			if (dest_size.w - drawn_w < w) {
				w = dest_size.w - drawn_w;
			}
			util::Size<int> sz(w, h);
			Vertex_Cache::instance()->cache(colours, source_position+internal->offset, sz, util::Point<float>(dest_position.x + x * source_size.w, dest_position.y + y * source_size.h), sz, flags);
			drawn_w += w;
		}
		drawn_h += h;
	}

	if (was_batching == false) end_batch();
}

void Image::stretch_region_tinted(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, util::Size<int> dest_size, int flags)
{
	SDL_Colour colours[4];
	colours[0] = colours[1] = colours[2] = colours[3] = tint;
	bool was_batching = get_root()->batching;
	if (was_batching == false) start_batch();
	Vertex_Cache::instance()->cache(colours, source_position+internal->offset, source_size, dest_position, dest_size, flags);
	if (was_batching == false) end_batch();
}

void Image::stretch_region(util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, util::Size<int> dest_size, int flags)
{
	stretch_region_tinted(shim::white, source_position, source_size, dest_position, dest_size, flags);
}

void Image::draw_region_lit_z_range(SDL_Colour colours[4], util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z_top, float z_bottom, int flags)
{
	bool was_batching = get_root()->batching;
	if (was_batching == false) start_batch();
	Vertex_Cache::instance()->cache_z_range(colours, source_position+internal->offset, source_size, dest_position, z_top, z_bottom, source_size, flags);
	if (was_batching == false) end_batch();
}

void Image::draw_region_lit_z(SDL_Colour colours[4], util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z, int flags)
{
	draw_region_lit_z_range(colours, source_position, source_size, dest_position, z, z, flags);
}

void Image::draw_region_tinted_z_range(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z_top, float z_bottom, int flags)
{
	SDL_Colour colours[4];
	colours[0] = colours[1] = colours[2] = colours[3] = tint;
	draw_region_lit_z_range(colours, source_position, source_size, dest_position, z_top, z_bottom, flags);
}

void Image::draw_region_tinted_z(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z, int flags)
{
	draw_region_tinted_z_range(tint, source_position, source_size, dest_position, z, z, flags);
}

void Image::draw_region_tinted(SDL_Colour tint, util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, int flags)
{
	SDL_Colour colours[4];
	colours[0] = colours[1] = colours[2] = colours[3] = tint;
	draw_region_lit_z_range(colours, source_position, source_size, dest_position, 0.0f, 0.0f, flags);
}

void Image::draw_region_z_range(util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z_top, float z_bottom, int flags)
{
	SDL_Colour colours[4];
	colours[0] = colours[1] = colours[2] = colours[3] = shim::white;
	draw_region_lit_z_range(colours, source_position, source_size, dest_position, z_top, z_bottom, flags);
}

void Image::draw_region_z(util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, float z, int flags)
{
	draw_region_tinted_z(shim::white, source_position, source_size, dest_position, z, flags);
}

void Image::draw_region(util::Point<float> source_position, util::Size<int> source_size, util::Point<float> dest_position, int flags)
{
	draw_region_z(source_position, source_size, dest_position, 0.0f, flags);
}

void Image::draw_z(util::Point<float> dest_position, float z, int flags)
{
	draw_region_z(util::Point<float>(0.0f, 0.0f), size, dest_position, z, flags);
}

void Image::draw_tinted(SDL_Colour tint, util::Point<float> dest_position, int flags)
{
	draw_region_tinted(tint, util::Point<float>(0.0f, 0.0f), size, dest_position, flags);
}

void Image::draw(util::Point<float> dest_position, int flags)
{
	draw_z(dest_position, 0.0f, flags);
}

void Image::draw_tinted_rotated(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, int flags)
{
	SDL_Colour colours[4];
	colours[0] = colours[1] = colours[2] = colours[3] = tint;
	bool was_batching = get_root()->batching;
	if (was_batching == false) start_batch();
	Vertex_Cache::instance()->cache(colours, centre, internal->offset, size, dest_position, angle, 1.0f, flags);
	if (was_batching == false) end_batch();
}

void Image::draw_tinted_rotated_scaledxy_z(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, float scale_x, float scale_y, float z, int flags)
{
	SDL_Colour colours[4];
	colours[0] = colours[1] = colours[2] = colours[3] = tint;
	bool was_batching = get_root()->batching;
	if (was_batching == false) start_batch();
	Vertex_Cache::instance()->cache_z(colours, centre, internal->offset, size, dest_position, angle, scale_x, scale_y, z, flags);
	if (was_batching == false) end_batch();
}

void Image::draw_tinted_rotated_scaled_z(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, float scale, float z, int flags)
{
	draw_tinted_rotated_scaledxy_z(tint, centre, dest_position, angle, scale, scale, z, flags);
}

void Image::draw_rotated_scaled_z(util::Point<float> centre, util::Point<float> dest_position, float angle, float scale, float z, int flags)
{
	draw_tinted_rotated_scaledxy_z(shim::white, centre, dest_position, angle, scale, scale, z, flags);
}

void Image::draw_tinted_rotated_scaled(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, float scale, int flags)
{
	draw_tinted_rotated_scaled_z(tint, centre, dest_position, angle, scale, 0.0f, flags);
}

void Image::draw_tinted_rotated_scaledxy(SDL_Colour tint, util::Point<float> centre, util::Point<float> dest_position, float angle, float scale_x, float scale_y, int flags)
{
	draw_tinted_rotated_scaledxy_z(tint, centre, dest_position, angle, scale_x, scale_y, 0.0f, flags);
}

void Image::draw_rotated(util::Point<float> centre, util::Point<float> dest_position, float angle, int flags)
{
	draw_tinted_rotated(shim::white, centre, dest_position, angle, flags);
}

void Image::draw_rotated_scaled(util::Point<float> centre, util::Point<float> dest_position, float angle, float scale, int flags)
{
	draw_tinted_rotated_scaled(shim::white, centre, dest_position, angle, scale, flags);
}

} // End namespace gfx

} // End namespace noo
