#include "Nooskewl_Shim/pixel_font.h"
#include "Nooskewl_Shim/image.h"
#include "Nooskewl_Shim/tokenizer.h"
#include "Nooskewl_Shim/utf8.h"
#include "Nooskewl_Shim/util.h"

using namespace noo;

namespace noo {

namespace gfx {

Pixel_Font::Pixel_Font(std::string name)
{
	std::string dirname = "gfx/fonts/" + name + "/";

	bool old_keep_data = gfx::Image::keep_data;
	gfx::Image::keep_data = true;
	/*
	if (shim::use_hires_font) {
		gfx::Image::linear_filter = true;
	}
	*/
	sheet = new gfx::Image(dirname + "font.tga", true);
	gfx::Image::keep_data = old_keep_data;
	//gfx::Image::linear_filter = false;

	std::string info = util::load_text(dirname + "info.ini");

	util::Tokenizer t(info, '\n');
	std::string line;

	size = 10; // default

	while ((line = t.next()) != "") {
		util::trim(line);
		util::Tokenizer t2(line, '=');
		std::string key = t2.next();
		std::string value = t2.next();

		util::trim(key);
		util::trim(value);

		if (key == "size") {
			size = atoi(value.c_str());
		}
	}

	int PAD;

	if (shim::use_hires_font) {
		PAD = 5;
	}
	else {
		PAD = 2;
	}

	int glyphs_wide = sheet->size.w / ((size + PAD*2)*3);

	unsigned char *loaded_data = sheet->get_loaded_data();

	std::string g = util::load_text(dirname + "glyphs.utf8");
	Uint32 ch;
	int offset = 0;
	int count = 0;
	while ((ch = util::utf8_char_next(g, offset)) != 0) {
		int x = count % glyphs_wide;
		int y = count / glyphs_wide;
		Glyph *glyph = new Glyph;
		glyph->position = util::Point<int>(x * ((size + PAD*2)*3) + PAD, y * (size + PAD*2) + PAD);
		glyph->size.h = size;
		int max_x;
		if (ch == ' ') {
			max_x = name[0] == 'h' ? 6 : 1; // FIXME: need a better way
		}
		else {
			max_x = 0;
			for (int y = 0; y < size; y++) {
				unsigned char *p = loaded_data + (sheet->size.w * 4 * (sheet->size.h-1)) - ((glyph->position.y+y) * sheet->size.w * 4) + (glyph->position.x * 4);
				for (int x = 0; x < size; x++) {
					if (x > max_x && p[3] != 0) {
						max_x = x;
					}
					p += 4;
				}
			}
		}
		glyph->size.w = max_x + 2; // +1 for an extra pixel space and +1 because max_x is an index and this is a size
		glyphs[ch] = glyph;
		count++;
	}
}

Pixel_Font::~Pixel_Font()
{
	for (std::vector<Font *>::iterator it = loaded_fonts.begin(); it != loaded_fonts.end(); it++) {
		Font *f = *it;
		if (this == f) {
			loaded_fonts.erase(it);
			break;
		}
	}

	std::map<Uint32, Glyph *>::iterator it;
	for  (it = glyphs.begin(); it != glyphs.end(); it++) {
		std::pair<int, Glyph *> p = *it;
		delete p.second;
	}

	delete sheet;
}

void Pixel_Font::clear_cache()
{
}

int Pixel_Font::get_height()
{
	if (shim::use_hires_font) {
		return size / 4.0f;
	}
	else {
		return size;
	}
}

bool Pixel_Font::cache_glyphs(std::string text)
{
	// This can safely be here
	if (std::find(loaded_fonts.begin(), loaded_fonts.end(), this) == loaded_fonts.end()) {
		loaded_fonts.push_back(this);
	}
	return true;
}

} // End namespace gfx

} // End namespace noo
