#ifndef NOO_TTF_H
#define NOO_TTF_H

#include "Nooskewl_Shim/main.h"
#include "Nooskewl_Shim/font.h"

namespace noo {

namespace gfx {

class Shader;
class Image;

class TTF : public Font
{
public:
	static const int DEFAULT_SHEET_SIZE = 2048;

	TTF(std::string filename, int size, int sheet_size = DEFAULT_SHEET_SIZE);
	virtual ~TTF();

	bool cache_glyphs(std::string text);
	void clear_cache();

	int get_height();
	
private:
	int get_sheet_size();
	void set_sheet_target();
	void restore_target();
	Glyph *create_glyph(Uint32 ch, gfx::Image *glyph_image);
	void render_glyph(Glyph *glyph, Shadow_Type type, gfx::Image *glyph_image, gfx::Image *glyph_image_noaa);
	void load_glyph_image(Uint32 ch, gfx::Image *&glyph_image, gfx::Image *&glyph_image_noaa);

	TTF_Font *font;

	int num_glyphs;

	int sheet_size;

	glm::mat4 modelview, proj;
	gfx::Shader *old_shader;
	gfx::Image *old_target;

	SDL_RWops *file;
};

} // End namespace gfx

} // End namespace noo

#endif // NOO_TTF_H
