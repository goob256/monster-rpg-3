#include <zlib.h>

#ifdef TVOS
#include <Nooskewl_Shim/ios.h>
#endif

#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/battle_game.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/omnipresent.h"
#include "Nooskewl_Wedge/player_input.h"
#include "Nooskewl_Wedge/systems.h"

using namespace wedge;

namespace wedge {

util::Point<int> mouse_position(-1, -1);

util::Point<int> add_direction(util::Point<int> pos, Direction direction, int n)
{
	switch (direction) {
		case DIR_N:
			pos.y -= n;
			break;
		case DIR_E:
			pos.x += n;
			break;
		case DIR_S:
			pos.y += n;
			break;
		case DIR_W:
			pos.x -= n;
			break;
		default:
			break;
	}

	return pos;
}

Direction direction_from_offset(util::Point<int> offset)
{
	if (offset.x < 0) {
		return DIR_W;
	}
	else if (offset.x > 0) {
		return DIR_E;
	}
	else if (offset.y < 0) {
		return DIR_N;
	}
	else if (offset.y > 0) {
		return DIR_S;
	}
	else {
		return DIR_NONE;
	}
}

std::string integer_point_to_string(util::Point<int> p)
{
	return util::string_printf("{ \"x\": %d, \"y\": %d }", p.x, p.y);
}

std::string float_point_to_string(util::Point<float> p)
{
	return util::string_printf("{ \"x\": %f, \"y\": %f }", p.x, p.y);
}

std::string integer_size_to_string(util::Size<int> s)
{
	return util::string_printf("{ \"w\": %d, \"h\": %d }", s.w, s.h);
}

std::string float_size_to_string(util::Size<float> s)
{
	return util::string_printf("{ \"w\": %f, \"h\": %f }", s.w, s.h);
}

std::string direction_to_string(Direction d)
{
	switch (d) {
		case DIR_N:
			return std::string("n");
		case DIR_E:
			return std::string("e");
		case DIR_S:
			return std::string("s");
		case DIR_W:
			return std::string("w");
		default:
			return "";
	}
}

std::string bool_to_string(bool b)
{
	return util::string_printf("%s", b ? "true" : "false");
}

std::string sprite_to_string(gfx::Sprite *sprite)
{
	std::string json_filename, image_directory;
	sprite->get_filenames(json_filename, image_directory);
	return util::string_printf("{ \"json_filename\": \"%s\", \"image_directory\": \"%s\", \"animation\": \"%s\" }", json_filename.c_str(), image_directory.c_str(), sprite->get_animation().c_str());
}

std::string image_to_string(gfx::Image *image)
{
	return util::string_printf("{ \"filename\": \"%s\" }", image->filename.c_str());
}

util::Point<int> json_to_integer_point(util::JSON::Node *json)
{
	util::Point<int> p;
	util::JSON::Node *n;
	n = json->find("\"x\"");
	if (n != NULL) {
		p.x = atoi(n->value.c_str());
	}
	n = json->find("\"y\"");
	if (n != NULL) {
		p.y = atoi(n->value.c_str());
	}
	return p;
}

util::Point<float> json_to_float_point(util::JSON::Node *json)
{
	util::Point<float> p;
	util::JSON::Node *n;
	n = json->find("\"x\"");
	if (n != NULL) {
		p.x = atof(n->value.c_str());
	}
	n = json->find("\"y\"");
	if (n != NULL) {
		p.y = atof(n->value.c_str());
	}
	return p;
}

util::Size<int> json_to_integer_size(util::JSON::Node *json)
{
	util::Size<int> s;
	util::JSON::Node *n;
	n = json->find("\"w\"");
	if (n != NULL) {
		s.w = atoi(n->value.c_str());
	}
	n = json->find("\"h\"");
	if (n != NULL) {
		s.h = atoi(n->value.c_str());
	}
	return s;
}

util::Size<float> json_to_float_size(util::JSON::Node *json)
{
	util::Size<float> s;
	util::JSON::Node *n;
	n = json->find("\"w\"");
	if (n != NULL) {
		s.w = atof(n->value.c_str());
	}
	n = json->find("\"h\"");
	if (n != NULL) {
		s.h = atof(n->value.c_str());
	}
	return s;
}

bool json_to_bool(util::JSON::Node *json)
{
	if (json->value == "true") {
		return true;
	}
	else {
		return false;
	}
}

gfx::Sprite *json_to_sprite(util::JSON::Node *json)
{
	std::string json_filename, image_directory, animation;
	util::JSON::Node *n;
	n = json->find("\"json_filename\"");
	if (n) {
		json_filename = util::JSON::trim_quotes(n->value);
	}
	n = json->find("\"image_directory\"");
	if (n) {
		image_directory = util::JSON::trim_quotes(n->value);
	}
	n = json->find("\"animation\"");
	if (n) {
		animation = util::JSON::trim_quotes(n->value);
	}
	gfx::Sprite *sprite = new gfx::Sprite(json_filename, image_directory, true);
	sprite->set_animation(animation);
	return sprite;
}

int json_to_integer(util::JSON::Node *json)
{
	return atoi(json->value.c_str());
}

gfx::Image *json_to_image(util::JSON::Node *json)
{
	util::JSON::Node *n = json->find("\"filename\"");
	if (n == NULL) {
		return NULL;
	}
	return new gfx::Image(util::JSON::trim_quotes(n->value), true);
}

std::string save()
{
	Area *area = AREA->get_current_area();

	area->set_entities_standing();

	std::string s;
	s += "{";
	s += INSTANCE->save() + ",";
	s += "\"areas\": {";
	std::string current_area_name = area->get_name();
	s += "\"" + current_area_name + "\": ";
	s += area->save(true);
	s += ",";
	std::map<std::string, std::string>::iterator it;
	int count = 0;
	int num_to_save = (int)INSTANCE->saved_levels.size()-1; // -1 for current area already saved
	for (it = INSTANCE->saved_levels.begin(); it != INSTANCE->saved_levels.end(); it++) {
		std::pair<std::string, std::string> p = *it;
		if (p.first != current_area_name) {
			s += p.second;
			if (count < num_to_save-1) {
				s += ",";
			}
			count++;
		}
	}
	s += "}";
	s += "}";

	return s;
}

// s is the JSON as a string
bool save(std::string s, std::string filename)
{
#ifdef TVOS
	return util::tvos_save_file(filename, s);
#else
	SDL_RWops *file = SDL_RWFromFile(filename.c_str(), "wb");

	if (file) {
		SDL_RWops *memfile = SDL_RWFromMem((void *)s.c_str(), (int)s.length());
		util::JSON *json = new util::JSON(memfile);
		s = json->get_root()->to_string(0);
		delete json;
		SDL_RWclose(memfile);
		uLongf needed = compressBound(s.length());
		Bytef *tmp = new Bytef[needed];
		bool ret;
		if (compress(tmp, &needed, (const Bytef *)s.c_str(), s.length()) == Z_OK) {
			SDL_WriteLE32(file, (int)s.length());
			SDL_RWwrite(file, tmp, needed, 1);
			ret = true;
		}
		else {
			ret = false;
		}
		delete[] tmp;
		SDL_RWclose(file);
		utime(filename.c_str(), NULL); // touch the file (for cloud syncing create time
		return ret;
	}
	else {
		return false;
	}
#endif
}

bool save(std::string filename)
{
	std::string s = save();
	return save(s, filename);
}

bool save_play_time(std::string filename)
{
	util::JSON *json;
	try {
		json = load_savegame(filename);
	}
	catch (util::FileNotFoundError e) {
		return false;
	}
	if (json == NULL) {
		util::errormsg("Error loading save %s.\n", filename.c_str());
		return false;
	}
	util::JSON::Node *root = json->get_root();
	if (root == NULL) {
		util::errormsg("JSON has no root.\n");
		delete json;
		return false;
	}
	util::JSON::Node *cfg = root->find("\"game\"");
	if (cfg == NULL) {
		util::errormsg("Save has no game section.\n");
		delete json;
		return false;
	}
	util::JSON::Node *n;
	n = cfg->find("\"play_time\"");
	if (n == NULL) {
		util::errormsg("Save has no play_time.\n");
		delete json;
		return false;
	}

	int t = atoi(n->value.c_str());
	Uint32 now = GET_TICKS();
	Uint32 played_time = now - INSTANCE->play_start;
	t += (played_time / 1000);
	INSTANCE->play_start = now;
	n->value = util::itos(t);

	std::string s = root->to_string(0);

	delete json;

	return save(s, filename);
}

util::Point<int> get_mouse_position()
{
	return mouse_position;
}

void pause_player_input(bool paused)
{
	Map_Entity_Input_Step *meis = AREA->get_player(0)->get_input_step();
	if (meis != NULL) {
		Player_Input_Step *pis = dynamic_cast<Player_Input_Step *>(meis);
		if (pis) {
			pis->set_input_paused(paused);
			if (paused) {
				pis->clear(); // hack? for fishing, otherwise she looks North when done
			}
		}
	}
}

void pause_presses(bool paused, bool repeat_pressed)
{
	Map_Entity *player = AREA->get_player(0);
	Map_Entity_Input_Step *meis = player->get_input_step();
	meis->pause_presses(paused);
	if (paused == false && repeat_pressed) {
		meis->repeat_pressed();
	}
}

bool are_presses_paused()
{
	Map_Entity *player = AREA->get_player(0);
	Map_Entity_Input_Step *meis = player->get_input_step();
	return meis->are_presses_paused();
}

void add_tiles(util::Point<int> &pos, util::Point<float> &offset, util::Point<float> tiles)
{
	// add half size to centre correctly
	offset += tiles;
	int x = abs((int)offset.x) * util::sign(offset.x);
	int y = abs((int)offset.y) * util::sign(offset.y);
	pos.x += x;
	pos.y += y;
	offset.x -= x;
	offset.y -= y;
}

void abs_to_tile(util::Point<float> pos, util::Point<int> &out_tile, util::Point<float> &out_offset)
{
	out_tile = pos / shim::tile_size;
	pos -= out_tile * shim::tile_size;
	out_offset = pos / shim::tile_size;
}

void tile_to_abs(util::Point<int> pos, util::Point<float> offset, util::Point<float> &out_pos)
{
	out_pos = pos * shim::tile_size;
	out_pos += offset * shim::tile_size;
}

void rumble(float force, int milliseconds)
{
	if (globals->rumble_enabled) {
		input::rumble(force, milliseconds);
	}
}

util::JSON *load_savegame(std::string filename)
{
#ifdef TVOS
	std::string savegame;
	util::JSON *json;
	if (util::tvos_read_file(filename, savegame)) {
		SDL_RWops *memfile = SDL_RWFromMem((void *)savegame.c_str(), (int)savegame.length());
		json = new util::JSON(memfile);
		SDL_RWclose(memfile);
		return json;
	}
	else {
		throw util::FileNotFoundError(filename + " not found!");
	}
	return json;
#else
	SDL_RWops *file = SDL_RWFromFile(filename.c_str(), "rb");
	if (file == NULL) {
		throw util::FileNotFoundError(filename + "not found!");
	}
	SDL_RWseek(file, 0, RW_SEEK_END);
	int compressed_sz = (int)SDL_RWtell(file) - 4; // 4 for size
	SDL_RWseek(file, 0, RW_SEEK_SET);
	uLongf uncompressed_size = SDL_ReadLE32(file);
	Uint8 *bytes;
	try {
		bytes = new Uint8[compressed_sz];
	}
	catch (std::bad_alloc e) {
		SDL_RWclose(file);
		throw util::LoadError("Bad size in savegame!");
	}
	if (SDL_RWread(file, bytes, compressed_sz, 1) != 1) {
		delete[] bytes;
		SDL_RWclose(file);
		throw util::LoadError("SDL_Rwread failed while loading save");
	}
	SDL_RWclose(file);
	char *text = new char[uncompressed_size];
	util::JSON *json;
	if (uncompress((Bytef *)text, &uncompressed_size, (Bytef *)bytes, compressed_sz) == Z_OK) {
		std::string savegame(text, uncompressed_size);
		delete[] bytes;
		delete[] text;
		SDL_RWops *memfile = SDL_RWFromMem((void *)savegame.c_str(), (int)savegame.length());
		json = new util::JSON(memfile);
		SDL_RWclose(memfile);
	}
	else {
		delete[] bytes;
		delete[] text;
		throw util::LoadError("uncompress failed!");
	}

	return json;
#endif
}

void quit_all()
{
	if (BATTLE) {
		delete BATTLE;
		BATTLE = NULL;
	}
	if (MENU) {
		delete MENU;
		MENU = NULL;
	}
	if (SHOP) {
		delete SHOP;
		SHOP = NULL;
	}
	delete AREA;
	AREA = NULL;
	for (size_t i = 0; i < shim::guis.size(); i++) {
		shim::guis[i]->exit();
	}
	OMNIPRESENT->end_fade();
}

}
