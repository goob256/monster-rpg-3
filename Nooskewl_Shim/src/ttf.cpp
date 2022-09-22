#include "Nooskewl_Shim/error.h"
#include "Nooskewl_Shim/gfx.h"
#include "Nooskewl_Shim/image.h"
#include "Nooskewl_Shim/shader.h"
#include "Nooskewl_Shim/ttf.h"
#include "Nooskewl_Shim/utf8.h"
#include "Nooskewl_Shim/util.h"

#include "Nooskewl_Shim/internal/gfx.h"

using namespace noo;

namespace noo {

namespace gfx {

TTF::TTF(std::string filename, int size, int sheet_size) :
	sheet_size(sheet_size)
{
	filename = "gfx/fonts/" + filename;

	int sz;

	file = util::open_file(filename, &sz); // FIXME: does this get automatically closed? (I think yes)

	font = TTF_OpenFontRW(file, true, size);

	if (font == 0) {
		throw util::LoadError("TTF_OpenFontRW failed");
	}

	TTF_SetFontHinting(font, TTF_HINTING_NONE);

	num_glyphs = 0;

	sheet = 0;

	this->size = TTF_FontHeight(font);
}

TTF::~TTF()
{
	clear_cache();

	for (std::vector<Font *>::iterator it = loaded_fonts.begin(); it != loaded_fonts.end(); it++) {
		Font *f = *it;
		if (this == f) {
			loaded_fonts.erase(it);
			break;
		}
	}

	TTF_CloseFont(font);

	util::free_data(file);
}

void TTF::clear_cache()
{
	std::map<Uint32, Glyph *>::iterator it;
	for  (it = glyphs.begin(); it != glyphs.end(); it++) {
		std::pair<int, Glyph *> p = *it;
		delete p.second;
	}
	glyphs.clear();
	num_glyphs = 0;

	delete sheet;
	sheet = 0;
}

int TTF::get_height()
{
	if (shim::use_hires_font) {
		return (float)size / int(shim::scale);
	}
	else {
		return size;
	}
}

int TTF::get_sheet_size()
{
	int this_sheet_size = sheet_size;

	// Textures can't be bigger than the screen in D3D
	if (shim::opengl == false) {
		while (this_sheet_size > shim::real_screen_size.w || this_sheet_size > shim::real_screen_size.h) {
			this_sheet_size -= 16;
		}
		this_sheet_size = MAX(16, this_sheet_size);
	}

	return this_sheet_size;
}

void TTF::set_sheet_target()
{
	bool created;

	if (sheet == 0) {
		if (std::find(loaded_fonts.begin(), loaded_fonts.end(), this) == loaded_fonts.end()) {
			loaded_fonts.push_back(this);
		}

		int this_sheet_size = get_sheet_size();

		bool cdb = Image::create_depth_buffer;
		Image::create_depth_buffer = false;
		sheet = new Image(util::Size<int>(this_sheet_size, this_sheet_size)); // FIXME: adjust size for other languages
		Image::create_depth_buffer = cdb;
		if (batching) {
			select_vertex_cache();
			sheet->start_batch();
			select_previous_vertex_cache();
		}

		created = true;
	}
	else {
		created = false;
	}

	old_target = get_target_image();
	gfx::get_matrices(modelview, proj);
	old_shader = shim::current_shader;

	shim::current_shader = shim::default_shader;
	shim::current_shader->use();
	set_target_image(sheet);

	if (created) {
		gfx::clear(shim::transparent);
	}
}

void TTF::restore_target()
{
	shim::current_shader = old_shader;
	shim::current_shader->use();
	set_target_image(old_target);
	gfx::set_matrices(modelview, proj);
	gfx::update_projection();
}

TTF::Glyph *TTF::create_glyph(Uint32 ch, gfx::Image *glyph_image)
{
	int this_sheet_size = get_sheet_size();

	int PAD;
	if (shim::use_hires_font) {
		PAD = int(shim::scale)+1;
	}
	else {
		PAD = 2;
	}

	int n = num_glyphs;
	int n_per_row = this_sheet_size / ((size + PAD*2) * 3);
	int row = n / n_per_row;
	int col = n % n_per_row;
	Glyph *glyph = new Glyph;
	glyph->position = util::Point<int>(col * ((size+PAD*2) * 3) + PAD, row * (size+PAD*2) + PAD);
	glyph->size = glyph_image->size;

	glyphs[ch] = glyph;

	num_glyphs++;

	return glyph;
}

void TTF::render_glyph(Glyph *glyph, Shadow_Type type, gfx::Image *glyph_image, gfx::Image *glyph_image_noaa)
{
	int PAD;
	if (shim::use_hires_font) {
		PAD = int(shim::scale)+1;
	}
	else {
		PAD = 2;
	}

	int w = size + PAD*2;
	if (type == NO_SHADOW) {
		glyph_image->draw(glyph->position, Image::FLIP_V); // glyphs are rendered upside down
	}
	else if (type == DROP_SHADOW) {
		if (shim::use_hires_font) {
			glyph_image_noaa->start_batch();
			for (int y = 0; y < int(shim::scale); y++) {
				for (int x = 0; x < int(shim::scale); x++) {
					glyph_image_noaa->draw(glyph->position+util::Point<int>(w+x, y), Image::FLIP_V);
				}
			}
			glyph_image_noaa->end_batch();
		}
		else {
			glyph_image->start_batch();
			glyph_image->draw(glyph->position+util::Point<int>(w+1, 1), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w+0, 1), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w+1, 0), Image::FLIP_V);
			glyph_image->end_batch();
		}
	}
	else if (type == FULL_SHADOW) {
		if (shim::use_hires_font) {
			glyph_image_noaa->start_batch();
			for (int y = -int(shim::scale); y < int(shim::scale); y++) {
				for (int x = -int(shim::scale); x < int(shim::scale); x++) {
					glyph_image_noaa->draw(glyph->position+util::Point<int>(w*2+x, y), Image::FLIP_V);
				}
			}
			glyph_image_noaa->end_batch();
		}
		else {
			glyph_image->start_batch();
			glyph_image->draw(glyph->position+util::Point<int>(w*2+1, 0), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w*2+0, 1), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w*2-1, 0), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w*2+0, -1), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w*2+1, 1), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w*2-1, -1), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w*2-1, 1), Image::FLIP_V);
			glyph_image->draw(glyph->position+util::Point<int>(w*2+1, -1), Image::FLIP_V);
			glyph_image->end_batch();
		}
	}
}

void TTF::load_glyph_image(Uint32 ch, gfx::Image *&glyph_image, gfx::Image *&glyph_image_noaa)
{
	//SDL_Surface *surface = TTF_RenderGlyph_Blended(font, (Uint16)ch, shim::white);
	SDL_Surface *surface = TTF_RenderGlyph_Solid(font, (Uint16)ch, shim::white);
	if (surface == 0) {
		util::errormsg("Error rendering glyph.\n");
		return;
	}

	SDL_Surface *surface_noaa = TTF_RenderGlyph_Solid(font, (Uint16)ch, shim::white);
	if (surface_noaa == 0) {
		util::errormsg("Error rendering glyph.\n");
		return;
	}

	bool cdb = Image::create_depth_buffer;
	Image::create_depth_buffer = false;

	glyph_image = new Image(surface);
	glyph_image_noaa = new Image(surface_noaa);

	Image::create_depth_buffer = cdb;
	
	SDL_FreeSurface(surface);
	SDL_FreeSurface(surface_noaa);
}

bool TTF::cache_glyphs(std::string text)
{
#ifdef _WIN32
	if (internal::gfx_context.d3d_lost) {
		return false;
	}
#endif

	bool set_target = false;

	int offset = 0;
	Uint32 ch;
	while ((ch = util::utf8_char_next(text, offset)) != 0) {
		if (glyphs.find(ch) == glyphs.end()) {
			if (set_target == false) {
				set_target = true;
				set_sheet_target();
			}
			gfx::Image *glyph_image;
			gfx::Image *glyph_image_noaa;
			load_glyph_image(ch, glyph_image, glyph_image_noaa);
			if (glyph_image == 0) {
				return false;
			}
			Glyph *glyph = create_glyph(ch, glyph_image);
			if (glyph == 0) {
				delete glyph_image;
				return false;
			}
			for (int i = 0; i < NUM_SHADOW_TYPES; i++) {
				render_glyph(glyph, (Shadow_Type)i, glyph_image, glyph_image_noaa);
			}
			delete glyph_image;
			delete glyph_image_noaa;
		}
	}

	if (set_target == true) {
		restore_target();
	}
	
	return true;
}

} // End namespace gfx

} // End namespace noo
