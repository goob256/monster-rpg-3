#include "Nooskewl_Shim/font.h"
#include "Nooskewl_Shim/gfx.h"
#include "Nooskewl_Shim/image.h"
#include "Nooskewl_Shim/utf8.h"
#include "Nooskewl_Shim/util.h"

using namespace noo;

namespace noo {

namespace gfx {

std::vector<Font *> Font::loaded_fonts;

std::string Font::strip_codes(std::string text, bool strip_colour_codes, bool strip_wrap_codes)
{
	Uint32 ch;
	int offset = 0;
	std::string stripped;
	while ((ch = util::utf8_char_next(text, offset)) != 0) {
		// Colour code interpretation
		if (strip_colour_codes && ch == '|') {
			Uint32 a = util::utf8_char_next(text, offset);
			if (a == 0) {
				break;
			}
			Uint32 b = util::utf8_char_next(text, offset);
			if (b == 0) {
				break;
			}
			continue;
		}
		else if (strip_wrap_codes && (ch == '^' || ch == '$')) {
			continue;
		}
		stripped += util::utf8_char_to_string(ch);
	}
	return stripped;
}

void Font::static_start()
{
	loaded_fonts.clear();
}

void Font::release_all()
{
	for (std::vector<Font *>::iterator it = loaded_fonts.begin(); it != loaded_fonts.end(); it++) {
		Font *f = *it;
		f->clear_cache();
	}
}

void Font::end_batches()
{
	if (loaded_fonts.size() == 0) {
		return;
	}

	glm::mat4 mv_save, proj_save;
	gfx::get_matrices(mv_save, proj_save);
	if (shim::use_hires_font) {
#ifdef USE_TTF
		glm::mat4 m = glm::scale(mv_save, glm::vec3(1.0f/int(shim::scale), 1.0f/int(shim::scale), 1.0f));
#else
		glm::mat4 m = glm::scale(mv_save, glm::vec3(1.0f/4.0f, 1.0f/4.0f, 1.0f));
#endif
		gfx::set_matrices(m, proj_save);
		gfx::update_projection();
	}

	for (std::vector<Font *>::iterator it = loaded_fonts.begin(); it != loaded_fonts.end(); it++) {
		Font *f = *it;
		if (f->is_batching()) {
			f->flush_vertex_cache();
		}
	}

	if (shim::use_hires_font) {
		gfx::set_matrices(mv_save, proj_save);
		gfx::update_projection();
	}
}

Font::Font() :
	shadow_type(NO_SHADOW),
	batching(false),
	vertex_cache_id(0)
{
}

Font::~Font()
{
}

int Font::get_text_width(std::string text, bool interpret_colour_codes)
{
	if (cache_glyphs(strip_codes(text, interpret_colour_codes, false)) == false) {
		return 0;
	}

	int width = 0;
	int offset = 0;
	int ch;

	while ((ch = util::utf8_char_next(text, offset)) != 0) {
		// Colour code interpretation
		if (interpret_colour_codes && ch == '|') {
			// Skip them here, use them below
			int a = util::utf8_char_next(text, offset);
			if (a == 0) {
				break;
			}
			int b = util::utf8_char_next(text, offset);
			if (b == 0) {
				break;
			}

			continue;
		}

		std::map<Uint32, Glyph *>::iterator it = glyphs.find(ch);

		if (it == glyphs.end()) {
			width += 5;
			continue;
		}

		std::pair<Uint32, Glyph *> pair = *it;
		Glyph *glyph = pair.second;
		if (shim::use_hires_font) {
#ifdef USE_TTF
			width += (float)glyph->size.w / int(shim::scale) + 1;
#else
			width += glyph->size.w / 4.0f + 1;
#endif
		}
		else {
			width += glyph->size.w;
		}
	}

	return width - 1; // glyph size includes advance, subtract 1 because no advance is needed on the final character
}

void Font::enable_shadow(SDL_Colour shadow_colour, Shadow_Type shadow_type)
{
	this->shadow_colour = shadow_colour;
	this->shadow_type = shadow_type;
}

void Font::disable_shadow()
{
	this->shadow_type = NO_SHADOW;
}

SDL_Colour Font::draw(SDL_Colour colour, std::string text, util::Point<float> dest_position, bool interpret_colour_codes, bool centre)
{
	if (text == "") {
		return colour;
	}

	if (cache_glyphs(strip_codes(text, interpret_colour_codes, false)) == false) {
		return colour;
	}

	if (shim::use_hires_font) {
		dest_position.y++; // hack
	}

	if (centre) {
		dest_position.x -= get_text_width(text, interpret_colour_codes) / 2;
	}

	dest_position.x = dest_position.x;
	dest_position.y = (dest_position.y-1);

	if (shim::use_hires_font) {
#ifdef USE_TTF
		dest_position.x *= int(shim::scale);
		dest_position.y *= int(shim::scale);
#else
		dest_position.x *= 4.0f;
		dest_position.y *= 4.0f;
#endif
	}

	util::Point<float> pos = dest_position;
	int offset = 0;
	int ch;

	if (batching) {
		select_vertex_cache();
	}
	else {
		sheet->start_batch();
	}

	util::Point<int> glyph_offset;
	util::Size<int> glyph_size_diff;
	util::Point<int> draw_offset;
	int PAD;
	if (shim::use_hires_font) {
#ifdef USE_TTF
		PAD = int(shim::scale)+1;
#else
		PAD = 5;
#endif
	}
	else {
		PAD = 2;
	}
	int w = size + PAD*2;
	if (shadow_type == DROP_SHADOW) {
		glyph_offset = util::Point<int>(w, 0);
		glyph_size_diff = util::Size<int>(PAD-1, PAD-1);
		draw_offset = util::Point<int>(0, 0);
	}
	else {
		glyph_offset = util::Point<int>(w*2-(PAD-1), -(PAD-1));
		glyph_size_diff = util::Size<int>((PAD-1)*2, (PAD-1)*2);
		draw_offset = util::Point<int>(-(PAD-1), -(PAD-1));
	}

	util::Point<float> bak = pos;

	/*
	// FIXME:
	static int count = 0;
	if (sheet && count++ > 10) {
		gfx::clear(shim::black);
		gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, 1.0f);
		gfx::update_projection();
		sheet->draw(util::Point<int>(0, 0));
		gfx::flip();
		//SDL_Delay(10000);;
	}
	*/

	if (shadow_type != NO_SHADOW) {
		while ((ch = util::utf8_char_next(text, offset)) != 0) {
			// Colour code interpretation
			if (interpret_colour_codes && ch == '|') {
				// Skip them here, use them below
				int a = util::utf8_char_next(text, offset);
				if (a == 0) {
					break;
				}
				int b = util::utf8_char_next(text, offset);
				if (b == 0) {
					break;
				}

				if (a >= 'A' && a <= 'F') {
					a = a - 'A' + 10;
				}
				else if (a >= 'a' && a <= 'f') {
					a = a - 'a' + 10;
				}
				else {
					a = a - '0';
				}
				if (b >= 'A' && b <= 'F') {
					b = b - 'A' + 10;
				}
				else if (b >= 'a' && b <= 'f') {
					b = b - 'a' + 10;
				}
				else {
					b = b - '0';
				}
				int index = a * 16 + b;
				colour = shim::palette[index];

				continue;
			}

			std::map<Uint32, Glyph *>::iterator it = glyphs.find(ch);
			if (it == glyphs.end()) {
				if (ch != 160) { // non-breaking space
					util::debugmsg("missing glyph! %u\n", ch);
				}
				pos.x += 5;
				continue;
			}

			std::pair<Uint32, Glyph *> pair = *it;
			Glyph *glyph = pair.second;

			sheet->draw_region_tinted(shadow_colour, glyph->position+glyph_offset, glyph->size+glyph_size_diff, pos+draw_offset, 0);

			if (shim::use_hires_font) {
#ifdef USE_TTF
				pos.x += int((float)glyph->size.w / int(shim::scale) + 1) * int(shim::scale);
#else
				pos.x += int(glyph->size.w / 4.0f + 1) * 4;
#endif
			}
			else {
				pos.x += glyph->size.w;
			}
		}

		offset = 0;
		pos = bak;
	}

	while ((ch = util::utf8_char_next(text, offset)) != 0) {
		// Colour code interpretation
		if (interpret_colour_codes && ch == '|') {
			// Skip them here, use them below
			int a = util::utf8_char_next(text, offset);
			if (a == 0) {
				break;
			}
			int b = util::utf8_char_next(text, offset);
			if (b == 0) {
				break;
			}

			if (a >= 'A' && a <= 'F') {
				a = a - 'A' + 10;
			}
			else if (a >= 'a' && a <= 'f') {
				a = a - 'a' + 10;
			}
			else {
				a = a - '0';
			}
			if (b >= 'A' && b <= 'F') {
				b = b - 'A' + 10;
			}
			else if (b >= 'a' && b <= 'f') {
				b = b - 'a' + 10;
			}
			else {
				b = b - '0';
			}
			int index = a * 16 + b;
			colour = shim::palette[index];

			continue;
		}

		std::map<Uint32, Glyph *>::iterator it = glyphs.find(ch);
		if (it == glyphs.end()) {
			if (ch != 160) { // non-breaking space
				util::debugmsg("missing glyph! %u\n", ch);
			}
			pos.x += 5;
			continue;
		}

		std::pair<Uint32, Glyph *> pair = *it;
		Glyph *glyph = pair.second;

		sheet->draw_region_tinted(colour, glyph->position, glyph->size, pos, 0);

		if (shim::use_hires_font) {
#ifdef USE_TTF
			pos.x += int((float)glyph->size.w / int(shim::scale) + 1) * int(shim::scale);
#else
			pos.x += int(glyph->size.w / 4.0f + 1) * 4;
#endif
		}
		else {
			pos.x += glyph->size.w;
		}
	}

	if (batching) {
		select_previous_vertex_cache();
	}
	else {
		sheet->end_batch();
	}
	
	return colour;
}

int Font::draw_wrapped(SDL_Colour colour, std::string text, util::Point<float> dest_position, int w, int line_height, int max_lines, int elapsed, int delay, bool dry_run, bool &full, int &num_lines, int &width, bool interpret_colour_codes, bool centre, int first_line_indent)
{
	if (cache_glyphs(strip_codes(text, interpret_colour_codes, true)) == false) {
		return 0;
	}

	bool was_batching = batching;
	if (was_batching == false) {
		start_batch();
	}

	full = false;
	float curr_y = dest_position.y;
	bool done = false;
	int lines = 0;
	if (max_lines == -1) {
		max_lines = 1000000;
	}
	if (elapsed < 0) {
		elapsed = 1000000;
	}
	int chars_to_draw;
	if (delay == 0) {
		chars_to_draw = 1000000;
	}
	else {
		chars_to_draw = elapsed / delay;
	}
	int chars_drawn = 0;
	int max_width = 0;
	std::string p = text;
	int total_position = 0;
	while (done == false && lines < max_lines) {
		int count = 0;
		int offset = 0;
		std::vector<int> last_offset;
		int max = 0;
		int this_w = 0;
		int chars_drawn_this_time = 0;
		last_offset.push_back(offset);
		Uint32 ch = util::utf8_char_next(p, offset);
		bool set_full = false;
		bool any_spaces = false;
		int skip_next = 0;
		bool skip_check = false;
		while (ch) {
			if (ch == ' ') {
				any_spaces = true;
			}
			if (interpret_colour_codes && ch == '|') {
				skip_next = 2;
			}
			else if (ch != '^' && ch != '$') {
				if (skip_next > 0) {
					skip_next--;
				}
				else {
					std::map<Uint32, Glyph *>::iterator it = glyphs.find(ch);
					if (it == glyphs.end()) {
						this_w += 5;
					}
					else {
						std::pair<Uint32, Glyph *> pair = *it;
						Glyph *glyph = pair.second;
						if (shim::use_hires_font) {
#ifdef USE_TTF
							this_w += (float)glyph->size.w/int(shim::scale) + 1;
#else
							this_w += glyph->size.w/4.0f + 1;
#endif
						}
						else {
							this_w += glyph->size.w;
						}
					}
				}
			}
			int line_max_w = (w == INT_MAX) ? w : w+1; // + 1 because the last character will have no advance (see get_text_width)
			if (lines == 0) {
				line_max_w -= first_line_indent;
			}
			if (ch == '^' || ch == '$' || this_w > line_max_w) {
				if (count == 0) {
					if (lines == 0) {
						max = 0;
					}
					else {
						done = true;
					}
				}
				else {
					if (ch == '^') {
						max = count;
						if (set_full) {
							full = true;
						}
					}
					else if (ch == '$') {
						max = count;
					}
					else if (this_w > line_max_w) {
						if (any_spaces && ch != ' ') {
							count--;
							last_offset.pop_back();
							offset = last_offset.back();
						}
						else {
							if (any_spaces == false && ch != ' ' && lines == 0) {
								/*
								lines++;
								curr_y += line_height;
								max = (int)text.length();
								skip_check = true;
								chars_drawn_this_time = max;
								set_full = true;
								*/
								max = 0;
							}
							else {
								max = count;
							}
						}
					}
				}
				break;
			}
			if (ch == ' ') {
				max = count;
			}
			count++;
			last_offset.push_back(offset);
			ch =  util::utf8_char_next(p, offset);
			if (chars_drawn+count < chars_to_draw) {
				chars_drawn_this_time++;
				set_full = true;
			}
			else {
				set_full = false;
			}
		}
		if (skip_check == false && util::utf8_char_offset(p, last_offset.back()) == 0) {
			max = count;
		}
		int old_max = max;
		max = MIN(chars_drawn_this_time, max);
		if (done == false) {
			std::string s = util::utf8_substr(p, 0, max);
			int line_w = get_text_width(strip_codes(s, interpret_colour_codes, true));
			if (line_w > max_width) {
				max_width = line_w;
			}
			if (dry_run == false) {
				float dx = dest_position.x;
				if (lines == 0) {
					dx += first_line_indent;
				}
				colour = draw(colour, s, util::Point<float>(dx, curr_y), interpret_colour_codes, centre);
			}
			total_position += max;
			p = util::utf8_substr(text, total_position);
			Uint32 ch2 = util::utf8_char(p, 0);
			if (ch2 == ' ') {
				total_position++;
				p = util::utf8_substr(text, total_position);
			}
			else if (ch == '^') { // new window
				total_position++;
				done = true;
				// Don't include in printed string
			}
			else if (ch == '$') { // newline
				total_position++;
				p = util::utf8_substr(text, total_position);
			}
			chars_drawn = total_position;
			curr_y += line_height;
			if (max < old_max) {
				done = true;
			}
			else {
				lines++;
				if (lines >= max_lines) {
					full = true;
				}
			}
		}
		if (util::utf8_char(p, 0) == 0) {
			done = true;
			full = true;
		}
		if (chars_drawn >= chars_to_draw) {
			done = true;
		}
	}

	width = max_width;
	num_lines = lines;

	if (was_batching == false) {
		end_batch();
	}

	return chars_drawn;
}

bool Font::is_batching()
{
	return batching;
}

void Font::set_vertex_cache_id(Uint32 cache_id)
{
	vertex_cache_id = cache_id;
}

void Font::start_batch()
{
	if (sheet != 0) {
		select_vertex_cache();
		sheet->start_batch();
		select_previous_vertex_cache();
	}
	batching = true;
}

void Font::end_batch()
{
	flush_vertex_cache();
	batching = false;
}

void Font::flush_vertex_cache()
{
	if (sheet != 0) {
		select_vertex_cache();
		Vertex_Cache::instance()->end();
		sheet->start_batch();
		select_previous_vertex_cache();
	}
}

void Font::select_vertex_cache()
{
	prev_vertex_cache_id = Vertex_Cache::instance()->get_current_cache();
	Vertex_Cache::instance()->select_cache(vertex_cache_id);
}

void Font::select_previous_vertex_cache()
{
	Vertex_Cache::instance()->select_cache(prev_vertex_cache_id);
}

#ifdef _WIN32
void *Font::operator new(size_t i)
{
	return _mm_malloc(i,16);
}

void Font::operator delete(void* p)
{
	_mm_free(p);
}
#endif

} // End namespace gfx

} // End namespace noo
