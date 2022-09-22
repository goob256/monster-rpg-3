#ifndef NOO_IMAGE_BASE_H
#define NOO_IMAGE_BASE_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace gfx {

class Shader;

class NOOSKEWL_SHIM_EXPORT Image_Base {
public:
	friend class NOOSKEWL_SHIM_EXPORT Image;
	friend class NOOSKEWL_SHIM_EXPORT Shader;
	friend class NOOSKEWL_SHIM_EXPORT Vertex_Cache;

	enum Flags {
		FLIP_H = 1,
		FLIP_V = 2
	};

	static void static_start();
	static void release_all(bool include_managed = false);
	static void reload_all(bool include_managed = false);
	static int get_unfreed_count();
	static void audit();
	static unsigned char *read_tga(std::string filename, util::Size<int> &out_size, SDL_Colour *out_palette = 0, util::Point<int> *opaque_topleft = 0, util::Point<int> *opaque_bottomright = 0, bool *has_alpha = 0, bool load_from_filesystem = false);

	// These parameters affect newly created images
	static bool dumping_colours;
	static bool keep_data;
	static bool save_rle;
	static bool ignore_palette;
	static bool create_depth_buffer;
	static bool create_stencil_buffer;
	static bool premultiply_alpha;
	static bool save_rgba;
	static bool save_palettes;
	static bool linear_filter;

	std::string filename;
	util::Size<int> size;

	Image_Base(std::string filename, bool is_absolute_path = false, bool load_from_filesystem = false);
	Image_Base(Uint8 *data, util::Size<int> size, bool destroy_data = false);
	Image_Base(SDL_Surface *surface);
	Image_Base(util::Size<int> size);
	Image_Base(Image_Base *parent, util::Point<int> offset, util::Size<int> size);
	virtual ~Image_Base();

	void release();
	void reload(bool load_from_filesystem = false);

	bool save(std::string filename);

	// these are not normally to be used, use gfx::set_target_image and gfx::set_target_backbuffer instead
	void set_target();
	void release_target();

	void get_bounds(util::Point<int> &topleft, util::Point<int> &bottomright);
	void set_bounds(util::Point<int> topleft, util::Point<int> bottomright);

	void destroy_data();

	Image_Base *get_root();

	unsigned char *get_loaded_data();

protected:
	struct TGA_Header {
		char idlength;
		char colourmaptype;
		char datatypecode;
		short int colourmaporigin;
		short int colourmaplength;
		char colourmapdepth;
		short int x_origin;
		short int y_origin;
		short width;
		short height;
		char bitsperpixel;
		char imagedescriptor;
		SDL_Colour palette[256];
	};

	// returns true if pixel is transparent
	static bool merge_bytes(unsigned char *pixel, unsigned char *p, int bytes, TGA_Header *header, bool *alpha);

	unsigned char find_colour_in_palette(unsigned char *p);

	struct Internal {
		Internal(std::string filename, bool keep_data, bool support_render_to_texture = false, bool load_from_filesystem = false);
		Internal(unsigned char *pixels, util::Size<int> size, bool support_render_to_texture = false);
		Internal();
		~Internal();

		void upload(unsigned char *pixels);

		void release();
		unsigned char *reload(bool keep_data, bool load_from_filesystem = false);
		void unbind();

		void destroy_data();
		bool is_transparent(util::Point<int> position);

		unsigned char *loaded_data;

		std::string filename;
		util::Size<int> size;
		int refcount;

		bool has_render_to_texture;

#ifdef _WIN32
		LPDIRECT3DTEXTURE9 video_texture;
		LPDIRECT3DTEXTURE9 system_texture;
		LPDIRECT3DSURFACE9 render_target;
		LPDIRECT3DSURFACE9 depth_stencil_buffer;
#endif
		GLuint texture;
		GLuint fbo;
		GLuint depth_buffer;
		GLuint stencil_buffer;

		// For sub-images
		Image_Base *parent;
		util::Point<int> offset;

		// Position of outmost opaque pixels
		util::Point<int> opaque_topleft;
		util::Point<int> opaque_bottomright;

		bool create_depth_buffer;
		bool create_stencil_buffer;

		bool has_alpha;
	};

	static std::vector<Internal *> loaded_images;
	static int d3d_depth_buffer_count;
	static int d3d_video_texture_count;
	static int d3d_system_texture_count;
	static int d3d_surface_level_count;

	bool batching;

	Internal *internal;

	bool flipped;

	Uint8 *data_to_destroy;
};

} // End namespace gfx

} // End namespace noo

#endif // NOO_IMAGE_BASE_H
