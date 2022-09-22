#ifndef NOOSKEWL_WEDGE_GENERAL_H
#define NOOSKEWL_WEDGE_GENERAL_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/stats.h"

namespace wedge {

util::Point<int> NOOSKEWL_WEDGE_EXPORT add_direction(util::Point<int> pos, Direction direction, int n);
Direction NOOSKEWL_WEDGE_EXPORT direction_from_offset(util::Point<int> offset);

std::string NOOSKEWL_WEDGE_EXPORT integer_point_to_string(util::Point<int> p);
std::string NOOSKEWL_WEDGE_EXPORT float_point_to_string(util::Point<float> p);
std::string NOOSKEWL_WEDGE_EXPORT integer_size_to_string(util::Size<int> s);
std::string NOOSKEWL_WEDGE_EXPORT float_size_to_string(util::Size<float> s);
std::string NOOSKEWL_WEDGE_EXPORT direction_to_string(Direction d);
std::string NOOSKEWL_WEDGE_EXPORT bool_to_string(bool b);
std::string NOOSKEWL_WEDGE_EXPORT sprite_to_string(gfx::Sprite *sprite);
std::string NOOSKEWL_WEDGE_EXPORT image_to_string(gfx::Image *image);

util::Point<int> NOOSKEWL_WEDGE_EXPORT json_to_integer_point(util::JSON::Node *json);
util::Point<float> NOOSKEWL_WEDGE_EXPORT json_to_float_point(util::JSON::Node *json);
util::Size<int> NOOSKEWL_WEDGE_EXPORT json_to_integer_size(util::JSON::Node *json);
util::Size<float> NOOSKEWL_WEDGE_EXPORT json_to_float_size(util::JSON::Node *json);
Direction NOOSKEWL_WEDGE_EXPORT json_to_direction(util::JSON::Node *json);
bool NOOSKEWL_WEDGE_EXPORT json_to_bool(util::JSON::Node *json);
gfx::Sprite NOOSKEWL_WEDGE_EXPORT *json_to_sprite(util::JSON::Node *json);
int NOOSKEWL_WEDGE_EXPORT json_to_integer(util::JSON::Node *json);
gfx::Image NOOSKEWL_WEDGE_EXPORT *json_to_image(util::JSON::Node *json);

std::string NOOSKEWL_WEDGE_EXPORT save();
bool NOOSKEWL_WEDGE_EXPORT save(std::string s, std::string filename);
bool NOOSKEWL_WEDGE_EXPORT save(std::string filename);
bool NOOSKEWL_WEDGE_EXPORT save_play_time(std::string filename);

util::Point<int> NOOSKEWL_WEDGE_EXPORT get_mouse_position();

void NOOSKEWL_WEDGE_EXPORT pause_player_input(bool paused);
void NOOSKEWL_WEDGE_EXPORT pause_presses(bool paused, bool repeat_pressed = false);
bool NOOSKEWL_WEDGE_EXPORT are_presses_paused();

// add 'tiles' tiles to pos,offset
void NOOSKEWL_WEDGE_EXPORT add_tiles(util::Point<int> &pos, util::Point<float> &offset, util::Point<float> tiles);
void NOOSKEWL_WEDGE_EXPORT abs_to_tile(util::Point<float> pos, util::Point<int> &out_tile, util::Point<float> &out_offset);
void NOOSKEWL_WEDGE_EXPORT tile_to_abs(util::Point<int> pos, util::Point<float> offset, util::Point<float> &out_pos);

void NOOSKEWL_WEDGE_EXPORT rumble(float force, int milliseconds);

util::JSON NOOSKEWL_WEDGE_EXPORT *load_savegame(std::string filename);

void NOOSKEWL_WEDGE_EXPORT quit_all();

}

#endif // NOOSKEWL_WEDGE_GENERAL_H
