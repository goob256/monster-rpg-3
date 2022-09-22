#ifndef NOO_FONT_H
#define NOO_FONT_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace gfx {

class Image;

EXPORT_CLASS_ALIGN(Font, 16) {
public:
	static std::string strip_codes(std::string text, bool strip_colour_codes, bool strip_wrap_codes);
	static void static_start();
	static void release_all();
	static void end_batches();

	enum Shadow_Type {
		NO_SHADOW = 0,
		DROP_SHADOW,
		FULL_SHADOW,
		NUM_SHADOW_TYPES
	};

	Font();
	virtual ~Font();

	virtual bool cache_glyphs(std::string text) = 0;
	virtual void clear_cache() = 0;

	virtual int get_text_width(std::string text, bool interpret_colour_codes = true);
	virtual int get_height() = 0;

	void enable_shadow(SDL_Colour shadow_colour, Shadow_Type shadow_type);
	void disable_shadow();

	// If interpret_colour_codes is true, this returns the last colour used. Otherwise, it returns the colour passed in.
	SDL_Colour draw(SDL_Colour colour, std::string text, util::Point<float> dest_position, bool interpret_colour_codes = true, bool centre = false);

	// Returns number of characters drawn, plus whether or not it filled the max in bool &full
	int draw_wrapped(SDL_Colour colour, std::string text, util::Point<float> dest_position, int w, int line_height, int max_lines, int elapsed, int delay, bool dry_run, bool &full, int &num_lines, int &width, bool interpret_colour_codes = true, bool centre = false, int first_line_indent = 0);

	bool is_batching();

	void set_vertex_cache_id(Uint32 cache_id);

	void start_batch();
	void end_batch();
	void flush_vertex_cache();

	// For 16 byte alignment to make glm::mat4 able to use SIMD
#ifdef _WIN32
	void *operator new(size_t i);
	void operator delete(void* p);
#endif
	
protected:
	static std::vector<Font *> loaded_fonts;

	void select_vertex_cache();
	void select_previous_vertex_cache();

	struct Glyph {
		util::Point<int> position;
		util::Size<int> size;
	};

	std::map<Uint32, Glyph *> glyphs;

	SDL_Colour shadow_colour;
	Shadow_Type shadow_type;

	bool batching;
	Uint32 vertex_cache_id;
	Uint32 prev_vertex_cache_id;

	Image *sheet;

	int size;
};

} // End namespace gfx

} // End namespace noo

#endif // NOO_FONT_H
