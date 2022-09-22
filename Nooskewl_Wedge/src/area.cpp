#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/chest.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/npc.h"
#include "Nooskewl_Wedge/player_input.h"
#include "Nooskewl_Wedge/systems.h"
#include "Nooskewl_Wedge/tile_movement.h"

using namespace wedge;

namespace wedge {

bool entity_y_compare(Map_Entity *a, Map_Entity *b)
{
	if (a->get_offset() == b->get_offset() && a->get_position() == b->get_position()) {
		int a_index = -1;
		int b_index = -1;
		std::vector<Map_Entity *> players = AREA->get_players();
		for (size_t i = 0; i < players.size(); i++) {
			if (players[i] == a) {
				a_index = (int)i;
			}
			else if (players[i] == b) {
				b_index = (int)i;
			}
		}
		if (a_index >= 0 && b_index >= 0) {
			return a_index > b_index;
		}
	}
	return (a->get_offset().y + a->get_position().y) < (b->get_offset().y + b->get_position().y);
}

Area::Area(std::string name) :
	name(name),
	tilemap(NULL),
	done(false),
	hooks(NULL),
	json(NULL)
{
	overlay_colour.a = 0;
}

Area::Area(util::JSON::Node *json) :
	tilemap(NULL),
	done(false),
	hooks(NULL)
{
	overlay_colour.a = 0;
	name = util::JSON::trim_quotes(json->key);
	this->json = json;
}

Area::~Area()
{
	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
		delete *it;
	}

	delete tilemap;
	delete hooks;

	AREA->remove_system(entity_movement_system);
	delete entity_movement_system;
}

bool Area::start()
{
	entity_movement_system = new System(AREA);
	AREA->get_systems().push_back(entity_movement_system);

	new_game = (name == "--start--");
	loaded = json != NULL;

	bool is_initial_load = AREA->get_players().size() == 0;

	if (new_game) {
		name = "start";
	}

	tilemap = new gfx::Tilemap(name + ".map");

	if (new_game) {
		for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
			Map_Entity *player = AREA->create_player(INSTANCE->stats[i].name);
			AREA->set_player((int)i, player);
			player->start(this);
			player->set_sprite(new gfx::Sprite("eny"));
			player->set_position(globals->player_start_positions[i]);
			player->set_direction(globals->player_start_directions[i], true, false);
			entities.push_back(player);
		}
	}
	else if (loaded) {
		util::JSON::Node *entities = json->find("\"entities\"");

		for (size_t i = 0; i < entities->children.size(); i++) {
			util::JSON::Node *node = entities->children[i];
			Map_Entity *entity;
			std::string type;
			util::JSON::Node *type_node = node->find("\"type\"");
			if (type_node != NULL) {
				type = util::JSON::trim_quotes(type_node->value);
			}
			if ((entity = AREA->create_entity(type, node)) == NULL) {
				if (type == "chest") {
					entity = new Chest(node);
				}
				else if (type == "npc") {
					entity = new NPC(node);
				}
				else {
					entity = new Map_Entity(node);
				}
			}
			entity->start(this);
			this->entities.push_back(entity);
			for (size_t i = 0; i < MAX_PARTY; i++) {
				if (entity->get_name() == INSTANCE->stats[i].name) {
					AREA->set_player((int)i, entity);
				}
			}
		}

		util::JSON::Node *n = json->find("\"hook\"");
		if (n != NULL) {
			hook_save = util::JSON::trim_quotes(n->value);
		}
	}
	else { // changed areas
	}

	if (is_initial_load) {
		// don't use this generic one for player...
		Map_Entity_Input_Step *meis = AREA->get_player(0)->get_input_step();
		if (meis) {
			meis->die();
		}

		// ... use a Player_Input_System
		Task *new_task = new Task(entity_movement_system);
		Player_Input_Step *step = new Player_Input_Step(AREA->get_player(0), new_task);
		AREA->get_player(0)->set_input_step(step);
		ADD_STEP(step)
		entity_movement_system->get_tasks().push_back(new_task);
	}
	else {
		Map_Entity_Input_Step *meis = AREA->get_player(0)->get_input_step();
		if (meis) {
			meis->reset(entity_movement_system);
		}

		// other players' input step dies in the destructions of steps in each level, so recreate it
		std::vector<Map_Entity *> players = AREA->get_players();
		for (size_t i = 1; i < players.size(); i++) {
			Task *new_task = new Task(entity_movement_system);
			Map_Entity_Input_Step *step = new Map_Entity_Input_Step(players[i], new_task);
			players[i]->set_input_step(step);
			ADD_STEP(step)
			entity_movement_system->get_tasks().push_back(new_task);
		}
	}

	order_player_input_steps();

	return true;
}

void Area::set_hooks(Area_Hooks *hooks)
{
	this->hooks = hooks;

	if (hooks) {
		hooks->start(new_game, loaded, hook_save);
		hooks->set_animated_tiles();
	}
}

Area_Hooks *Area::get_hooks()
{
	return hooks;
}

std::string Area::get_name()
{
	return name;
}

gfx::Tilemap *Area::get_tilemap()
{
	return tilemap;
}

std::list<Map_Entity *> &Area::get_entities()
{
	return entities;
}

void Area::draw(util::Point<float> map_offset, Layer_Spec spec)
{
	int nlayers = tilemap->get_num_layers();
	int middle_layer = get_middle_layer();

	if (spec == BELOW) {
		nlayers = middle_layer + 1;
	}
	
	entities.sort(entity_y_compare);

	std::list<int> entity_layers;
	std::vector<int> pre_draw_layers;
	std::vector<int> post_draw_layers;
	std::list<int> all_used_layers;
	
	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *entity = *it;
		entity_layers.push_back(entity->get_layer());
#ifdef VERBOSE
		util::debugmsg("entity %s on layer %d\n", entity->get_name().c_str(), entity->get_layer());
#endif
	}

	entity_layers.sort();
	entity_layers.unique();

	if (hooks) {
		pre_draw_layers = hooks->get_pre_draw_layers();
		std::sort(pre_draw_layers.begin(), pre_draw_layers.end());
		std::unique(pre_draw_layers.begin(), pre_draw_layers.end());
		post_draw_layers = hooks->get_post_draw_layers();
		std::sort(post_draw_layers.begin(), post_draw_layers.end());
		std::unique(post_draw_layers.begin(), post_draw_layers.end());
	}

	for (std::list<int>::iterator it = entity_layers.begin(); it != entity_layers.end(); it++) {
		all_used_layers.push_back(*it);
	}
	for (size_t i = 0; i < pre_draw_layers.size(); i++) {
		all_used_layers.push_back(pre_draw_layers[i]);
#ifdef VERBOSE
		util::debugmsg("pre draw on %d\n", pre_draw_layers[i]);
#endif
	}
	for (size_t i = 0; i < post_draw_layers.size(); i++) {
		all_used_layers.push_back(post_draw_layers[i]);
#ifdef VERBOSE
		util::debugmsg("post draw on %d\n", post_draw_layers[i]);
#endif
	}
	all_used_layers.sort();
	all_used_layers.unique();
		
	int current_tile_layer = spec == ABOVE ? middle_layer + 1 : 0;
	int start_layer = current_tile_layer-1;
	int next_layer = current_tile_layer-1;

	std::list<int>::iterator it = all_used_layers.begin();
	while (it != all_used_layers.end() && *it < current_tile_layer) {
		it++;
	}

	if (it != all_used_layers.end()) {
		start_layer = *it;
		it++;
		if (it != all_used_layers.end()) {
			next_layer = *it;
			it++;
		}
	}

#ifdef VERBOSE
	util::debugmsg("begin: start_layer=%d\n", start_layer);
#endif

	if (start_layer == -1) {
		tilemap->draw(current_tile_layer, nlayers-1, map_offset, true);
	}
	else {
		while (true) {
			if (std::find(pre_draw_layers.begin(), pre_draw_layers.end(), start_layer) == pre_draw_layers.end()) {
				tilemap->draw(current_tile_layer, start_layer, map_offset, true);
			}
			else {
				tilemap->draw(current_tile_layer, start_layer-1, map_offset, true);
				hooks->pre_draw(start_layer, map_offset);
				tilemap->draw(start_layer, map_offset, true);
			}
			current_tile_layer = start_layer + 1;
			if (std::find(entity_layers.begin(), entity_layers.end(), start_layer) != entity_layers.end()) {
				for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
					Map_Entity *entity = *it;
					if (entity->get_layer() == start_layer) {
						entity->draw(map_offset);
					}
				}
			}
			if (std::find(post_draw_layers.begin(), post_draw_layers.end(), start_layer) != post_draw_layers.end()) {
				hooks->post_draw(start_layer, map_offset);
			}
			if (next_layer == -1) {
				if (start_layer < nlayers-1) {
					tilemap->draw(start_layer+1, nlayers-1, map_offset, true);
				}
				break;
			}
			start_layer = next_layer;
			if (it != all_used_layers.end()) {
				next_layer = *it;
				it++;
			}
			else {
				next_layer = -1;
			}
		}
	}

	if (overlay_colour.a != 0) {
		gfx::draw_filled_rectangle(overlay_colour, util::Point<int>(0, 0), shim::screen_size);
	}
}

void Area::draw(Layer_Spec spec)
{
	Map_Entity *player = AREA->get_player(0);
	util::Point<int> pos = player->get_position();
	util::Point<float> offset = player->get_offset();
	util::Size<int> player_size = player->get_size();
	util::Point<float> sz(player_size.w / 2.0f, 1.0f - player_size.h / 2.0f);
	add_tiles(pos, offset, sz);
	util::Point<float> map_offset = get_centred_offset(pos, offset, true);
	draw(map_offset, spec);
}

std::string Area::get_next_area_name()
{
	return next_area_name;
}

std::vector< util::Point<int> > Area::get_next_area_positions()
{
	return next_area_positions;
}

std::vector<Direction> Area::get_next_area_directions()
{
	return next_area_directions;
}

bool Area::get_next_area_scrolls_in()
{
	return next_area_scrolls_in;
}

bool Area::exit_gameplay()
{
	return done;
}

bool Area::on_tile(Map_Entity *entity)
{
	if (hooks) {
		return hooks->on_tile(entity);
	}
	else {
		return false;
	}
}

bool Area::try_tile(Map_Entity *entity, util::Point<int> tile_pos)
{
	if (hooks) {
		return hooks->try_tile(entity, tile_pos);
	}
	else {
		return false;
	}
}

void Area::set_next_area(std::string name, util::Point<int> position, Direction direction)
{
	next_area_name = name;

	next_area_positions.clear();
	next_area_directions.clear();

	next_area_positions.push_back(position);
	next_area_directions.push_back(direction);

	next_area_scrolls_in = true;

	entity_movement_system->set_paused(AREA->get_pause_entity_movement_on_next_area_change());
}

void Area::set_next_area(std::string name, std::vector< util::Point<int> > player_positions, std::vector<Direction> directions)
{
	next_area_name = name;
	next_area_positions = player_positions;
	next_area_directions = directions;
	next_area_scrolls_in = false;
	entity_movement_system->set_paused(AREA->get_pause_entity_movement_on_next_area_change());
}

util::Point<float> Area::get_centred_offset(util::Point<int> entity_position, util::Point<float> entity_offset, bool clamp_edges)
{
	util::Point<float> entity_pos;
	util::Point<float> centre;
	util::Point<float> map_offset;

	entity_pos = (entity_offset + entity_position) * shim::tile_size;

	util::Size<float> half_screen = shim::screen_size;
	half_screen /= 2.0f;

	centre = -entity_pos + half_screen;

	util::Size<int> tilemap_size = tilemap->get_size() * shim::tile_size;

	if (clamp_edges == false) {
		map_offset.x = centre.x;
	}
	else {
		if (tilemap_size.w < shim::screen_size.w) {
			map_offset.x = (shim::screen_size.w - tilemap_size.w) / 2.0f;
		}
		else {
			if (centre.x > 0.0f) {
				map_offset.x = 0.0f;
			}
			else if (entity_pos.x + half_screen.w > tilemap_size.w) {
				map_offset.x = (float)-(tilemap_size.w - shim::screen_size.w);
			}
			else {
				map_offset.x = centre.x;
			}
		}
	}

	if (clamp_edges == false) {
		map_offset.y = centre.y;
	}
	else {
		if (tilemap_size.h < shim::screen_size.h) {
			map_offset.y = (shim::screen_size.h - tilemap_size.h) / 2.0f;
		}
		else {
			if (centre.y > 0.0f) {
				map_offset.y = 0.0f;
			}
			else if (entity_pos.y + half_screen.h > tilemap_size.h) {
				map_offset.y = (float)-(tilemap_size.h - shim::screen_size.h);
			}
			else {
				map_offset.y = centre.y;
			}
		}
	}

	return map_offset;
}

System *Area::get_entity_movement_system()
{
	return entity_movement_system;
}

void Area::remove_entity(Map_Entity *entity, bool destroy)
{
	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *e = *it;
		if (e == entity) {
			if (hooks) {
				hooks->remove_entity(entity);
			}
			entities.erase(it);
			if (destroy) {
				delete entity;
			}
			return;
		}
	}
}

void Area::started()
{
	if (hooks) {
		hooks->set_player_start_zones();
		hooks->started();
	}
}

void Area::end()
{
	if (hooks) {
		hooks->end();
	}
}

Map_Entity *Area::entity_on_tile(util::Point<int> pos)
{
	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *entity = *it;
		util::Size<int> size = entity->get_size();
		util::Rectangle<int> rectangle(entity->get_position()-util::Point<int>(0, size.h-1), size);
		if (rectangle.contains(pos)) {
			return entity;
		}
	}

	return NULL;
}

std::vector< util::Rectangle<int> > Area::get_entity_solids(std::vector<Map_Entity *> ignore)
{
	std::vector< util::Rectangle<int> > solids;

	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *entity = *it;
		if (std::find(ignore.begin(), ignore.end(), entity) != ignore.end()) {
			continue;
		}
		if (entity->is_solid() == false) {
			continue;
		}
		util::Size<int> size = entity->get_size();
		solids.push_back(util::Rectangle<int>(entity->get_position()-util::Point<int>(0, size.h-1), size));
	}

	return solids;
}

void Area::activate_with(Map_Entity *entity)
{
	util::Point<int> pos = add_direction(entity->get_position(), entity->get_direction(), 1);

	Map_Entity *activated = NULL;

	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *e = *it;
		util::Point<int> epos = e->get_position();
		util::Size<int> esz = e->get_size();
		if (pos.x >= epos.x && pos.y >= epos.y-(esz.h-1) && pos.x < epos.x+esz.w && pos.y <= epos.y) {
			activated = e;
			break;
		}
	}

	if (activated) {
		if (hooks == NULL || hooks->activate(entity, activated) == false) {
			activated->activate(entity);
		}
	}

	if (hooks) {
		hooks->activate_with(entity);
	}
}

std::string Area::save(bool save_players)
{
	std::string s;
	s += "{";
	s += util::string_printf("\"entities\": {");

	int count = 0;

	std::vector<Map_Entity *> players = AREA->get_players();

	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *entity = *it;
		bool is_player = std::find(players.begin(), players.end(), entity) != players.end();
		if (save_players == false && is_player) {
			continue;
		}
		s += "\"" + entity->get_name() + "\": {" + entity->save() + "}" + (count == (int)entities.size()-1 ? "" : ",");
		count++;
	}

	s += util::string_printf("},");
	s += "\"hook\": \"" + (hooks ? hooks->save() : "") + "\"";
	s += "}";
	return s;
}

Map_Entity *Area::find_entity(std::string name)
{
	for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *entity = *it;
		if (entity->get_name() == name) {
			return entity;
		}
	}
	return NULL;
}

void Area::run()
{
	if (hooks != NULL) {
		hooks->run();
	}
}
	
void Area::lost_device()
{
	if (hooks != NULL) {
		hooks->lost_device();
	}
}

void Area::found_device()
{
	if (hooks != NULL) {
		hooks->found_device();
	}
}
	
void Area::resize(util::Size<int> new_size)
{
	if (hooks != NULL) {
		hooks->resize(new_size);
	}
}

Battle_Game *Area::get_random_battle()
{
	if (hooks) {
		return hooks->get_random_battle();
	}
	else {
		return NULL;
	}
}

bool Area::can_save()
{
	if (hooks) {
		return hooks->can_save();
	}
	else {
		return false;
	}
}

void Area::disconnect_player_input_step()
{
	Map_Entity_Input_Step *input_step = AREA->get_player(0)->get_input_step();
	if (input_step != NULL) {
		input_step->get_task()->remove_step(input_step);
	}
}

void Area::dialogue_done(Map_Entity *entity)
{
	if (hooks) {
		hooks->dialogue_done(entity);
	}
}

bool Area::has_battles()
{
	if (hooks) {
		return hooks->has_battles();
	}
	else {
		return false;
	}
}

void Area::battle_ended(Battle_Game *battle)
{
	if (hooks) {
		hooks->battle_ended(battle);
	}
}

void Area::set_overlay_colour(SDL_Colour colour)
{
	overlay_colour = colour;
}

void Area::set_entities_standing()
{
	// change walking sprites to standing so they're not walking in place on fade and not starting in walking animation on level reload
	Map_Entity_List::iterator it;
	for (it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *entity = *it;
		gfx::Sprite *sprite = entity->get_sprite();
		if (sprite != NULL) {
			if (sprite->get_animation().substr(0, 4) == "walk") {
				entity->set_direction(entity->get_direction(), true, false);
			}
		}
	}
}

void Area::handle_event(TGUI_Event *event)
{
	if (hooks) {
		hooks->handle_event(event);
	}
}

void Area::order_player_input_steps()
{
	// Players have to be in order to avoid jittery walk
	// This function should be called before pretty much everything else
	std::vector<Map_Entity *> players = AREA->get_players();
	// put all the Input_Steps at the back...
	for (size_t i = 0; i < players.size(); i++) {
		std::list<Task *> &tasks = entity_movement_system->get_tasks();
		for (Task_List::iterator it = tasks.begin(); it != tasks.end(); it++) {
			Task *task = *it;
			Step *step = *task->get_steps().begin();
			Map_Entity_Input_Step *meis = players[i]->get_input_step();
			if (step == meis) {
				tasks.erase(it);
				tasks.push_back(task);
				break;
			}
		}
	}
	// Followed by all the Movement_Steps
	for (size_t i = 0; i < players.size(); i++) {
		std::list<Task *> &tasks = entity_movement_system->get_tasks();
		for (Task_List::iterator it = tasks.begin(); it != tasks.end(); it++) {
			Task *task = *it;
			Step *step = *task->get_steps().begin();
			Map_Entity_Input_Step *meis = players[i]->get_input_step();
			if (meis != NULL && step == meis->get_movement_step()) {
				tasks.erase(it);
				tasks.push_back(task);
				break;
			}
		}
	}
}

void Area::add_entity(Map_Entity *entity)
{
	entities.push_back(entity);
	for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
		if (INSTANCE->stats[i].name == entity->get_name()) {
			order_player_input_steps();
			break;
		}
	}
}

std::vector<util::A_Star::Way_Point> Area::get_way_points(util::Point<int> from)
{
	if (hooks) {
		return hooks->get_way_points(from);
	}
	std::vector<util::A_Star::Way_Point> v;
	return v; // nothing
}

bool Area::is_corner_portal(util::Point<int> pos)
{
	if (hooks) {
		return hooks->is_corner_portal(pos);
	}
	return false;
}

bool Area::is_new_game()
{
	return new_game;
}

bool Area::was_loaded()
{
	return loaded;
}

int Area::get_middle_layer()
{
	int nlayers = tilemap->get_num_layers();
	return (nlayers-1)/2;
}

//--

Area_Hooks::Area_Hooks(Area *area) :
	area(area),
	ignore_next_on_tile(false),
	rand_battle_last(-1)
{
}

Area_Hooks::~Area_Hooks()
{
}

bool Area_Hooks::start(bool new_game, bool loaded, std::string save)
{
	return true;
}

void Area_Hooks::started()
{
}

void Area_Hooks::end()
{
}

bool Area_Hooks::on_tile(Map_Entity *entity)
{
	if (entity != AREA->get_player(0)) {
		return false;
	}

	util::Point<int> pos = entity->get_position();

	std::vector<Scroll_Zone *> zones = get_scroll_zones(pos);
	Scroll_Zone *activated = NULL;
	// Remove any player_start_scroll_zones not stepped on anymore (eg if scrolled in on a corner then moved)
	for (std::vector<Scroll_Zone *>::iterator it = player_start_scroll_zones.begin(); it != player_start_scroll_zones.end();) {
		Scroll_Zone *z = *it;
		if (std::find(zones.begin(), zones.end(), z) == zones.end()) {
			it = player_start_scroll_zones.erase(it);
		}
		else {
			it++;
		}
	}
	for (size_t i = 0; i < zones.size(); i++) {
		Scroll_Zone *z = zones[i];
		// if player is on initial scroll zone and not facing scroll zone direction, don't scroll
		if (!(z && std::find(player_start_scroll_zones.begin(), player_start_scroll_zones.end(), z) != player_start_scroll_zones.end() && z->direction != entity->get_direction())) {
			activated = z;
			break;
		}
	}
	
	Fade_Zone *z2 = get_fade_zone(pos);

	util::Point<int> path_goal = entity->get_input_step()->get_path_goal();

	if (path_goal.x >= 0 && path_goal != pos && get_fade_zone(path_goal) != z2 && is_corner_portal(pos)) {
		z2 = NULL; // don't change areas here if it's just a node on a different path
	}
	
	if (z2 == NULL || z2 != player_start_fade_zone) {
		player_start_fade_zone = NULL;
	}

	if (zones.size() > 0 && activated == NULL && z2 == NULL) {
		return false;
	}

	player_start_scroll_zones.clear();
	
	if (z2 && z2 == player_start_fade_zone && activated == NULL) {
		return false;
	}

	if (activated != NULL) {
		area->set_next_area(activated->area_name, util::Point<int>(pos.x-activated->zone.pos.x+activated->topleft_dest.x, pos.y-activated->zone.pos.y+activated->topleft_dest.y), activated->direction);
		return true;
	}

	if (z2) {
		// Redrawing here avoids a little jerk as you step on a tile leading to another area
		if (AREA->get_pause_entity_movement_on_next_area_change()) {
			Map_Entity *player0 = AREA->get_player(0);
			Map_Entity_List &entities = area->get_entities();
			for (Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
				Map_Entity *e = *it;
				if (e == player0) {
					continue;
				}
				Map_Entity_Input_Step *meis = e->get_input_step();
				if (meis) {
					Tile_Movement_Step *tms = meis->get_movement_step();
					if (tms) {
						tms->die();
					}
					meis->die();
					e->set_input_step(NULL); // So die() isn't called again in destructor
				}
			}
			area->set_entities_standing();
		}
		shim::user_render();
		area->set_next_area(z2->area_name, z2->player_positions, z2->directions);
		return true;
	}

	return false;
}

bool Area_Hooks::try_tile(Map_Entity *entity, util::Point<int> tile_pos)
{
	return false;
}

bool Area_Hooks::activate(Map_Entity *activator, Map_Entity *activated)
{
	return false;
}

bool Area_Hooks::activate_with(Map_Entity *activator)
{
	return false;
}

void Area_Hooks::pre_draw(int layer, util::Point<float> map_offset)
{
}

void Area_Hooks::post_draw(int layer, util::Point<float> map_offset)
{
}

std::vector<int> Area_Hooks::get_pre_draw_layers()
{
	std::vector<int> v;
	return v;
}

std::vector<int> Area_Hooks::get_post_draw_layers()
{
	std::vector<int> v;
	return v;
}

void Area_Hooks::remove_entity(Map_Entity *entity)
{
}

void Area_Hooks::lost_device()
{
}

void Area_Hooks::found_device()
{
}

std::string Area_Hooks::save()
{
	return "";
}

bool Area_Hooks::has_battles()
{
	return false;
}

Battle_Game *Area_Hooks::get_random_battle()
{
	return NULL;
}

bool Area_Hooks::can_save()
{
	return false;
}

void Area_Hooks::dialogue_done(Map_Entity *entity)
{
}

std::vector<Area_Hooks::Scroll_Zone *> Area_Hooks::get_scroll_zones(util::Point<int> pos)
{
	std::vector<Scroll_Zone *> zones;

	for (size_t i = 0; i < scroll_zones.size(); i++) {
		Scroll_Zone &z = scroll_zones[i];

		if (pos.x >= z.zone.pos.x && pos.x < z.zone.pos.x+z.zone.size.w && pos.y >= z.zone.pos.y && pos.y < z.zone.pos.y+z.zone.size.h) {
			zones.push_back(&z);
		}
	}

	return zones;
}

Area_Hooks::Fade_Zone *Area_Hooks::get_fade_zone(util::Point<int> pos)
{
	for (size_t i = 0; i < fade_zones.size(); i++) {
		Fade_Zone &z = fade_zones[i];

		if (pos.x >= z.zone.pos.x && pos.x < z.zone.pos.x+z.zone.size.w && pos.y >= z.zone.pos.y && pos.y < z.zone.pos.y+z.zone.size.h) {
			return &z;
		}
	}

	return NULL;
}

void Area_Hooks::insert_rand_battle_type(int type)
{
	int start = (type == rand_battle_last ? (rand_battle_table[0] == type ? 2 : 0) : 0);
	int pos = util::rand(start, (int)rand_battle_table.size());
	rand_battle_table.insert(rand_battle_table.begin() + pos, type);
}

void Area_Hooks::gen_rand_battle_table(int num_types)
{
	rand_battle_table.clear();

	for (int i = 0; i < num_types; i++) {
		if (i != rand_battle_last) {
			gen_rand_battle_type(i);
		}
	}

	if (rand_battle_last >= 0) {
		gen_rand_battle_type(rand_battle_last);
	}

	rand_battle_last = rand_battle_table.back();
}

void Area_Hooks::gen_rand_battle_type(int type)
{
	int n = util::rand(1, 2);
	for (int j = 0; j < n; j++) {
		insert_rand_battle_type(type);
	}
}

void Area_Hooks::set_animated_tiles()
{
}

void Area_Hooks::run()
{
}

void Area_Hooks::resize(util::Size<int> new_size)
{
}

void Area_Hooks::handle_event(TGUI_Event *event)
{
}

void Area_Hooks::set_player_start_zones()
{
	Map_Entity *player = AREA->get_player(0);
	util::Point<int> pos = player->get_position();
	player_start_scroll_zones = get_scroll_zones(pos);
	player_start_fade_zone = get_fade_zone(pos);
}

void Area_Hooks::battle_ended(Battle_Game *battle)
{
}

std::vector<util::A_Star::Way_Point> Area_Hooks::get_way_points(util::Point<int> from)
{
	std::vector<util::A_Star::Way_Point> v;
	return v; // nothing by default
}

bool Area_Hooks::is_corner_portal(util::Point<int> pos)
{
	return false;
}

}
