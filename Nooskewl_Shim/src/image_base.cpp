// TGA loader taken from http://paulbourke.net/dataformats/tga/

#include "Nooskewl_Shim/error.h"
#include "Nooskewl_Shim/gfx.h"
#include "Nooskewl_Shim/image_base.h"
#include "Nooskewl_Shim/shader.h"
#include "Nooskewl_Shim/shim.h"
#include "Nooskewl_Shim/util.h"

#include "Nooskewl_Shim/internal/gfx.h"

#if defined ANDROID || defined IOS
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#endif

using namespace noo;

static inline unsigned char *pixel_ptr(unsigned char *p, int n, bool flip, int w, int h)
{
	if (flip) {
		int x = n % w;
		int y = n / w;
		return p + (w * 4) * (h-1) - (y * w * 4) +  x * 4;
	}
	else {
		return p + n * 4;
	}
}

static inline void pixel_xy(int n, int w, int *x, int *y)
{
	*x = n % w;
	*y = n / w;
}

static inline void maybe_resize_bounds(util::Point<int> &opaque_topleft, util::Point<int> &opaque_bottomright, int x, int y)
{
	if (opaque_topleft.x == -1 || x < opaque_topleft.x) {
		opaque_topleft.x = x;
	}
	if (opaque_topleft.y == -1 || y < opaque_topleft.y) {
		opaque_topleft.y = y;
	}
	if (opaque_bottomright.x == -1 || x > opaque_bottomright.x) {
		opaque_bottomright.x = x;
	}
	if (opaque_bottomright.y == -1 || y > opaque_bottomright.y) {
		opaque_bottomright.y = y;
	}
}

namespace noo {

namespace gfx {

std::vector<Image_Base::Internal *> Image_Base::loaded_images;

static GLuint bound_fbo;
#ifdef _WIN32
static LPDIRECT3DSURFACE9 bound_depth_buffer;
static bool depth_buffer_bound;
#endif
bool Image_Base::dumping_colours;
bool Image_Base::keep_data;
bool Image_Base::save_rle;
bool Image_Base::ignore_palette;
bool Image_Base::create_depth_buffer;
bool Image_Base::create_stencil_buffer;
int Image_Base::d3d_depth_buffer_count;
int Image_Base::d3d_video_texture_count;
int Image_Base::d3d_system_texture_count;
int Image_Base::d3d_surface_level_count;
bool Image_Base::premultiply_alpha;
bool Image_Base::save_rgba;
bool Image_Base::save_palettes;
bool Image_Base::linear_filter;

void Image_Base::static_start()
{
	bound_fbo = 0;
#ifdef _WIN32
	bound_depth_buffer = 0;
	depth_buffer_bound = false;
#endif
	dumping_colours = false;
	keep_data = false;
	save_rle = false;
	ignore_palette = false;
	create_depth_buffer = false;
	create_stencil_buffer = false;
	premultiply_alpha = true;
	save_rgba = false;
	save_palettes = true;
	linear_filter = false;
	d3d_depth_buffer_count = 0;
	d3d_video_texture_count = 0;
	d3d_system_texture_count = 0;
	d3d_surface_level_count = 0;
}

void Image_Base::release_all(bool include_managed)
{
	util::infomsg("Releasing %d textures...\n", loaded_images.size());
	for (size_t i = 0; i < loaded_images.size(); i++) {
#ifdef _WIN32
		if (shim::opengl == true || (include_managed && loaded_images[i]->has_render_to_texture == false)) {
#endif
			loaded_images[i]->release();
#ifdef _WIN32
		}
		else {
			loaded_images[i]->unbind();
		}
#endif
	}
}

void Image_Base::reload_all(bool include_managed)
{
	for (size_t i = 0; i < loaded_images.size(); i++) {
#ifdef _WIN32
		if (shim::opengl == true || (include_managed && loaded_images[i]->has_render_to_texture == false)) {
#endif
			loaded_images[i]->reload(false);
#ifdef _WIN32
		}
#endif
	}
}

int Image_Base::get_unfreed_count()
{
	for (size_t i = 0; i < loaded_images.size(); i++) {
		util::infomsg("Unfreed: %s.\n", loaded_images[i]->filename.c_str());
	}
	return (int)loaded_images.size();
}

void Image_Base::audit()
{
	util::debugmsg("d3d_depth_buffer_count=%d\n", d3d_depth_buffer_count);
	util::debugmsg("d3d_video_texture_count=%d\n", d3d_video_texture_count);
	util::debugmsg("d3d_system_texture_count=%d\n", d3d_system_texture_count);
	util::debugmsg("d3d_surface_level_count=%d\n", d3d_surface_level_count);
}

unsigned char *Image_Base::read_tga(std::string filename, util::Size<int> &out_size, SDL_Colour *out_palette, util::Point<int> *opaque_topleft, util::Point<int> *opaque_bottomright, bool *has_alpha, bool load_from_filesystem)
{
	SDL_RWops *file;
	if (load_from_filesystem) {
		file = SDL_RWFromFile(filename.c_str(), "rb");
	}
	else {
		file = util::open_file(filename, 0);
	}

	if (file == 0) {
		return 0;
	}

	int n = 0, i = 0, j;
	int bytes2read;
	unsigned char p[5];
	TGA_Header header;
	unsigned char *pixels;
	int pixel_x, pixel_y;
	bool alpha;

	/* Display the header fields */
	header.idlength = util::SDL_fgetc(file);
	header.colourmaptype = util::SDL_fgetc(file);
	header.datatypecode = util::SDL_fgetc(file);
	header.colourmaporigin = SDL_ReadLE16(file);
	header.colourmaplength = SDL_ReadLE16(file);
	header.colourmapdepth = util::SDL_fgetc(file);
	header.x_origin = SDL_ReadLE16(file);
	header.y_origin = SDL_ReadLE16(file);
	header.width = SDL_ReadLE16(file);
	header.height = SDL_ReadLE16(file);
	header.bitsperpixel = util::SDL_fgetc(file);
	header.imagedescriptor = util::SDL_fgetc(file);

	int w, h;
	out_size.w = w = header.width;
	out_size.h = h = header.height;

	if (has_alpha) {
		*has_alpha = false;
	}

	try {
		/* Allocate space for the image */
		if ((pixels = new unsigned char[header.width*header.height*4]) == 0) {
			throw util::MemoryError("malloc of image failed");
		}

		/* What can we handle */
		if (header.datatypecode != 1 && header.datatypecode != 2 && header.datatypecode != 9 && header.datatypecode != 10) {
			throw util::LoadError("can only handle image type 1, 2, 9 and 10");
		}
		if (header.bitsperpixel != 8 && header.bitsperpixel != 16 && header.bitsperpixel != 24 && header.bitsperpixel != 32) {
			throw util::LoadError("can only handle pixel depths of 8, 16, 24 and 32");
		}
		if (header.colourmaptype != 0 && header.colourmaptype != 1) {
			throw util::LoadError("can only handle colour map types of 0 and 1");
		}

		/* Skip over unnecessary stuff */
		SDL_RWseek(file, header.idlength, RW_SEEK_CUR);

		/* Read the palette if there is one */
		if (header.colourmaptype == 1) {
			if (header.colourmapdepth != 24) {
				throw util::LoadError("can't handle anything but 24 bit palettes");
			}
			if (header.bitsperpixel != 8) {
				throw util::LoadError("can only read 8 bpp paletted images");
			}
			int skip = header.colourmaporigin * (header.colourmapdepth / 8);
			SDL_RWseek(file, skip, RW_SEEK_CUR);
			// We can only read 256 colour palettes max, skip the rest
			int size = MIN(header.colourmaplength-skip, 256);
			skip = (header.colourmaplength - size) * (header.colourmapdepth / 8);
			for (i = 0; i < size; i++) {
				header.palette[i].b = util::SDL_fgetc(file);
				header.palette[i].g = util::SDL_fgetc(file);
				header.palette[i].r = util::SDL_fgetc(file);
			}
			SDL_RWseek(file, skip, RW_SEEK_CUR);
		}
		else {
			// Skip the palette on truecolour images
			SDL_RWseek(file, (header.colourmapdepth / 8) * header.colourmaplength, RW_SEEK_CUR);
		}

		bool flip = (header.imagedescriptor & 0x20) != 0;

		/* Read the image */
		bytes2read = header.bitsperpixel / 8;
		while (n < header.width * header.height) {
			if (header.datatypecode == 1 || header.datatypecode == 2) {                     /* Uncompressed */
				if (SDL_RWread(file, p, 1, bytes2read) != (size_t)bytes2read) {
					delete[] pixels;
					throw util::LoadError("unexpected end of file at pixel " + util::itos(i));
				}
				if (merge_bytes(pixel_ptr(pixels, n, flip, w, h), p, bytes2read, &header, &alpha) == false) {
					if (opaque_topleft != 0 && opaque_bottomright != 0) {
						pixel_xy(n, w, &pixel_x, &pixel_y);
						maybe_resize_bounds(*opaque_topleft, *opaque_bottomright, pixel_x, pixel_y);
					}
				}
				n++;
				if (alpha && has_alpha) {
					*has_alpha = true;
				}
			}
			else if (header.datatypecode == 9 || header.datatypecode == 10) {             /* Compressed */
				if (SDL_RWread(file, p, 1, bytes2read+1) != (size_t)bytes2read+1) {
					delete[] pixels;
					throw util::LoadError("unexpected end of file at pixel " + util::itos(i));
				}
				j = p[0] & 0x7f;
				if (merge_bytes(pixel_ptr(pixels, n, flip, w, h), &(p[1]), bytes2read, &header, &alpha) == false) {
					if (opaque_topleft != 0 && opaque_bottomright != 0) {
						pixel_xy(n, w, &pixel_x, &pixel_y);
						maybe_resize_bounds(*opaque_topleft, *opaque_bottomright, pixel_x, pixel_y);
					}
				}
				n++;
				if (alpha && has_alpha) {
					*has_alpha = true;
				}
				if (p[0] & 0x80) {         /* RLE chunk */
					for (i = 0; i < j; i++) {
						if (merge_bytes(pixel_ptr(pixels, n, flip, w, h), &(p[1]), bytes2read, &header, &alpha) == false) {
							if (opaque_topleft != 0 && opaque_bottomright != 0) {
								pixel_xy(n, w, &pixel_x, &pixel_y);
								maybe_resize_bounds(*opaque_topleft, *opaque_bottomright, pixel_x, pixel_y);
							}
						}
						n++;
						if (alpha && has_alpha) {
							*has_alpha = true;
						}
					}
				}
				else {                   /* Normal chunk */
					for (i = 0; i < j; i++) {
						if (SDL_RWread(file, p, 1, bytes2read) != (size_t)bytes2read) {
							delete[] pixels;
							throw util::LoadError("unexpected end of file at pixel " + util::itos(i));
						}
						if (merge_bytes(pixel_ptr(pixels, n, flip, w, h), p, bytes2read, &header, &alpha) == false) {
							if (opaque_topleft != 0 && opaque_bottomright != 0) {
								pixel_xy(n, w, &pixel_x, &pixel_y);
								maybe_resize_bounds(*opaque_topleft, *opaque_bottomright, pixel_x, pixel_y);
							}
						}
						n++;
						if (alpha && has_alpha) {
							*has_alpha = true;
						}
					}
				}
			}
		}
	}
	catch (util::Error &) {
		util::close_file(file);
		throw;
	}

	util::close_file(file);

	if (out_palette != 0) {
		memcpy(out_palette, header.palette, 256 * 3);
	}

	return pixels;
}

bool Image_Base::merge_bytes(unsigned char *pixel, unsigned char *p, int bytes, TGA_Header *header, bool *alpha)
{
	if (header->colourmaptype == 1) {
		SDL_Colour *colour;
		if (ignore_palette) {
			colour = &shim::palette[*p];
		}
		else {
			colour = &header->palette[*p];
		}
		// Magic pink
		// Paletted
		if (colour->r == 255 && colour->g == 0 && colour->b == 255) {
			// transparent
			*pixel++ = 0;
			*pixel++ = 0;
			*pixel++ = 0;
			*pixel++ = 0;
			return true;
		}
		else {
			*pixel++ = colour->r;
			*pixel++ = colour->g;
			*pixel++ = colour->b;
			*pixel++ = 255;
		}
		*alpha = false;
	}
	else {
		if (bytes == 4) {
			if (premultiply_alpha) {
				float a = p[3] / 255.0f;
				*pixel++ = (unsigned char)(p[2] * a);
				*pixel++ = (unsigned char)(p[1] * a);
				*pixel++ = (unsigned char)(p[0] * a);
			}
			else {
				*pixel++ = p[2];
				*pixel++ = p[1];
				*pixel++ = p[0];
			}
			*pixel++ = p[3];
			if (p[3] != 255 && p[3] != 0) {
				*alpha = true;
			}
			else {
				*alpha = false;
			}
			return p[3] == 0;
		}
		else if (bytes == 3) {
			*pixel++ = p[2];
			*pixel++ = p[1];
			*pixel++ = p[0];
			*pixel++ = 255;
			*alpha = false;
		}
		else if (bytes == 2) {
			*pixel++ = (p[1] & 0x7c) << 1;
			*pixel++ = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
			*pixel++ = (p[0] & 0x1f) << 3;
			*pixel++ = (p[1] & 0x80);
			*alpha = false;
			return (p[1] & 0x80) == 0;
		}
	}

	return false;
}

Image_Base::Image_Base(std::string filename, bool is_absolute_path, bool load_from_filesystem) :
	batching(false),
	flipped(true),
	data_to_destroy(0)
{
	if (is_absolute_path == false && load_from_filesystem == false) {
		filename = "gfx/images/" + filename;
	}

	this->filename = filename;

	reload(load_from_filesystem);
}

Image_Base::Image_Base(Uint8 *data, util::Size<int> size, bool destroy_data) :
	filename("--NOT LOADED--"),
	size(size),
	batching(false),
	flipped(true)
{
	if (destroy_data) {
		data_to_destroy = data;
	}
	else {
		data_to_destroy = 0;
	}

	try {
		internal = new Internal(data, size);
	}
	catch (util::Error &) {
		delete[] data;
		throw;
	}

	internal->has_alpha = false;
	Uint8 *p = data;
	for (int i = 0; i < size.w * size.h; i++) {
		if (p[3] != 255) {
			internal->has_alpha = true;
			break;
		}
		p += 4;
	}
}

Image_Base::Image_Base(SDL_Surface *surface) :
	filename("--NOT LOADED--"),
	batching(false),
	flipped(true),
	data_to_destroy(0)
{
	unsigned char *pixels;
	unsigned char *packed = 0;
	SDL_Surface *tmp = 0;
	SDL_Surface *fetch;

	if (surface->format->format == SDL_PIXELFORMAT_RGBA8888) {
		fetch = surface;
	}
	else {
		SDL_PixelFormat format;
		format.format = SDL_PIXELFORMAT_RGBA8888;
		format.palette = 0;
		format.BitsPerPixel = 32;
		format.BytesPerPixel = 4;
		format.Rmask = 0xff;
		format.Gmask = 0xff00;
		format.Bmask = 0xff0000;
		format.Amask = 0xff000000;
		tmp = SDL_ConvertSurface(surface, &format, 0);
		if (tmp == 0) {
			throw util::Error("SDL_ConvertSurface returned 0");
		}
		fetch = tmp;
	}

	if (fetch->pitch != fetch->w * 4) {
		// must be packed
		packed = new unsigned char[fetch->w * 4 * fetch->h];
		memset(packed, 0, fetch->w*4*fetch->h);
		for (int y = 0; y < fetch->h; y++) {
			memcpy(packed + (y * fetch->w * 4), ((unsigned char *)fetch->pixels) + y * fetch->pitch, fetch->w * 4);
		}
		pixels = packed;
	}
	else {
		pixels = (unsigned char *)fetch->pixels;
	}

	/*
	// FIXME:
	for (int y = 0; y < fetch->h; y++) {
		for (int x = 0; x < fetch->w; x++) {
			Uint8 *p = ((Uint8 *)fetch->pixels) + y * fetch->pitch + x * 4;
			printf("%02x", p[3]);
		}
		printf("\n");
	}
	*/

	size = util::Size<int>(surface->w, surface->h);

	try {
		internal = new Internal(pixels, size);
	}
	catch (util::Error &) {
		if (tmp) SDL_FreeSurface(tmp);
		throw;
	}

	internal->has_alpha = false;
	Uint8 *p = pixels;
	for (int i = 0; i < size.w * size.h; i++) {
		if (p[3] != 255) {
			internal->has_alpha = true;
			break;
		}
		p += 4;
	}

	if (tmp) {
		SDL_FreeSurface(tmp);
	}

	if (packed) {
		delete[] packed;
	}
}

Image_Base::Image_Base(util::Size<int> size) :
	filename("--NOT LOADED--"),
	size(size),
	batching(false),
	flipped(false),
	data_to_destroy(0)
{
	unsigned char *pixels = (unsigned char *)calloc(1, size.w * size.h * 4);

	try {
		internal = new Internal(pixels, size, true); // support render to texture
	}
	catch (util::Error &) {
		free(pixels);
		throw;
	}

	internal->has_alpha = true;

	free(pixels);
}

Image_Base::Image_Base(Image_Base *parent, util::Point<int> offset, util::Size<int> size) :
	filename("--NOT LOADED--"),
	size(size),
	batching(false),
	flipped(true),
	data_to_destroy(0)
{
	internal = new Internal;
	internal->parent = parent;
	internal->offset = offset;

	internal->has_alpha = parent->internal->has_alpha; // FIXME: not always true

	Image_Base::Internal *parent_internal = parent->internal;

	for (int y = 0; y < size.h; y++) {
		for (int x = 0; x < size.w; x++) {
			if (parent_internal->is_transparent(util::Point<int>(x, y) + offset) == false) {
				maybe_resize_bounds(internal->opaque_topleft, internal->opaque_bottomright, x, y);
			}
		}
	}
}

Image_Base::~Image_Base()
{
	release();
	delete data_to_destroy;
}

void Image_Base::release()
{
	if (filename == "--NOT LOADED--") {
		internal->unbind();
		delete internal;
		internal = 0;
		return;
	}

	for (size_t i = 0; i < loaded_images.size(); i++) {
		Internal *ii = loaded_images[i];
		if (ii->filename == filename) {
			ii->refcount--;
			if (ii->refcount == 0) {
				internal->unbind();
				loaded_images.erase(loaded_images.begin()+i);
				delete ii;
				return;
			}
		}
	}
}

void Image_Base::reload(bool load_from_filesystem)
{
	if (filename == "--NOT LOADED--") {
		return;
	}

	for (size_t i = 0; i < loaded_images.size(); i++) {
		Internal *ii = loaded_images[i];
		if (ii->filename == filename) {
			ii->refcount++;
			internal = ii;
			size = internal->size;
			return;
		}
	}

	internal = new Internal(filename, keep_data, false, load_from_filesystem);
	size = internal->size;
	loaded_images.push_back(internal);
}

bool Image_Base::save(std::string filename)
{
	bool _save_rgba = internal->has_alpha || save_rgba; // FIXME: should be able to force NOT saving RGBA

	unsigned char *loaded_data = internal->loaded_data;

	unsigned char header[] = {
		(unsigned char)0x00, // idlength
		(unsigned char)0x01, // colourmap type 1 == palette
		(unsigned char)(save_rle ? 0x09 : 0x01),
		(unsigned char)0x00, (unsigned char)0x00, // colourmap origin (little endian)
		(unsigned char)0x00, save_palettes ? (unsigned char)0x01 : (unsigned char)0x00, // # of palette entries
		(unsigned char)0x18, // colourmap depth
		(unsigned char)0x00, (unsigned char)0x00, // x origin
		(unsigned char)0x00, (unsigned char)0x00, // y origin
		(unsigned char)(size.w & 0xff), (unsigned char)((size.w >> 8) & 0xff), // width
		(unsigned char)(size.h & 0xff), (unsigned char)((size.h >> 8) & 0xff), // height
		(unsigned char)0x08, // bits per pixel
		(unsigned char)0x00 // image descriptor
	};

	if (_save_rgba) {
		header[1] = 0;
		header[2] = 2;
		header[5] = 0;
		header[6] = 0;
		header[7] = 0;
		header[16] = 32;
		header[17] = 8;
	}

	int header_size = 18;

	SDL_RWops *file = SDL_RWFromFile(filename.c_str(), "wb");
	if (file == 0) {
		throw util::Error("Couldn't open " + filename + " for writing");
	}

	for (int i = 0; i < header_size; i++) {
		if (util::SDL_fputc(header[i], file) == EOF) {
			throw util::Error("Write error writing to " + filename);
		}
	}

	if (_save_rgba == false) {
		if (save_palettes) {
			for (int i = 0; i < 256; i++) {
				if (util::SDL_fputc(shim::palette[i].b, file) == EOF) {
					throw util::Error("Write error writing to " + filename);
				}
				if (util::SDL_fputc(shim::palette[i].g, file) == EOF) {
					throw util::Error("Write error writing to " + filename);
				}
				if (util::SDL_fputc(shim::palette[i].r, file) == EOF) {
					throw util::Error("Write error writing to " + filename);
				}
			}
		}

		#define R(n) *(pixel_ptr(loaded_data, n, false, size.w, size.h)+0)
		#define G(n) *(pixel_ptr(loaded_data, n, false, size.w, size.h)+1)
		#define B(n) *(pixel_ptr(loaded_data, n, false, size.w, size.h)+2)

		if (save_rle) {
			for (int i = 0; i < size.w * size.h;) {
				int j, count;
				int next_line = i - (i % size.w) + size.w - 1;
				for (j = i, count = 0; j < size.w * size.h - 1 && j < next_line && count < 127; j++, count++) {
					if (R(j) != R(j+1) || G(j) != G(j+1) || B(j) != B(j+1)) {
						break;
					}
				}
				int run_length = j - i + 1;
				if (run_length > 1) {
					util::SDL_fputc((run_length-1) | 0x80, file);
					util::SDL_fputc(find_colour_in_palette(&R(i)), file);
				}
				else {
					for (j = i, count = 0; j < size.w * size.h - 1 && j < next_line && count < 127; j++, count++) {
						if (R(j) == R(j+1) && G(j) == G(j+1) && B(j) == B(j+1)) {
							break;
						}
					}
					run_length = j - i + 1;
					// I noticed PSP never stores a non-run of 2 pixels, and this saves some space usually, so we do the same
					if (run_length == 2) {
						run_length--;
					}
					util::SDL_fputc((run_length-1), file);
					util::SDL_fputc(find_colour_in_palette(&R(i)), file);
					for (j = 0; j < run_length-1; j++) {
						util::SDL_fputc(find_colour_in_palette(&R(i+j+1)), file);
					}
				}
				i += run_length;
			}
		}
		else {
			for (int i = 0; i < size.w * size.h; i++) {
				util::SDL_fputc(find_colour_in_palette(&R(i)), file);
			}
		}
	}
	else {
		unsigned char *tmp = new unsigned char[size.w * size.h * 4];
		unsigned char *p = tmp;
		unsigned char *p2 = loaded_data;
		for (int i = 0; i < size.w * size.h; i++) {
			unsigned char r = *p2++;
			unsigned char g = *p2++;
			unsigned char b = *p2++;
			unsigned char a = *p2++;
			*p++ = b;
			*p++ = g;
			*p++ = r;
			*p++ = a;
		}
		SDL_RWwrite(file, tmp, size.w * size.h * 4, 1);
		delete[] tmp;
	}

	return true;
}

unsigned char Image_Base::find_colour_in_palette(unsigned char *p)
{
	if (p[3] == 0) {
		return 0;
	}

	for (unsigned int i = 0; i < 256; i++) {
		if (p[0] == shim::palette[i].r && p[1] == shim::palette[i].g && p[2] == shim::palette[i].b) {
			return i;
		}
	}

	util::errormsg("Error: colour %d,%d,%d not found!\n", p[0], p[1], p[2]);

	return 0;
}

void Image_Base::set_target()
{
	if (shim::opengl) {
		bound_fbo = internal->fbo;
		glBindFramebuffer_ptr(GL_FRAMEBUFFER, internal->fbo);
		glViewport_ptr(0, 0, size.w, size.h);
		glDisable_ptr(GL_SCISSOR_TEST);
	}
#ifdef _WIN32
	else {
		shim::d3d_device->EndScene();

		util::verbosemsg("device render_target->Release=%d\n", internal::gfx_context.render_target->Release());

		if (internal->video_texture->GetSurfaceLevel(0, &internal->render_target) != D3D_OK) {
			util::infomsg("Image_Base::set_target: Unable to get texture surface level\n");
			return;
		}
		d3d_surface_level_count++;

		if (shim::d3d_device->SetRenderTarget(0, internal->render_target) != D3D_OK) {
			util::infomsg("Image_Base::set_target: Unable to set render target to texture surface\n");
			util::verbosemsg("Image_Base::set_target (failure), render_target->Release=%d\n", internal->render_target->Release());
			return;
		}

		if (internal->depth_stencil_buffer) {
			shim::d3d_device->SetDepthStencilSurface(internal->depth_stencil_buffer);
			bound_depth_buffer = internal->depth_stencil_buffer;
		}
		else {
			shim::d3d_device->SetDepthStencilSurface(0);
			bound_depth_buffer = 0;
		}
		depth_buffer_bound = true;

		D3DVIEWPORT9 viewport;
		viewport.MinZ = 0;
		viewport.MaxZ = 1;
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = size.w;
		viewport.Height = size.h;
		shim::d3d_device->SetViewport(&viewport);

		shim::d3d_device->BeginScene();

		shim::d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}
#endif

	// Set an ortho projection the size of the image
	if (this == (Image_Base *)internal::gfx_context.work_image) {
		// mimic the real backbuffer
		set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
		update_projection();
	}
	else {
		glm::mat4 modelview = glm::mat4();
		glm::mat4 proj = glm::ortho(0.0f, (float)size.w, (float)size.h, 0.0f);
		set_matrices(modelview, proj);
		update_projection();
	}
}

void Image_Base::release_target()
{
	if (shim::opengl) {
		bound_fbo = 0;
#ifdef IOS
		//glBindRenderbuffer_ptr(GL_RENDERBUFFER, internal::gfx_context.colorbuffer); // don't know if this is needed
		glBindFramebuffer_ptr(GL_FRAMEBUFFER, internal::gfx_context.framebuffer);
#else
		glBindFramebuffer_ptr(GL_FRAMEBUFFER, 0);
#endif
	}
#ifdef _WIN32
	else {
		shim::d3d_device->EndScene();

		if (internal->render_target != 0) {
			util::verbosemsg("release_target (%p), render_target->Release=%d\n", this, internal->render_target->Release());
			internal->render_target = 0;
			d3d_surface_level_count--;
		}

		if (shim::d3d_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &internal::gfx_context.render_target) != D3D_OK) {
			util::infomsg("GetBackBuffer failed.\n");
		}

		if (shim::d3d_device->SetRenderTarget(0, internal::gfx_context.render_target) != D3D_OK) {
			util::infomsg("Image_Base::release_target: Unable to set render target to backbuffer.\n");
		}

		if (internal::gfx_context.depth_stencil_buffer) {
			shim::d3d_device->SetDepthStencilSurface(internal::gfx_context.depth_stencil_buffer);
			bound_depth_buffer = internal::gfx_context.depth_stencil_buffer;
			depth_buffer_bound = true;
		}

		shim::d3d_device->BeginScene();
	}
#endif

	set_screen_size(shim::real_screen_size); // this sets the viewport and scissor, updates projection
}

void Image_Base::get_bounds(util::Point<int> &topleft, util::Point<int> &bottomright)
{
	topleft = internal->opaque_topleft;
	bottomright = internal->opaque_bottomright;
}

void Image_Base::set_bounds(util::Point<int> topleft, util::Point<int> bottomright)
{
	internal->opaque_topleft = topleft;
	internal->opaque_bottomright = bottomright;
}

void Image_Base::destroy_data()
{
	if (internal != 0) {
		internal->destroy_data();
	}
}

Image_Base *Image_Base::get_root()
{
	gfx::Image_Base *root = this;
	while (root && root->internal->parent != 0) {
		root = root->internal->parent;
	}
	return root;
}

unsigned char *Image_Base::get_loaded_data()
{
	return internal->loaded_data;
}

//--

Image_Base::Internal::Internal(std::string filename, bool keep_data, bool support_render_to_texture, bool load_from_filesystem) :
	loaded_data(0),
	filename(filename),
	refcount(1),
	has_render_to_texture(support_render_to_texture),
#ifdef _WIN32
	video_texture(0),
	render_target(0),
#endif
	texture(0),
	depth_buffer(0),
	stencil_buffer(0),
	parent(0),
	offset(0, 0),
	opaque_topleft(-1, -1),
	opaque_bottomright(-1, -1)
{
	this->create_depth_buffer = Image_Base::create_depth_buffer;
	this->create_stencil_buffer = Image_Base::create_stencil_buffer;
	loaded_data = reload(keep_data, load_from_filesystem);
}

Image_Base::Internal::Internal(unsigned char *pixels, util::Size<int> size, bool support_render_to_texture) :
	loaded_data(0),
	size(size),
	has_render_to_texture(support_render_to_texture),
#ifdef _WIN32
	video_texture(0),
	render_target(0),
#endif
	texture(0),
	depth_buffer(0),
	stencil_buffer(0),
	parent(0),
	offset(0, 0),
	opaque_topleft(-1, -1),
	opaque_bottomright(-1, -1)
{
	this->create_depth_buffer = Image_Base::create_depth_buffer;
	this->create_stencil_buffer = Image_Base::create_stencil_buffer;
	filename = "--NOT LOADED--";
	upload(pixels);
}

Image_Base::Internal::Internal() :
	loaded_data(0),
	has_render_to_texture(false),
#ifdef _WIN32
	video_texture(0),
	render_target(0),
#endif
	texture(0),
	depth_buffer(0),
	stencil_buffer(0),
	parent(0),
	offset(0, 0),
	opaque_topleft(-1, -1),
	opaque_bottomright(-1, -1)
{
	this->create_depth_buffer = Image_Base::create_depth_buffer;
	this->create_stencil_buffer = Image_Base::create_stencil_buffer;
}

Image_Base::Internal::~Internal()
{
	release();

	delete[] loaded_data;
	loaded_data = 0;
}

void Image_Base::Internal::release()
{
	if (parent) {
		return;
	}

	unbind();

	if (shim::opengl) {
		if (bound_fbo == fbo) {
			bound_fbo = 0;
		}
		if (has_render_to_texture) {
			if (depth_buffer != 0) {
				glDeleteRenderbuffers_ptr(1, &depth_buffer);
				depth_buffer = 0;
			}
			if (stencil_buffer != 0) {
				glDeleteRenderbuffers_ptr(1, &stencil_buffer);
				stencil_buffer = 0;
			}
			if (fbo != 0) {
				glDeleteFramebuffers_ptr(1, &fbo);
				fbo = 0;
			}
		}

		if (texture != 0) {
			glDeleteTextures_ptr(1, &texture);
			PRINT_GL_ERROR("glDeleteTextures\n");
			texture = 0;
		}
	}
#ifdef _WIN32
	else {
		if (video_texture) {
			util::verbosemsg("Internal::release (%p), video_texture->Release=%d\n", this, video_texture->Release());
			video_texture = 0;
			d3d_video_texture_count--;
		}

		if (has_render_to_texture) {
			if (system_texture) {
				util::verbosemsg("Internal::release (%p), system_texture->Release=%d\n", this, system_texture->Release());
				system_texture = 0;
				d3d_system_texture_count--;
			}

			if (depth_stencil_buffer != 0) {
				if (bound_depth_buffer == depth_stencil_buffer) {
					shim::d3d_device->SetDepthStencilSurface(0);
				}

				bound_depth_buffer = 0;
				depth_buffer_bound = true;

				util::verbosemsg("Internal::release(%p), depth_stencil_buffer->Release=%d\n", this, depth_stencil_buffer->Release());
				depth_stencil_buffer = 0;
				
				d3d_depth_buffer_count--;
			}
		}
		
		if (render_target) {
			util::verbosemsg("Image_Base::Internal::release (%p), render_target->release=%d\n", this, render_target->Release());
			//while (render_target->Release());
			render_target = 0;
			d3d_surface_level_count--;
		}
	}
#endif
}

unsigned char *Image_Base::Internal::reload(bool keep_data, bool load_from_filesystem)
{
	unsigned char *pixels = Image_Base::read_tga(filename, size, NULL, &opaque_topleft, &opaque_bottomright, &has_alpha, load_from_filesystem);

	if (pixels == 0) {
		return 0;
	}

	try {
		upload(pixels);
	}
	catch (util::Error &) {
		delete[] pixels;
		throw;
	}

	if (keep_data == false) {
		delete[] pixels;
		return 0;
	}
	else {
		return pixels;
	}
}

void Image_Base::Internal::upload(unsigned char *pixels)
{
	// To get a complete palette..
	if (dumping_colours) {
		unsigned char *rgb = pixels;
		for (int i = 0; i < size.w*size.h; i++) {
			if (rgb[3] != 0) {
				printf("rgb: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
			}
			rgb += 4;
		}
	}

	if (shim::opengl && texture == 0) {
		glGenTextures_ptr(1, &texture);
		PRINT_GL_ERROR("glGenTextures\n");
		if (texture == 0) {
			throw util::GLError("glGenTextures failed");
		}

		glActiveTexture_ptr(GL_TEXTURE0);
		PRINT_GL_ERROR("glActiveTexture\n");

		glBindTexture_ptr(GL_TEXTURE_2D, texture);
		PRINT_GL_ERROR("glBindTexture\n");

#if 0
		glTexImage2D_ptr(GL_TEXTURE_2D, 0, GL_RGBA4, size.w, size.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#else
		glTexImage2D_ptr(GL_TEXTURE_2D, 0, GL_RGBA, size.w, size.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#endif
		PRINT_GL_ERROR("glTexImage2D\n");

		if (linear_filter) {
			glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			PRINT_GL_ERROR("glTexParameteri\n");
			glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			PRINT_GL_ERROR("glTexParameteri\n");
		}
		else {
			glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			PRINT_GL_ERROR("glTexParameteri\n");
			glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			PRINT_GL_ERROR("glTexParameteri\n");
		}
		glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		PRINT_GL_ERROR("glTexParameteri\n");
		glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		PRINT_GL_ERROR("glTexParameteri\n");

		if (has_render_to_texture) {
			// Create an FBO for render-to-texture

			glGenFramebuffers_ptr(1, &fbo);
			PRINT_GL_ERROR("glGenFramebuffers\n");

			glBindFramebuffer_ptr(GL_FRAMEBUFFER, fbo);
			PRINT_GL_ERROR("glBindFramebuffer\n");

			glFramebufferTexture2D_ptr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
			PRINT_GL_ERROR("glFramebufferTexture2D\n");

			if (this->create_depth_buffer) {
				GLenum format;
				if (this->create_stencil_buffer) {
					format = GL_DEPTH24_STENCIL8;
				}
				else {
					format = GL_DEPTH_COMPONENT16;
				}
#if defined ANDROID || defined IOS || defined RASPBERRYPI
				if (strstr((const char *)glGetString(GL_EXTENSIONS), "GL_OES_packed_depth_stencil") != 0) {
					glGenRenderbuffers_ptr(1, &depth_buffer); // use a combined depth and stencil as it must be supported
					PRINT_GL_ERROR("glGenRenderbuffers\n");
					glBindRenderbuffer_ptr(GL_RENDERBUFFER, depth_buffer);
					PRINT_GL_ERROR("glBindRenderbuffer\n");
					glRenderbufferStorage_ptr(GL_RENDERBUFFER, format, size.w, size.h);
					PRINT_GL_ERROR("glRenderbufferStorage\n");
					glFramebufferRenderbuffer_ptr(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
					PRINT_GL_ERROR("glFramebufferRenderbuffer\n");
					if (this->create_stencil_buffer) {
						glFramebufferRenderbuffer_ptr(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
						PRINT_GL_ERROR("glFramebufferRenderbuffer\n");
					}
				}
				else { // there will be no stencil buffer
					glGenRenderbuffers_ptr(1, &depth_buffer);
					PRINT_GL_ERROR("glGenRenderbuffers\n");
					glBindRenderbuffer_ptr(GL_RENDERBUFFER, depth_buffer);
					PRINT_GL_ERROR("glBindRenderbuffer\n");
					glRenderbufferStorage_ptr(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.w, size.h);
					PRINT_GL_ERROR("glRenderbufferStorage\n");
					glFramebufferRenderbuffer_ptr(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
					PRINT_GL_ERROR("glFramebufferRenderbuffer\n");
				}
#else
				glGenRenderbuffers_ptr(1, &depth_buffer); // use a combined depth and stencil as it must be supported
				PRINT_GL_ERROR("glGenRenderbuffers\n");
				glBindRenderbuffer_ptr(GL_RENDERBUFFER, depth_buffer);
				PRINT_GL_ERROR("glBindRenderbuffer\n");
				glRenderbufferStorage_ptr(GL_RENDERBUFFER, format, size.w, size.h);
				PRINT_GL_ERROR("glRenderbufferStorage\n");
				glFramebufferRenderbuffer_ptr(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
				PRINT_GL_ERROR("glFramebufferRenderbuffer\n");
				if (this->create_stencil_buffer) {
					glFramebufferRenderbuffer_ptr(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
					PRINT_GL_ERROR("glFramebufferRenderbuffer\n");
				}
#endif
			}

			GLenum result = glCheckFramebufferStatus_ptr(GL_FRAMEBUFFER);
			if (result != GL_FRAMEBUFFER_COMPLETE) {
				throw util::GLError("Incomplete framebuffer!");
			}
			PRINT_GL_ERROR("glCheckFramebufferStatus\n");

#ifdef IOS
			if (bound_fbo == 0) {
				glBindRenderbuffer_ptr(GL_RENDERBUFFER, internal::gfx_context.colorbuffer);
				glBindFramebuffer_ptr(GL_FRAMEBUFFER, internal::gfx_context.framebuffer);
			}
			else {
#endif
				glBindFramebuffer_ptr(GL_FRAMEBUFFER, bound_fbo);
#ifdef IOS
			}
#endif

			PRINT_GL_ERROR("glBindFramebuffer\n");
		}

		Shader::rebind_opengl_texture0();
	}
#ifdef _WIN32
	else if (video_texture == 0) {
		int err;

		if (has_render_to_texture) {
			err = shim::d3d_device->CreateTexture(size.w, size.h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &video_texture, 0);
			if (err != D3D_OK) {
				util::errormsg("CreateTexture failed for video texture (%dx%d, %d).\n", size.w, size.h, err);
			}
			d3d_video_texture_count++;

			err = shim::d3d_device->CreateTexture(size.w, size.h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &system_texture, 0);
			if (err != D3D_OK) {
				util::errormsg("CreateTexture failed for system texture (%dx%d, %d).\n", size.w, size.h, err);
			}
			d3d_system_texture_count++;

			D3DLOCKED_RECT locked_rect;
			if (system_texture->LockRect(0, &locked_rect, 0, 0) == D3D_OK) {
				for (int y = 0; y < size.h; y++) {
					unsigned char *dest = ((unsigned char *)locked_rect.pBits) + y * locked_rect.Pitch;
					for (int x = 0; x < size.w; x++) {
						unsigned char r = *pixels++;
						unsigned char g = *pixels++;
						unsigned char b = *pixels++;
						unsigned char a = *pixels++;
						*dest++ = b;
						*dest++ = g;
						*dest++ = r;
						*dest++ = a;
					}
				}
				system_texture->UnlockRect(0);
			}
			else {
				util::errormsg("Unable to lock system texture.\n");
			}

			if (shim::d3d_device->UpdateTexture((IDirect3DBaseTexture9 *)system_texture, (IDirect3DBaseTexture9 *)video_texture) != D3D_OK) {
				util::errormsg("UpdateTexture failed.\n");
			}

			if (this->create_depth_buffer) {
				D3DFORMAT format;
				if (this->create_stencil_buffer) {
					format = D3DFMT_D24S8;
				}
				else {
					format = D3DFMT_D16;
				}

				// Direct3D9 can't render if the depth buffer is smaller than the largest texture or screen: so adjust the size of the depth buffer if needed
				util::Size<int> depth_buffer_size;
				if (size.w < shim::real_screen_size.w || size.h < shim::real_screen_size.h) {
					depth_buffer_size = shim::real_screen_size;
				}
				else {
					depth_buffer_size = size;
				}
				if (shim::d3d_device->CreateDepthStencilSurface(depth_buffer_size.w, depth_buffer_size.h, format, D3DMULTISAMPLE_NONE, 0, true, &depth_stencil_buffer, 0) != D3D_OK) {
					throw util::Error("CreateDepthStencilSurface failed");
				}

				d3d_depth_buffer_count++;

				shim::d3d_device->SetDepthStencilSurface(depth_stencil_buffer);

				shim::d3d_device->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

				if (depth_buffer_bound) {
					shim::d3d_device->SetDepthStencilSurface(bound_depth_buffer);
				}
				else {
					shim::d3d_device->SetDepthStencilSurface(internal::gfx_context.depth_stencil_buffer);
				}
				depth_buffer_bound = true;
			}
			else {
				depth_stencil_buffer = 0;
			}
		}
		else {
			err = shim::d3d_device->CreateTexture(size.w, size.h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &video_texture, 0);
			if (err != D3D_OK) {
				util::errormsg("CreateTexture failed for video texture (%dx%d, %d).\n", size.w, size.h, err);
			}
			d3d_video_texture_count++;

			D3DLOCKED_RECT locked_rect;
			if (video_texture->LockRect(0, &locked_rect, 0, 0) == D3D_OK) {
				for (int y = 0; y < size.h; y++) {
					unsigned char *dest = ((unsigned char *)locked_rect.pBits) + y * locked_rect.Pitch;
					for (int x = 0; x < size.w; x++) {
						unsigned char r = *pixels++;
						unsigned char g = *pixels++;
						unsigned char b = *pixels++;
						unsigned char a = *pixels++;
						*dest++ = b;
						*dest++ = g;
						*dest++ = r;
						*dest++ = a;
					}
				}
				video_texture->UnlockRect(0);
			}
			else {
				util::errormsg("Unable to lock video texture.\n");
			}

			depth_stencil_buffer = 0;
		}
	}
#endif
}

void Image_Base::Internal::unbind()
{
	std::vector< std::pair<std::string, Image_Base *> > bound_images = Shader::get_bound_images();
	for (size_t unit = 0; unit < bound_images.size(); unit++) {
		std::pair<std::string, Image_Base *> p = bound_images[unit];
		if (p.second->internal == this) {
			shim::current_shader->set_texture(p.first, 0, (int)unit);
			break;
		}
	}
}

void Image_Base::Internal::destroy_data()
{
	delete[] loaded_data;
	loaded_data = 0;
}

bool Image_Base::Internal::is_transparent(util::Point<int> position)
{
	if (loaded_data == 0) {
		return false;
	}

	unsigned char *p = loaded_data + ((((size.h-1)-position.y) * size.w) + position.x) * 4;

	return p[3] == 0;
}

} // End namespace gfx

} // End namespace noo
