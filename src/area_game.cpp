#include <Nooskewl_Wedge/a_star.h>
#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/battle_player.h>
#include <Nooskewl_Wedge/branch.h>
#include <Nooskewl_Wedge/check_positions.h>
#include <Nooskewl_Wedge/chest.h>
#include <Nooskewl_Wedge/delay.h>
#include <Nooskewl_Wedge/delete_map_entity.h>
#include <Nooskewl_Wedge/give_object.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/generic_callback.h>
#include <Nooskewl_Wedge/generic_immediate_callback.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/input.h>
#include <Nooskewl_Wedge/inventory.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/npc.h>
#include <Nooskewl_Wedge/omnipresent.h>
#include <Nooskewl_Wedge/onscreen_controller.h>
#include <Nooskewl_Wedge/pause_presses.h>
#include <Nooskewl_Wedge/pause_sprite.h>
#include <Nooskewl_Wedge/pause_task.h>
#include <Nooskewl_Wedge/play_animation.h>
#include <Nooskewl_Wedge/play_sound.h>
#include <Nooskewl_Wedge/player_input.h>
#include <Nooskewl_Wedge/set_direction.h>
#include <Nooskewl_Wedge/set_integer.h>
#include <Nooskewl_Wedge/set_solid.h>
#include <Nooskewl_Wedge/set_visible.h>
#include <Nooskewl_Wedge/shop_step.h>
#include <Nooskewl_Wedge/slide_entity.h>
#include <Nooskewl_Wedge/stop_sound.h>
#include <Nooskewl_Wedge/wait.h>
#include <Nooskewl_Wedge/wait_for_integer.h>

#include "achievements.h"
#include "area_game.h"
#include "autosave.h"
#include "battles.h"
#include "buy_scroll.h"
#include "captain.h"
#include "custom_slide_entity.h"
#include "dialogue.h"
#include "fishing.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "inn.h"
#include "inventory.h"
#include "jump.h"
#include "menu_game.h"
#include "milestones.h"
#include "monster_rpg_3.h"
#include "revive_entity.h"
#include "sailor.h"
#include "sailor_npc.h"
#include "sailship.h"
#include "save_slot.h"
#include "scroll_help.h"
#include "shop_game.h"
#include "settings.h"
#include "start_battle.h"
#include "stats.h"

struct Reset_Anim {
	bool was_moving;
	wedge::Direction direction;
	wedge::Map_Entity *entity;
};

static void reset_anim(void *data)
{
	Reset_Anim *d = static_cast<Reset_Anim *>(data);
	d->entity->set_direction(d->direction, true, d->was_moving);
	delete d;
}

static void set_following(void *data)
{
	INSTANCE->party_following_player = data != NULL;
}

static void achieve_vampire(void *data)
{
	util::achieve((void *)ACHIEVE_VAMPIRE);
}

STRUCT_ALIGN(Area_Hooks_Monster_RPG_3, 16) : public wedge::Area_Hooks
{
public:
	Area_Hooks_Monster_RPG_3(wedge::Area *area) :
		wedge::Area_Hooks(area),
		work(NULL),
		on_ship(false)
	{
		torch_size = shim::tile_size;
		flicker_size = 1.0f;
		beam_size = shim::tile_size * 2;

		// Add cactus fruit!
		if (area->was_loaded() == false) {
			gfx::Tilemap *tilemap = area->get_tilemap();
			int num_layers = tilemap->get_num_layers();
			util::Size<int> size = tilemap->get_size();

			int count = 0;

			for (int layer = 0; layer < num_layers; layer++) {
				for (int y = 0; y < size.h; y++) {
					for (int x = 0; x < size.w; x++) {
						util::Point<int> tile_xy;
						bool solid;
						if (tilemap->get_tile(layer, util::Point<int>(x, y), tile_xy, solid)) {
							if (tile_xy.x == 8 && tile_xy.y == 11 && util::rand(0, 1) == 0) {
								// it's a big cactus base!
								wedge::Chest *chest = new wedge::Chest("cactus" + util::itos(count++), "cactus_fruit", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_CACTUS_FRUIT, 1));
								chest->start(area);
								chest->set_position(util::Point<int>(x, y));
								chest->set_layer(num_layers-1);
								area->add_entity(chest);
								break;
							}
						}
					}
				}
			}
		}
	}

	virtual ~Area_Hooks_Monster_RPG_3()
	{
		delete work;
	}

	void draw_darkness(util::Point<float> map_offset, float alpha = 1.0f)
	{
		SDL_Colour tint;
		tint.r = 255 * alpha;
		tint.g = 255 * alpha;
		tint.b = 255 * alpha;
		tint.a = 255 * alpha;

		int x = int(darkness_offset1.x) % darkness_image1->size.w;
		int y = int(darkness_offset1.y) % darkness_image1->size.h;
		x -= darkness_image1->size.w;
		y -= darkness_image1->size.h;
		x += map_offset.x;
		y += map_offset.y;
		x = -(-x % darkness_image1->size.w);
		y = -(-y % darkness_image1->size.h);
		int w = shim::screen_size.w / darkness_image1->size.w + 2;
		int h = shim::screen_size.h / darkness_image1->size.h + 2;
		darkness_image1->start_batch();
		for (int yy = 0; yy < h; yy++) {
			for (int xx = 0; xx < w; xx++) {
				darkness_image1->draw_tinted(tint, util::Point<int>(x + xx*darkness_image1->size.w, y + yy*darkness_image1->size.h));
			}
		}
		darkness_image1->end_batch();

		x = int(darkness_offset2.x) % darkness_image2->size.w;
		y = int(darkness_offset2.y) % darkness_image2->size.h;
		x -= darkness_image1->size.w;
		y -= darkness_image1->size.h;
		x += map_offset.x;
		y += map_offset.y;
		x = -(-x % darkness_image2->size.w);
		y = -(-y % darkness_image2->size.h);
		w = shim::screen_size.w / darkness_image2->size.w + 2;
		h = shim::screen_size.h / darkness_image2->size.h + 2;
		darkness_image2->start_batch();
		for (int yy = 0; yy < h; yy++) {
			for (int xx = 0; xx < w; xx++) {
				darkness_image2->draw_tinted(tint, util::Point<int>(x + xx*darkness_image2->size.w, y + yy*darkness_image2->size.h));
			}
		}
		darkness_image2->end_batch();
	}

	void draw_beam(util::Point<float> map_offset, util::Point<float> pos, float size, bool draw_triangles)
	{
		const float alpha = 0.1f;
		SDL_Colour light = shim::palette[12];
		light.r *= alpha;
		light.g *= alpha;
		light.b *= alpha;
		light.a = 255 * alpha;

		gfx::draw_filled_circle(light, pos, size, 6);

		if (draw_triangles == false) {
			return;
		}

		SDL_Colour colours[3];
		colours[0] = light;
		colours[1] = light;
		colours[2] = light;

		util::Point<float> tri1[3];
		util::Point<float> tri2[3];

		tri1[0] = util::Point<float>(pos.x-size/2, map_offset.y);
		tri1[1] = util::Point<float>(pos.x+size/2, map_offset.y);
		tri1[2] = util::Point<float>(pos.x+size, pos.y);

		tri2[0] = util::Point<float>(pos.x-size/2, map_offset.y);
		tri2[1] = util::Point<float>(pos.x+size, pos.y);
		tri2[2] = util::Point<float>(pos.x-size, pos.y);

		gfx::draw_filled_triangle(colours, tri1[0], tri1[1], tri1[2]);
		gfx::draw_filled_triangle(colours, tri2[0], tri2[1], tri2[2]);
	}

	void draw_beams(util::Point<float> map_offset, std::vector< util::Point<float> > beams, float size, bool base_only)
	{
		for (size_t i = 0; i < beams.size(); i++) {
			util::Point<float> pos = map_offset;
			pos += beams[i] * shim::tile_size + util::Point<float>(shim::tile_size/2-0.5f, shim::tile_size/2.0f);
			draw_beam(map_offset, pos, size, base_only == false);
		}
	}

	void draw_torches(SDL_Colour tint, util::Point<float> map_offset, std::vector< util::Point<float> > torches, std::vector< util::Point<float> > beams, bool include_darkness, int pass = 0, bool is_platform_room = false)
	{
		static util::Size<int> work_size(0, 0);

		if (work == NULL || shim::screen_size != work_size) {
			delete work;
			gfx::Image::create_depth_buffer = true;
			work = new gfx::Image(shim::screen_size*FX_SCALE); // using a lower-than real_screen_size buffer makes this much faster
			gfx::Image::create_depth_buffer = false;
			work_size = shim::screen_size; // yeah, not work->size... work needs to be recreated if shim::screen_size changes
		}

		if (pass == 0) {
			gfx::set_target_image(work);
			gfx::set_default_projection(work->size, util::Point<int>(0, 0), FX_SCALE);
			gfx::update_projection();
			gfx::clear(shim::transparent);
		}

		std::vector<float> sizes;
		for (size_t i = 0; i < torches.size(); i++) {
			sizes.push_back(util::rand(0, 1000)/1000.0f * flicker_size * 1.5f + torch_size);
		}

		float bs = beam_size;
		if (pass == 1) {
			bs -= pass * torch_size/4+util::rand(0, 1000)/1000.0f*flicker_size;
		}

		gfx::enable_depth_test(true);
		gfx::enable_depth_write(true);
		gfx::clear_depth_buffer(1.0f);
		gfx::set_depth_mode(gfx::COMPARE_LESS);
		gfx::enable_colour_write(false);
		
		// all this z_add stuff is for masking

		if (is_platform_room) {
			shim::z_add = -0.025f;
			gfx::draw_primitives_start();
			draw_beams(map_offset, beams, bs, false);
			gfx::draw_primitives_end();
			if (pass == 0) {
				gfx::enable_depth_test(false);
				gfx::enable_colour_write(true);
				// draw yellow -> blue (in various stages) across tilemap)
				gfx::Tilemap *tilemap = area->get_tilemap();
				util::Size<int> sz = tilemap->get_size();
				SDL_Colour colour1 = shim::palette[12];
				colour1.r *= 0.1f;
				colour1.g *= 0.1f;
				colour1.b *= 0.1f;
				colour1.a *= 0.1f;
				colour1 = shim::transparent;
				SDL_Colour colour2 = shim::palette[18];
				colour2.r *= 0.1f;
				colour2.g *= 0.1f;
				colour2.b *= 0.1f;
				colour2.a *= 0.1f;
				gfx::draw_filled_rectangle(colour1, map_offset, util::Size<int>(shim::tile_size*30, sz.h*shim::tile_size));
				gfx::draw_filled_rectangle(colour2, map_offset+util::Point<int>(38*shim::tile_size, 0), util::Size<int>((sz.w-38)*shim::tile_size, sz.h*shim::tile_size));
				SDL_Colour gradient[3];
				gradient[0] = colour2;
				gradient[1] = colour1;
				gradient[2] = colour2;
				gfx::draw_filled_triangle(gradient, map_offset+util::Point<int>(shim::tile_size*38, sz.h*shim::tile_size), map_offset+util::Point<int>(shim::tile_size*30, 0), map_offset+util::Point<int>(shim::tile_size*38, 0));
				gradient[0] = colour1;
				gradient[1] = colour1;
				gradient[2] = colour2;
				gfx::draw_filled_triangle(gradient, map_offset+util::Point<int>(shim::tile_size*30, sz.h*shim::tile_size), map_offset+util::Point<int>(shim::tile_size*30, 0), map_offset+util::Point<int>(shim::tile_size*38, sz.h*shim::tile_size));
				gfx::enable_depth_test(true);
				gfx::enable_colour_write(false);
			}
		}
		else {
			shim::z_add = -0.025f;
			gfx::draw_primitives_start();
			draw_beams(map_offset, beams, bs, false);
			gfx::draw_primitives_end();
		}
		
		shim::z_add = -0.05f;
		
		gfx::draw_primitives_start();
		for (size_t i = 0; i < torches.size(); i++) {
			float size = sizes[i];
			size -= pass * torch_size/4+util::rand(0, 1000)/1000.0f*flicker_size;
			gfx::draw_filled_circle(shim::black, map_offset+torches[i]*shim::tile_size+util::Point<float>(shim::tile_size/2-0.5f, shim::tile_size/3), size, 6);
		}
		gfx::draw_primitives_end();

		gfx::set_depth_mode(gfx::COMPARE_LESS);
		gfx::enable_colour_write(true);
		gfx::enable_depth_write(false);
		gfx::enable_depth_test(true);

		shim::z_add = -0.05f;

		if (include_darkness) {
			draw_darkness(map_offset, 0.5f);
		}
		else {
			SDL_Colour dark = shim::black;
			dark.r *= 0.1f;
			dark.g *= 0.1f;
			dark.b *= 0.1f;
			dark.a *= 0.1f;
			dark.r += tint.r * 0.025f;
			dark.g += tint.g * 0.025f;
			dark.b += tint.b * 0.025f;
			dark.a += tint.a * 0.025f;
			gfx::draw_filled_rectangle(dark, util::Point<int>(0, 0), shim::screen_size);
		}
		
		shim::z_add = -0.02f;
		
		gfx::enable_depth_write(true);

		gfx::draw_primitives_start();
		draw_beams(map_offset, beams, bs, false);
		gfx::draw_primitives_end();
		
		shim::z_add = 0.0f;

		if (pass < 1) {
			draw_torches(tint, map_offset, torches, beams, include_darkness, pass+1, is_platform_room);
		}
		else {
			gfx::enable_depth_write(false);
			gfx::enable_depth_test(false);

			gfx::draw_primitives_start();
			draw_beams(map_offset, beams, bs, true);
			gfx::draw_primitives_end();
			gfx::set_depth_mode(gfx::COMPARE_LESSEQUAL);

			gfx::set_target_backbuffer();
		
			work->stretch_region(util::Point<int>(0, 0), work->size, util::Point<int>(0, 0), shim::screen_size);
		}
	}

	void pre_draw(int layer, util::Point<float> map_offset)
	{
		if (on_ship && layer == 0) {
			gfx::get_matrices(old_mv, old_p);
			glm::mat4 mv = old_mv;
			mv = glm::translate(mv, glm::vec3(shim::screen_size.w/2.0f, shim::screen_size.h/2.0f, 0.0f));
			mv = glm::rotate(mv, get_ship_angle(), glm::vec3(0.0f, 0.0f, 1.0f));
			mv = glm::translate(mv, glm::vec3(-shim::screen_size.w/2.0f, -shim::screen_size.h/2.0f, 0.0f));
			gfx::set_matrices(mv, old_p);
			gfx::update_projection();
		}
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (on_ship && layer == area->get_tilemap()->get_num_layers()-1) {
			gfx::set_matrices(old_mv, old_p);
			gfx::update_projection();
		}
	}

	std::vector<int> get_pre_draw_layers()
	{
		std::vector<int> v = wedge::Area_Hooks::get_pre_draw_layers();
		if (on_ship) {
			v.push_back(0);
		}
		return v;
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = wedge::Area_Hooks::get_post_draw_layers();
		if (on_ship) {
			v.push_back(area->get_tilemap()->get_num_layers()-1);
		}
		return v;
	}

	void lost_device()
	{
		delete work;
		work = NULL;
	}

	void found_device()
	{
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks::on_tile(entity)) {
			return true;
		}

		return false;
	}

	void started()
	{
		if (static_cast<Monster_RPG_3_Area_Game *>(AREA)->get_num_areas_created() > 1 && can_autosave()) {
			autosave(true);
		}
	}

	// For 16 byte alignment to make glm::mat4 able to use SIMD
#ifdef _WIN32
	void *operator new(size_t i) { return _mm_malloc(i,16); }
	void operator delete(void* p) { _mm_free(p); }
#endif

protected:
	static const int FX_SCALE;

	float torch_size;
	float flicker_size;
	float beam_size;
	gfx::Image *darkness_image1;
	gfx::Image *darkness_image2;
	gfx::Image *work;
	util::Point<float> darkness_offset1;
	util::Point<float> darkness_offset2;
	bool on_ship; // set true to rock like a boat
	glm::mat4 old_mv, old_p;
};

const int Area_Hooks_Monster_RPG_3::FX_SCALE = 2; // don't know why g++ won't let me just assign it inline

class Area_Hooks_Start : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Start(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Scroll_Zone z;
		z.zone = util::Rectangle<int>(util::Point<int>(7, 0), util::Size<int>(3, 1));
		z.area_name = "fiddler";
		z.topleft_dest = util::Point<int>(8, 19);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);
		
		z.zone = util::Rectangle<int>(util::Point<int>(10, 16), util::Size<int>(5, 1));
		z.area_name = "forest_save";
		z.topleft_dest = util::Point<int>(7, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Start()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/forest.mml");

		if (new_game) {
			wedge::Map_Entity *tiggy = new wedge::Map_Entity("tig");
			tiggy->start(area);
			tiggy->set_position(util::Point<int>(2, 9));
			tiggy->set_sprite(new gfx::Sprite("tiggy"));
			tiggy->set_direction(wedge::DIR_S, true, false);
			tiggy->set_solid(false);
			area->add_entity(tiggy);

			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 3));
			chest->start(area);
			chest->set_position(util::Point<int>(15, 3));
			area->add_entity(chest);

			NEW_SYSTEM_AND_TASK(AREA)
			Dialogue_Step *ds = new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1018)/* Originally: They were up this way! Follow me! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task);
			wedge::A_Star_Step *as = new wedge::A_Star_Step(tiggy, util::Point<int>(8, -1), new_task);
			as->set_allow_out_of_bounds(true);
			wedge::Delete_Map_Entity_Step *dmes = new wedge::Delete_Map_Entity_Step(tiggy, new_task);
			ADD_STEP(ds)
			ADD_STEP(as)
			ADD_STEP(dmes)
			ADD_TASK(new_task)

#ifdef TVOS
			bool joystick_connected = true;
#else
			bool joystick_connected = input::is_joystick_connected();
#endif

			ANOTHER_TASK
			wedge::Wait_Step *wait_step = new wedge::Wait_Step(new_task);
			ds->add_monitor(wait_step);
			wedge::Delay_Step *delay_step = new wedge::Delay_Step(2000, new_task);
			Dialogue_Step *ds2 = NULL;
			if (joystick_connected == false) {
				if (GLOBALS->onscreen_controller_was_enabled) {
					ds2 = new Dialogue_Step("", GLOBALS->game_t->translate(1019)/* Originally: Use the DPAD to move. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1020)/* Originally: Press A to interact with things. */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_AUTO, new_task);
				}
#if defined ANDROID || defined IOS
				else if (util::system_has_touchscreen()) {
					ds2 = new Dialogue_Step("", GLOBALS->game_t->translate(1021)/* Originally: Tap where you want to walk to. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1022)/* Originally: Tap things next to you to interact with them. */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_AUTO, new_task);
				}
#endif
				if (ds2 == NULL) {
				    std::string text;
				    if (GLOBALS->key_l == TGUIK_LEFT && GLOBALS->key_r == TGUIK_RIGHT && GLOBALS->key_u == TGUIK_UP && GLOBALS->key_d == TGUIK_DOWN) {
					text = GLOBALS->game_t->translate(1023)/* Originally: Use the ARROW KEYS to move. */ + NEW_PARAGRAPH;
				    }
				    ds2 = new Dialogue_Step("", text + GLOBALS->game_t->translate(1024)/* Originally: Press */ + " " + get_key_name(GLOBALS->key_b1) + " " + GLOBALS->game_t->translate(1025)/* Originally: to interact with things. */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_AUTO, new_task);
				}
			}
			else {
				ds2 = new Dialogue_Step("", GLOBALS->game_t->translate(1019)/* Originally: Use the DPAD to move. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1024)/* Originally: Press */ + " " +  get_joystick_button_name(GLOBALS->joy_b1) + " " + GLOBALS->game_t->translate(1025)/* Originally: to interact with things. */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_AUTO, new_task);
			}
			ADD_STEP(wait_step)
			ADD_STEP(delay_step)
			ADD_STEP(ds2)
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			if (joystick_connected == false) {
				bool go = true;
				if (false) {// (never used anymore...)  wedge::is_onscreen_controller_enabled()) {
					gfx::add_notification(GLOBALS->game_t->translate(1029)/* Originally: Press A to scroll... */);
					go = false;
				}
#if defined ANDROID || defined IOS
				else if (util::system_has_touchscreen()) {
					gfx::add_notification(GLOBALS->game_t->translate(1030)/* Originally: Tap to scroll... */);
					go = false;
				}
#endif
				if (go) {
					gfx::add_notification(GLOBALS->game_t->translate(1024)/* Originally: Press */ + " " + get_key_name(GLOBALS->key_b1) + " " + GLOBALS->game_t->translate(1032)/* Originally: to scroll... */);
				}
			}
			else {
				gfx::add_notification(GLOBALS->game_t->translate(1024)/* Originally: Press */ + " " + get_joystick_button_name(GLOBALS->joy_b1) + " " + GLOBALS->game_t->translate(1032)/* Originally: to scroll... */);
			}
		}

		return true;
	}

	bool try_tile(wedge::Map_Entity *entity, util::Point<int> tile_pos)
	{
		if (INSTANCE->is_milestone_complete(MS_FIRST_SCENE) == false) {
			if (tile_pos.x >= 10 && tile_pos.x <= 14 && tile_pos.y == 16) {
				entity->get_input_step()->end_movement();
				GLOBALS->do_dialogue(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1036)/* Originally: (shouting) THIS WAY ENY! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
				return true;
			}
		}

		return false;
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data water_anim;
		water_anim.topleft = util::Point<int>(7, 0);
		water_anim.size = util::Size<int>(1, 1);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(7, 1));
		water_anim.frames.push_back(util::Point<int>(7, 2));
		tilemap->add_animation_data(water_anim);
		water_anim.topleft = util::Point<int>(8, 0);
		water_anim.size = util::Size<int>(1, 1);
		water_anim.delay = 250;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(8, 1));
		water_anim.frames.push_back(util::Point<int>(8, 2));
		water_anim.frames.push_back(util::Point<int>(8, 3));
		water_anim.frames.push_back(util::Point<int>(8, 4));
		water_anim.frames.push_back(util::Point<int>(8, 5));
		tilemap->add_animation_data(water_anim);
	}
};

class Area_Hooks_Fiddler : public Area_Hooks_Monster_RPG_3
{
public:
	static const int DARKNESS_TIME = 5000;

	Area_Hooks_Fiddler(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		tiggy(NULL),
		fiddler(NULL),
		buffer(NULL),
		darkness_sound(NULL)
	{
		Scroll_Zone z;
		z.zone = util::Rectangle<int>(util::Point<int>(8, 19), util::Size<int>(3, 1));
		z.area_name = "start";
		z.topleft_dest = util::Point<int>(7, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Fiddler()
	{
		delete buffer;
		delete darkness_sound;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/forest.mml");
		
		growl = new audio::MML("sfx/growl.mml");

		if (loaded == false) {
			tiggy = new wedge::Map_Entity("tig");
			tiggy->start(area);
			tiggy->set_position(util::Point<int>(9, 13));
			gfx::Sprite *tiggy_sprite = new gfx::Sprite("tiggy");
			tiggy_sprite->set_animation("kneel_e");
			tiggy->set_sprite(tiggy_sprite);
			area->add_entity(tiggy);

			fiddler = new wedge::Map_Entity("fiddler");
			fiddler->start(area);
			fiddler->set_position(util::Point<int>(10, 8));
			fiddler->set_size(util::Size<int>(2, 2));
			gfx::Sprite *fiddler_sprite = new gfx::Sprite("fiddler");
			fiddler->set_sprite(fiddler_sprite);
			area->add_entity(fiddler);

			wedge::Chest *chest1 = new wedge::Chest("chest1", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 3));
			chest1->start(area);
			chest1->set_position(util::Point<int>(6, 5));
			area->add_entity(chest1);

			wedge::Chest *chest2 = new wedge::Chest("chest2", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(7, 5));
			area->add_entity(chest2);

			do_darkness = true;
		}
		else {
			tiggy = area->find_entity("tig");
			fiddler = area->find_entity("fiddler");
			if (INSTANCE->is_milestone_complete(MS_FIRST_SCENE)) {
				do_darkness = false;
			}
			else {
				do_darkness = true;
			}
		}

		return true;
	}

	void started()
	{
		if (INSTANCE->is_milestone_complete(MS_TIGGY_HOO) == false) {
			INSTANCE->set_milestone_complete(MS_TIGGY_HOO, true);
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1038)/* Originally: Hoo! I told you so! Look at these! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		util::Point<int> pos = entity->get_position();

		if (INSTANCE->is_milestone_complete(MS_FIRST_SCENE) == false) {
			if (pos.y <= 10) {
				INSTANCE->set_milestone_complete(MS_FIRST_SCENE, true);

				NEW_SYSTEM_AND_TASK(AREA)
				wedge::Map_Entity *player = AREA->get_player(ENY);
				wedge::Pause_Presses_Step *pp1 = new wedge::Pause_Presses_Step(true, false, new_task);
				wedge::A_Star_Step *as1 = new wedge::A_Star_Step(player, util::Point<int>(10, 9), new_task);
				Dialogue_Step *ds1 = new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1040)/* Originally: ZAA! T-Tiggy! Come here! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task);
				wedge::A_Star_Step *as2 = new wedge::A_Star_Step(tiggy, util::Point<int>(11, 9), new_task);
				Dialogue_Step *ds2 = new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1042)/* Originally: Man alive! How do we carry it home? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task);
				gfx::Sprite *sprite = fiddler->get_sprite();
				wedge::Play_Animation_Step *pas1 = new wedge::Play_Animation_Step(sprite, "open", new_task);
				wedge::Play_Animation_Step *pas2 = new wedge::Play_Animation_Step(sprite, "idle", new_task);
				Dialogue_Step *ds3 = new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1044)/* Originally: Did you see that? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task);
				Dialogue_Step *ds4 = new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1046)/* Originally: See what? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task);
				Dialogue_Step *ds5 = new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1048)/* Originally: Tiggy, look out! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task);
				wedge::Play_Sound_Step *sound1 = new wedge::Play_Sound_Step(growl, false, false, new_task);
				
				wedge::Delay_Step *delay = new wedge::Delay_Step(1000, new_task);
				Start_Battle_Step *battle_step = new Start_Battle_Step(new Battle_Fiddler(), new_task);
				ADD_STEP(pp1)
				ADD_STEP(as1)
				ADD_STEP(ds1)
				ADD_STEP(as2)
				ADD_STEP(ds2)
				ADD_STEP(pas1);
				ADD_STEP(pas2);
				ADD_STEP(ds3)
				ADD_STEP(ds4)
				ADD_STEP(ds5)
				ADD_STEP(sound1)
				ADD_STEP(delay)
				ADD_STEP(battle_step)
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)
				return true;
			}
			else if (pos.y <= 15 && INSTANCE->is_milestone_complete(MS_LOOK_FOR_MORE) == false) {
				//entity->get_input_step()->end_movement();
				GLOBALS->do_dialogue(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1050)/* Originally: I'll get these ones, you look for more! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
				INSTANCE->set_milestone_complete(MS_LOOK_FOR_MORE, true);
				return true;
			}
		}
		return false;
	}

	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activated == tiggy) {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1052)/* Originally: Go look for fiddleheads! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
			return true;
		}
		return false;
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer < 3) {
			return;
		}

		if (do_darkness && fiddler == NULL) {
			Uint32 now = GET_TICKS() - darkness_start;
			if (now >= DARKNESS_TIME) {
				if (buffer) {
					delete buffer;
					buffer = NULL;
				}
			}
			else {
				if (buffer != NULL && buffer_size != shim::screen_size) {
					delete buffer;
					buffer = NULL;
				}
				if (buffer == NULL) {
					buffer = new gfx::Image(shim::screen_size*2);
					buffer_size = shim::screen_size;
				}

				gfx::set_target_image(buffer);
				gfx::clear(shim::transparent);
			
				shim::current_shader = M3_GLOBALS->darkness_shader;
				shim::current_shader->use();
				gfx::update_projection();
				const int phase = DARKNESS_TIME;
				const int half_phase = phase / 2;
				Uint32 mod = now % half_phase;
				float t = mod / (float)half_phase;
				shim::current_shader->set_int("screen_w", buffer->size.w);
				shim::current_shader->set_int("screen_h", buffer->size.h);
				shim::current_shader->set_float("t", t);

				float mush_x = float(buffer->size.w/2+6);
				float mush_y = float(buffer->size.h/2+18);
				float max_x = mush_x;
				float max_y = mush_y;
				float max = sqrt(max_x*max_x + max_y*max_y);
				float j = fmodf(float(now), float(phase));
				bool b;
				if (j >= float(phase/2)) {
					b = false;
				}
				else {
					b = true;
				}
				SDL_Colour colour1;
				SDL_Colour colour2;
				if (b) {
					colour1 = shim::black;
					colour2 = shim::transparent;
				}
				else {
					colour1 = shim::transparent;
					colour2 = shim::black;
				}

				shim::current_shader->set_float("mush_x", mush_x);
				shim::current_shader->set_float("mush_y", mush_y);
				shim::current_shader->set_float("maxx", max);
				shim::current_shader->set_colour("colour1", colour1);
				shim::current_shader->set_colour("colour2", colour2);

				gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), buffer->size);

				shim::current_shader = shim::default_shader;
				shim::current_shader->use();
				gfx::update_projection();

				gfx::set_target_backbuffer();

				buffer->stretch_region(util::Point<int>(0, 0), buffer->size, util::Point<int>(0, 0), shim::screen_size);
			}
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	void remove_entity(wedge::Map_Entity *entity)
	{
		if (entity == fiddler) {
			INSTANCE->set_milestone_complete(MS_FIRST_SCENE, true);

			fiddler = NULL;
			darkness_start = GET_TICKS();
			darkness_sound = new audio::MML("sfx/darkness.mml");
			darkness_sound->play(false);
			
			NEW_SYSTEM_AND_TASK(AREA)
			wedge::Delay_Step *delay_step = new wedge::Delay_Step(DARKNESS_TIME, new_task);
			Dialogue_Step *ds1 = new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1054)/* Originally: What's going on??? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task);
			Dialogue_Step *ds2 = new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1056)/* Originally: What was that thing? */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1057)/* Originally: I haven't seen a monster in ages! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task);
			Dialogue_Step *ds3 = new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1059)/* Originally: I hope it's the last! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1060)/* Originally: Let's head back to the village. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task);
			wedge::Pause_Presses_Step *pp1 = new wedge::Pause_Presses_Step(false, false, new_task);
			ADD_STEP(delay_step)
			ADD_STEP(ds1)
			ADD_STEP(ds2)
			ADD_STEP(ds3)
			ADD_STEP(pp1)
			ADD_STEP(new Autosave_Step(new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
		}
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete buffer;
		buffer = NULL;
	}

private:
	wedge::Map_Entity *tiggy;
	wedge::Map_Entity *fiddler;
	audio::Sound *growl;
	gfx::Image *buffer;
	util::Size<int> buffer_size;
	Uint32 darkness_start;
	audio::MML *darkness_sound;
	bool do_darkness;
};

class Area_Hooks_Forest_Save : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Forest_Save(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Scroll_Zone z;
		z.zone = util::Rectangle<int>(util::Point<int>(7, 0), util::Size<int>(5, 1));
		z.area_name = "start";
		z.topleft_dest = util::Point<int>(10, 16);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);

		z.zone = util::Rectangle<int>(util::Point<int>(18, 11), util::Size<int>(5, 1));
		z.area_name = "forest1";
		z.topleft_dest = util::Point<int>(12, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Forest_Save()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/forest.mml");

		return true;
	}

	bool can_save()
	{
		return true;
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data savezone_anim;
		savezone_anim.topleft = util::Point<int>(9, 0);
		savezone_anim.size = util::Size<int>(2, 2);
		savezone_anim.delay = 128;
		savezone_anim.frames.push_back(util::Point<int>(9, 2));
		savezone_anim.frames.push_back(util::Point<int>(9, 4));
		savezone_anim.frames.push_back(util::Point<int>(9, 6));
		savezone_anim.frames.push_back(util::Point<int>(9, 8));
		savezone_anim.frames.push_back(util::Point<int>(9, 10));
		savezone_anim.frames.push_back(util::Point<int>(9, 12));
		savezone_anim.frames.push_back(util::Point<int>(9, 14));
		savezone_anim.frames.push_back(util::Point<int>(9, 16));
		savezone_anim.frames.push_back(util::Point<int>(9, 18));
		savezone_anim.frames.push_back(util::Point<int>(9, 20));
		savezone_anim.frames.push_back(util::Point<int>(9, 22));
		tilemap->add_animation_data(savezone_anim);
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		util::Point<int> pos = entity->get_position();
		if (entity == AREA->get_player(ENY) && pos.x >= 10 && pos.y >= 4) {
			if (INSTANCE->is_milestone_complete(MS_SAVEZONE_INTRO) == false) {
				INSTANCE->set_milestone_complete(MS_SAVEZONE_INTRO, true);
#ifdef TVOS
				bool joystick_connected = true;
#else
				bool joystick_connected = input::is_joystick_connected();
#endif
				std::string menu_text;
				if (joystick_connected == false) {
					if (GLOBALS->onscreen_controller_was_enabled) {
						menu_text = GLOBALS->game_t->translate(1061)/* Originally: Press B to access the menu. */;
					}
#if defined ANDROID || defined IOS
					else {
						menu_text = GLOBALS->game_t->translate(1062)/* Originally: Tap the top left corner to access the menu. */;
					}
#else
					else {
						menu_text = GLOBALS->game_t->translate(1024)/* Originally: Press */ + " " + get_key_name(GLOBALS->key_b2) + " " + GLOBALS->game_t->translate(1064)/* Originally: to access the menu. */;
					}
#endif
				}
				else {
					menu_text = GLOBALS->game_t->translate(1024)/* Originally: Press */ + " " + get_joystick_button_name(GLOBALS->joy_b2) + " " + GLOBALS->game_t->translate(1064)/* Originally: to access the menu. */;
				}

				GLOBALS->do_dialogue("", GLOBALS->game_t->translate(1067)/* Originally: This is a Save Zone. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1068)/* Originally: You can save your progress here. */ + NEW_PARAGRAPH + menu_text, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_BOTTOM, NULL);

				return true;
			}
		}

		return false;
	}

private:
};


class Area_Hooks_Forest1 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Forest1(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Scroll_Zone z;
		z.zone = util::Rectangle<int>(util::Point<int>(12, 0), util::Size<int>(5, 1));
		z.area_name = "forest_save";
		z.topleft_dest = util::Point<int>(18, 11);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);

		z.zone = util::Rectangle<int>(util::Point<int>(29, 11), util::Size<int>(1, 20));
		z.area_name = "forest2";
		z.topleft_dest = util::Point<int>(0, 6);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Forest1()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/forest.mml");
		
		if (loaded == false) {
			wedge::Chest *chest1 = new wedge::Chest("chest1", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 1));
			chest1->start(area);
			chest1->set_position(util::Point<int>(24, 8));
			area->add_entity(chest1);
			
			wedge::Chest *chest2 = new wedge::Chest("chest2", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_AXE, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(3, 14));
			area->add_entity(chest2);
			chest2->set_layer(5);
			
			wedge::Chest *chest3 = new wedge::Chest("chest3", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_SHORT_SWORD, 1));
			chest3->start(area);
			chest3->set_position(util::Point<int>(5, 14));
			area->add_entity(chest3);
			chest3->set_layer(5);
			
			wedge::NPC *feller = new wedge::NPC("Feller", "Feller", "lumberjack1", "lumberjack1");
			feller->start(area);
			feller->set_position(util::Point<int>(8, 26));
			area->add_entity(feller);
		}

		tiles_up = true;

		return true;
	}
	
	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(3);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 2) {
			return new Battle_2Goos();
		}
		else if (type == 1) {
			return new Battle_Mushroom();
		}
		else {
			return new Battle_3Bloated();
		}
	}

	void dialogue_done(wedge::Map_Entity *entity)
	{
		if (entity != NULL && entity->get_name() == "Feller") {
			NEW_SYSTEM_AND_TASK(AREA)
			wedge::Pause_Presses_Step *pause1 = new wedge::Pause_Presses_Step(true, false, new_task);
			wedge::Set_Visible_Step *visible1 = new wedge::Set_Visible_Step(entity, false, new_task);
			wedge::Delay_Step *delay1 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible2 = new wedge::Set_Visible_Step(entity, true, new_task);
			wedge::Delay_Step *delay2 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible3 = new wedge::Set_Visible_Step(entity, false, new_task);
			wedge::Delay_Step *delay3 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible4 = new wedge::Set_Visible_Step(entity, true, new_task);
			wedge::Delay_Step *delay4 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible5 = new wedge::Set_Visible_Step(entity, false, new_task);
			wedge::Delay_Step *delay5 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible6 = new wedge::Set_Visible_Step(entity, true, new_task);
			wedge::Delay_Step *delay6 = new wedge::Delay_Step(250, new_task);
			wedge::Delete_Map_Entity_Step *del = new wedge::Delete_Map_Entity_Step(entity, new_task);
			wedge::Give_Object_Step *give = new wedge::Give_Object_Step(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_RING1, 1), wedge::DIALOGUE_AUTO, new_task);
			wedge::Pause_Presses_Step *pause2 = new wedge::Pause_Presses_Step(false, false, new_task);
			ADD_STEP(pause1)
			ADD_STEP(visible1)
			ADD_STEP(delay1)
			ADD_STEP(visible2)
			ADD_STEP(delay2)
			ADD_STEP(visible3)
			ADD_STEP(delay3)
			ADD_STEP(visible4)
			ADD_STEP(delay4)
			ADD_STEP(visible5)
			ADD_STEP(delay5)
			ADD_STEP(visible6)
			ADD_STEP(delay6)
			ADD_STEP(del)
			ADD_STEP(give)
			ADD_STEP(pause2)
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
		}
	}
	
	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		if (entity != AREA->get_player(ENY)) {
			return false;
		}

		util::Point<int> pos = entity->get_position();

		gfx::Tilemap *tilemap = area->get_tilemap();

		if (pos.x == 4 && pos.y == 16 && tiles_up) {
			tilemap->set_solid(-1, util::Point<int>(2, 14), util::Size<int>(5, 1), false);
			tilemap->set_solid(-1, util::Point<int>(1, 13), util::Size<int>(7, 1), true);
			tilemap->swap_tiles(2, 3, util::Point<int>(1, 13), util::Size<int>(7, 1));
			tiles_up = false;
		}
		else if (pos.x == 4 && pos.y == 17 && tiles_up == false) {
			tilemap->set_solid(-1, util::Point<int>(2, 14), util::Size<int>(5, 1), true);
			tilemap->set_solid(-1, util::Point<int>(1, 13), util::Size<int>(7, 1), false);
			tilemap->swap_tiles(2, 3, util::Point<int>(1, 13), util::Size<int>(7, 1));
			tiles_up = true;
		}

		return false;
	}

	std::vector<util::A_Star::Way_Point> get_way_points(util::Point<int> from)
	{
		std::vector<util::A_Star::Way_Point> v;
		util::Rectangle<int> r(util::Point<int>(2, 14), util::Size<int>(5, 3)); // the platform
		if (r.contains(from) == false) {
			util::A_Star::Way_Point wp;
			wp.to.push_back(util::Point<int>(2, 14));
			wp.to.push_back(util::Point<int>(4, 14));
			wp.to.push_back(util::Point<int>(6, 14));
			wp.by.push_back(util::Point<int>(4, 15));
			v.push_back(wp);
		}
		return v;
	}

private:
	bool tiles_up;
};

class Area_Hooks_Forest2 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Forest2(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Scroll_Zone z;
		z.zone = util::Rectangle<int>(util::Point<int>(0, 6), util::Size<int>(1, 20));
		z.area_name = "forest1";
		z.topleft_dest = util::Point<int>(29, 11);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
		
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(38, 29), util::Size<int>(3, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(22, 0));
		fz1.player_positions.push_back(util::Point<int>(23, 0));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Forest2()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/forest.mml");
		
		if (loaded == false) {
			wedge::NPC *feller = new wedge::NPC("Feller", "Feller", "lumberjack2", "lumberjack2");
			feller->start(area);
			feller->set_position(util::Point<int>(5, 7));
			area->add_entity(feller);
			
			wedge::Chest *chest1 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 1));
			chest1->start(area);
			chest1->set_position(util::Point<int>(24, 7));
			area->add_entity(chest1);
			
			wedge::Chest *chest2 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(25, 7));
			area->add_entity(chest2);
		}
		else {
		}

		return true;
	}
	
	bool try_tile(wedge::Map_Entity *entity, util::Point<int> tile_pos)
	{
		util::Point<int> curr_pos = entity->get_position();
		std::string name = entity->get_name();
		bool is_player = (name == "Eny" || name == "Tiggy");

		gfx::Tilemap *tilemap = area->get_tilemap();
		if (tilemap->is_solid(-1, tile_pos)) {
			return false;
		}

		if ((curr_pos.y == 22 && (curr_pos.x >= 12 && curr_pos.x <= 14)) || (tile_pos.y == 22 && (tile_pos.x >= 12 && tile_pos.x <= 14))) {
			if (is_player) {
				entity->set_jumping(true);
				entity->set_direction(entity->get_direction(), true, true);
				M3_GLOBALS->jump->play(false);
			}
		}
		else {
			if (is_player) {
				entity->set_jumping(false);
			}
		}

		return false;
	}

	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		util::Point<int> player_pos = AREA->get_player(ENY)->get_position();

		// no battles on rock bridge
		if (player_pos.y == 22 && (player_pos.x >= 12 && player_pos.x <= 14)) {
			return NULL;
		}

		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(3);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 2) {
			return new Battle_2Treant();
		}
		else if (type == 1) {
			return new Battle_Mushroom();
		}
		else {
			return new Battle_3Bloated();
		}
	}

	void dialogue_done(wedge::Map_Entity *entity)
	{
		if (entity != NULL && entity->get_name() == "Feller") {
			NEW_SYSTEM_AND_TASK(AREA)
			wedge::Pause_Presses_Step *pause1 = new wedge::Pause_Presses_Step(true, false, new_task);
			wedge::Set_Visible_Step *visible1 = new wedge::Set_Visible_Step(entity, false, new_task);
			wedge::Delay_Step *delay1 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible2 = new wedge::Set_Visible_Step(entity, true, new_task);
			wedge::Delay_Step *delay2 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible3 = new wedge::Set_Visible_Step(entity, false, new_task);
			wedge::Delay_Step *delay3 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible4 = new wedge::Set_Visible_Step(entity, true, new_task);
			wedge::Delay_Step *delay4 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible5 = new wedge::Set_Visible_Step(entity, false, new_task);
			wedge::Delay_Step *delay5 = new wedge::Delay_Step(250, new_task);
			wedge::Set_Visible_Step *visible6 = new wedge::Set_Visible_Step(entity, true, new_task);
			wedge::Delay_Step *delay6 = new wedge::Delay_Step(250, new_task);
			wedge::Delete_Map_Entity_Step *del = new wedge::Delete_Map_Entity_Step(entity, new_task);
			wedge::Give_Object_Step *give = new wedge::Give_Object_Step(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_RING2, 1), wedge::DIALOGUE_AUTO, new_task);
			wedge::Pause_Presses_Step *pause2 = new wedge::Pause_Presses_Step(false, false, new_task);
			ADD_STEP(pause1)
			ADD_STEP(visible1)
			ADD_STEP(delay1)
			ADD_STEP(visible2)
			ADD_STEP(delay2)
			ADD_STEP(visible3)
			ADD_STEP(delay3)
			ADD_STEP(visible4)
			ADD_STEP(delay4)
			ADD_STEP(visible5)
			ADD_STEP(delay5)
			ADD_STEP(visible6)
			ADD_STEP(delay6)
			ADD_STEP(del)
			ADD_STEP(give)
			ADD_STEP(pause2)
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
		}
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();

		gfx::Tilemap::Animation_Data river_anim;
		river_anim.topleft = util::Point<int>(5, 6);
		river_anim.size = util::Size<int>(3, 1);
		river_anim.delay = 128;
		river_anim.frames.push_back(util::Point<int>(5, 7));
		river_anim.frames.push_back(util::Point<int>(5, 8));
		river_anim.frames.push_back(util::Point<int>(5, 9));
		river_anim.frames.push_back(util::Point<int>(5, 10));
		river_anim.frames.push_back(util::Point<int>(5, 11));
		river_anim.frames.push_back(util::Point<int>(5, 12));
		river_anim.frames.push_back(util::Point<int>(5, 13));
		river_anim.frames.push_back(util::Point<int>(5, 14));
		river_anim.frames.push_back(util::Point<int>(5, 15));
		river_anim.frames.push_back(util::Point<int>(5, 16));
		river_anim.frames.push_back(util::Point<int>(5, 17));
		tilemap->add_animation_data(river_anim);

		gfx::Tilemap::Animation_Data crossing_rocks_anim;
		crossing_rocks_anim.topleft = util::Point<int>(3, 4);
		crossing_rocks_anim.size = util::Size<int>(1, 2);
		crossing_rocks_anim.delay = 128;
		crossing_rocks_anim.frames.push_back(util::Point<int>(3, 6));
		tilemap->add_animation_data(crossing_rocks_anim);
	}
};

class Area_Hooks_Riverside : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Riverside(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(6, 23), util::Size<int>(1, 1));
		fz1.area_name = "riverside_inn";
		fz1.player_positions.push_back(util::Point<int>(7, 11));
		fz1.player_positions.push_back(util::Point<int>(7, 11));
		fz1.directions.push_back(wedge::DIR_N);
		fz1.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz1);
		
		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(20, 0), util::Size<int>(6, 1));
		fz2.area_name = "forest2";
		fz2.player_positions.push_back(util::Point<int>(39, 29));
		fz2.player_positions.push_back(util::Point<int>(39, 29));
		fz2.directions.push_back(wedge::DIR_N);
		fz2.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz2);
		
		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(11, 12), util::Size<int>(1, 1));
		fz3.area_name = "riverside_item_shop";
		fz3.player_positions.push_back(util::Point<int>(3, 7));
		fz3.player_positions.push_back(util::Point<int>(3, 7));
		fz3.directions.push_back(wedge::DIR_N);
		fz3.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz3);
		
		Fade_Zone fz4;
		fz4.zone = util::Rectangle<int>(util::Point<int>(34, 12), util::Size<int>(1, 1));
		fz4.area_name = "riverside_equipment_shop";
		fz4.player_positions.push_back(util::Point<int>(3, 7));
		fz4.player_positions.push_back(util::Point<int>(3, 7));
		fz4.directions.push_back(wedge::DIR_N);
		fz4.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz4);
		
		Fade_Zone fz5;
		fz5.zone = util::Rectangle<int>(util::Point<int>(40, 14), util::Size<int>(1, 1));
		fz5.area_name = "lumberjacks_dads";
		fz5.player_positions.push_back(util::Point<int>(5, 7));
		fz5.player_positions.push_back(util::Point<int>(5, 7));
		fz5.directions.push_back(wedge::DIR_N);
		fz5.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz5);
		
		Fade_Zone fz6;
		fz6.zone = util::Rectangle<int>(util::Point<int>(32, 18), util::Size<int>(1, 1));
		fz6.area_name = "coros";
		fz6.player_positions.push_back(util::Point<int>(4, 6));
		fz6.player_positions.push_back(util::Point<int>(4, 6));
		fz6.directions.push_back(wedge::DIR_N);
		fz6.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz6);
		
		Fade_Zone fz7;
		fz7.zone = util::Rectangle<int>(util::Point<int>(13, 20), util::Size<int>(1, 1));
		fz7.area_name = "moryts";
		fz7.player_positions.push_back(util::Point<int>(4, 6));
		fz7.player_positions.push_back(util::Point<int>(4, 6));
		fz7.directions.push_back(wedge::DIR_N);
		fz7.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz7);
		
		Fade_Zone fz8;
		fz8.zone = util::Rectangle<int>(util::Point<int>(39, 23), util::Size<int>(1, 1));
		fz8.area_name = "captains1";
		fz8.player_positions.push_back(util::Point<int>(6, 7));
		fz8.player_positions.push_back(util::Point<int>(6, 7));
		fz8.directions.push_back(wedge::DIR_N);
		fz8.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz8);
		
		Fade_Zone fz9;
		fz9.zone = util::Rectangle<int>(util::Point<int>(5, 14), util::Size<int>(1, 1));
		fz9.area_name = "womans";
		fz9.player_positions.push_back(util::Point<int>(5, 8));
		fz9.player_positions.push_back(util::Point<int>(5, 8));
		fz9.directions.push_back(wedge::DIR_N);
		fz9.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz9);
		
		Fade_Zone fz10;
		fz10.zone = util::Rectangle<int>(util::Point<int>(22, 19), util::Size<int>(2, 1));
		fz10.area_name = "palace1";
		fz10.player_positions.push_back(util::Point<int>(9, 19));
		fz10.player_positions.push_back(util::Point<int>(10, 19));
		fz10.directions.push_back(wedge::DIR_N);
		fz10.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz10);
		
		Fade_Zone fz11;
		fz11.zone = util::Rectangle<int>(util::Point<int>(28, 51), util::Size<int>(8, 1));
		fz11.area_name = "hh";
		fz11.player_positions.push_back(util::Point<int>(6, 0));
		fz11.player_positions.push_back(util::Point<int>(7, 0));
		fz11.directions.push_back(wedge::DIR_S);
		fz11.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz11);
		
		Fade_Zone fz12;
		fz12.zone = util::Rectangle<int>(util::Point<int>(11, 51), util::Size<int>(9, 1));
		fz12.area_name = "beach_w";
		fz12.player_positions.push_back(util::Point<int>(14, 0));
		fz12.player_positions.push_back(util::Point<int>(15, 0));
		fz12.directions.push_back(wedge::DIR_S);
		fz12.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz12);
	}
	
	virtual ~Area_Hooks_Riverside()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			std::vector< util::Point<int> > soldier_positions;
			soldier_positions.push_back(util::Point<int>(20, 5));
			soldier_positions.push_back(util::Point<int>(25, 5));
			soldier_positions.push_back(util::Point<int>(21, 7));
			soldier_positions.push_back(util::Point<int>(24, 7));
			soldier_positions.push_back(util::Point<int>(21, 28));
			soldier_positions.push_back(util::Point<int>(24, 28));

			std::vector<wedge::Direction> soldier_directions;
			soldier_directions.push_back(wedge::DIR_N);
			soldier_directions.push_back(wedge::DIR_N);
			soldier_directions.push_back(wedge::DIR_N);
			soldier_directions.push_back(wedge::DIR_N);
			soldier_directions.push_back(wedge::DIR_S);
			soldier_directions.push_back(wedge::DIR_S);

			for (int i = 0; i < 6; i++) {
				std::string name = "soldier" + util::itos(i+1);
				soldiers[i] = new wedge::NPC(name, "Soldier", "soldier", name);
				soldiers[i]->start(area);
				soldiers[i]->set_position(soldier_positions[i]);
				soldiers[i]->set_direction(soldier_directions[i], true, false);
				area->add_entity(soldiers[i]);
			}
			
			wedge::NPC *coro = new wedge::NPC("coro", "Coro", "coro", "coro");
			coro->start(area);
			coro->set_position(util::Point<int>(33, 20));
			coro->set_size(util::Size<int>(2, 2));
			area->add_entity(coro);

			sign1 = new wedge::Map_Entity("sign1");
			sign1->start(area);
			sign1->set_position(util::Point<int>(16, 47));
			area->add_entity(sign1);

			sign2 = new wedge::Map_Entity("sign2");
			sign2->start(area);
			sign2->set_position(util::Point<int>(30, 48));
			area->add_entity(sign2);
		}
		else {
			for (int i = 0; i < 6; i++) {
				std::string name = "soldier" + util::itos(i+1);
				soldiers[i] = area->find_entity(name);
			}

			sign1 = area->find_entity("sign1");
			sign2 = area->find_entity("sign2");
		}

		return true;
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		util::Point<int> player_pos = entity->get_position();

		if (entity == AREA->get_player(ENY) && player_pos.y == 2 && INSTANCE->is_milestone_complete(MS_RIVERSIDE_SOLDIER_CHAT) == false) {
			INSTANCE->set_milestone_complete(MS_RIVERSIDE_SOLDIER_CHAT, true);

			util::Point<int> dest1;
			util::Point<int> dest2;

			if (player_pos.x == soldiers[1]->get_position().x) {
				dest1 = player_pos + util::Point<int>(-1, 1);
				dest2 = player_pos + util::Point<int>(0, 1);
			}
			else {
				dest1 = player_pos + util::Point<int>(0, 1);
				dest2 = player_pos + util::Point<int>(1, 1);
			}

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new wedge::A_Star_Step(soldiers[0], dest1, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(soldiers[0]);
			entities.push_back(soldiers[1]);
			std::vector< util::Point<int> > positions;
			positions.push_back(dest1);
			positions.push_back(dest2);
			ADD_STEP(new wedge::Check_Positions_Step(entities, positions, true, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(84)/* Originally: Soldier */ + TAG_END, GLOBALS->game_t->translate(1070)/* Originally: Halt! Who goes there?! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1072)/* Originally: It's just us. Eny and Tiggy. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(84)/* Originally: Soldier */ + TAG_END, GLOBALS->game_t->translate(1074)/* Originally: Oh, it's you... There has been a sudden influx of monsters so we're on our guard. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1076)/* Originally: Yeah, that might be our fault... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			Dialogue_Step *ds5 = new Dialogue_Step(GLOBALS->game_t->translate(84)/* Originally: Soldier */ + TAG_END, GLOBALS->game_t->translate(1078)/* Originally: I should have known. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1079)/* Originally: Well the mayor wants to see you. Head to the inn to rest then go to the palace! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task);
			ADD_STEP(ds5)
			ADD_STEP(new wedge::A_Star_Step(soldiers[0], soldiers[0]->get_position()/*back to current position*/, new_task))
			ADD_STEP(new wedge::Set_Direction_Step(soldiers[0], wedge::DIR_N, true, false, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			ANOTHER_TASK
			ADD_STEP(new wedge::A_Star_Step(soldiers[1], dest2, new_task))
			wedge::Wait_Step *ws = new wedge::Wait_Step(new_task);
			ADD_STEP(ws)
			ds5->add_monitor(ws);
			ADD_STEP(new wedge::A_Star_Step(soldiers[1], soldiers[1]->get_position()/*back to current position*/, new_task))
			ADD_STEP(new wedge::Set_Direction_Step(soldiers[1], wedge::DIR_N, true, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			return true;
		}

		return false;
	}

	bool try_tile(wedge::Map_Entity *entity, util::Point<int> tile_pos)
	{
		if (entity == AREA->get_player(0) && INSTANCE->is_milestone_complete(MS_TALKED_TO_MAYOR) == false && (tile_pos.x == 22 || tile_pos.x == 23) && tile_pos.y == 28) {
			util::Point<int> soldier4_pos(22, 28);
			util::Point<int> soldier5_pos(23, 28);

			NEW_SYSTEM_AND_TASK(AREA)

			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new wedge::A_Star_Step(soldiers[4], soldier4_pos, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(soldiers[4]);
			entities.push_back(soldiers[5]);
			std::vector< util::Point<int> > positions;
			positions.push_back(soldier4_pos);
			positions.push_back(soldier5_pos);
			ADD_STEP(new wedge::Check_Positions_Step(entities, positions, true, new_task))
			ADD_STEP(new wedge::Set_Direction_Step(soldiers[4], wedge::DIR_N, true, false, new_task))
			Dialogue_Step *ds;
			ADD_STEP(ds = new Dialogue_Step(GLOBALS->game_t->translate(84)/* Originally: Soldier */ + TAG_END, GLOBALS->game_t->translate(1081)/* Originally: You're not to leave town until you go to the palace. Mayor's orders. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::A_Star_Step(soldiers[4], soldiers[4]->get_position(), new_task))
			ADD_STEP(new wedge::Set_Direction_Step(soldiers[4], wedge::DIR_S, true, false, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			ADD_STEP(new wedge::A_Star_Step(soldiers[5], soldier5_pos, new_task))
			ADD_STEP(new wedge::Set_Direction_Step(soldiers[5], wedge::DIR_N, true, false, new_task))
			wedge::Wait_Step *wait_step = new wedge::Wait_Step(new_task);
			ds->add_monitor(wait_step);
			ADD_STEP(wait_step)
			ADD_STEP(new wedge::A_Star_Step(soldiers[5], soldiers[5]->get_position(), new_task))
			ADD_STEP(new wedge::Set_Direction_Step(soldiers[5], wedge::DIR_S, true, false, new_task))
			ADD_TASK(new_task)

			FINISH_SYSTEM(AREA)

			return true;
		}

		return false;
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data river_anim2;
		river_anim2.topleft = util::Point<int>(14, 0);
		river_anim2.size = util::Size<int>(1, 3);
		river_anim2.delay = 128;
		river_anim2.frames.push_back(util::Point<int>(15, 0));
		river_anim2.frames.push_back(util::Point<int>(16, 0));
		river_anim2.frames.push_back(util::Point<int>(17, 0));
		river_anim2.frames.push_back(util::Point<int>(18, 0));
		river_anim2.frames.push_back(util::Point<int>(19, 0));
		river_anim2.frames.push_back(util::Point<int>(20, 0));
		river_anim2.frames.push_back(util::Point<int>(21, 0));
		river_anim2.frames.push_back(util::Point<int>(22, 0));
		river_anim2.frames.push_back(util::Point<int>(23, 0));
		river_anim2.frames.push_back(util::Point<int>(24, 0));
		river_anim2.frames.push_back(util::Point<int>(25, 0));
		tilemap->add_animation_data(river_anim2);
	}

	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activated == sign1) {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1083)/* Originally: \"To the Coast\"... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
			return true;
		}
		else if (activated == sign2) {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1085)/* Originally: \"To the Haunted Mansion\"... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
			return true;
		}
		return false;
	}

private:
	wedge::Map_Entity *soldiers[6];
	wedge::Map_Entity *sign1;
	wedge::Map_Entity *sign2;
};

class Area_Hooks_Riverside_Inn : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Riverside_Inn(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(7, 11), util::Size<int>(1, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(6, 23));
		fz1.player_positions.push_back(util::Point<int>(6, 23));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(7, 1), util::Size<int>(1, 1));
		fz2.area_name = "riverside_inn_upper";
		fz2.player_positions.push_back(util::Point<int>(7, 2));
		fz2.player_positions.push_back(util::Point<int>(7, 2));
		fz2.directions.push_back(wedge::DIR_E);
		fz2.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_Riverside_Inn()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			util::Point<int> innkeep_pos(11, 3);
			innkeep = new wedge::Map_Entity("innkeep");
			innkeep->set_wanders(true, true, false, false, innkeep_pos, 2);
			innkeep->start(area);
			innkeep->set_position(innkeep_pos);
			gfx::Sprite *innkeep_sprite = new gfx::Sprite("innkeep");
			innkeep->set_sprite(innkeep_sprite);
			area->add_entity(innkeep);

			wedge::Chest *chest = new wedge::Chest("chest", "", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(13, 5));
			area->add_entity(chest);
		}
		else {
			innkeep = area->find_entity("innkeep");
		}

		return true;
	}

	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> activator_pos = activator->get_position();
		util::Point<int> innkeep_pos = innkeep->get_position();
		if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && (activator_pos-innkeep_pos) == util::Point<int>(0, 2)) {
			Reset_Anim *ra = new Reset_Anim;
			ra->was_moving = innkeep->is_moving();
			ra->direction = innkeep->get_direction();
			ra->entity = innkeep;
		
			NEW_SYSTEM_AND_TASK(AREA)
			wedge::Generic_Callback_Step *step = new wedge::Generic_Callback_Step(reset_anim, ra, new_task);
			ADD_STEP(step)
			ADD_TASK(new_task)

			ANOTHER_TASK

			innkeep->get_sprite()->set_animation("stand_s");
			ADD_STEP(new Inn_Step(wedge::DIALOGUE_SPEECH, GLOBALS->game_t->translate(1086)/* Originally: Innkeep */, GLOBALS->game_t->translate(1087)/* Originally: Eny, Tiggy! Would you like to stay at the inn? */, GLOBALS->game_t->translate(1088)/* Originally: Come back anytime! */, GLOBALS->game_t->translate(1089)/* Originally: Enjoy your sleep! */, "", 0, util::Point<int>(3, 2), new_task, step))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}

		return false;
	}

private:
	wedge::Map_Entity *innkeep;
};

class Area_Hooks_Riverside_Inn_Upper : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Riverside_Inn_Upper(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(7, 2), util::Size<int>(1, 1));
		fz1.area_name = "riverside_inn";
		fz1.player_positions.push_back(util::Point<int>(7, 1));
		fz1.player_positions.push_back(util::Point<int>(7, 1));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Riverside_Inn_Upper()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			wedge::Chest *chest = new wedge::Chest("chest", "", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_CURE, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(9, 2));
			area->add_entity(chest);

			wedge::Chest *chest2 = new wedge::Chest("chest2", "", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(7, 10));
			area->add_entity(chest2);
		}

		return true;
	}
};

class Area_Hooks_Riverside_Item_Shop : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Riverside_Item_Shop(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(3, 7), util::Size<int>(1, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(11, 12));
		fz1.player_positions.push_back(util::Point<int>(11, 12));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Riverside_Item_Shop()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			dealer = new wedge::Map_Entity("dealer");
			dealer->start(area);
			dealer->set_position(util::Point<int>(3, 3));
			gfx::Sprite *dealer_sprite = new gfx::Sprite("riverside_item_dealer");
			dealer->set_sprite(dealer_sprite);
			area->add_entity(dealer);
		}
		else {
			dealer = area->find_entity("dealer");
		}

		return true;
	}

	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> activator_pos = activator->get_position();
		util::Point<int> dealer_pos = dealer->get_position();
		if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && (activator_pos-dealer_pos) == util::Point<int>(0, 2)) {
			dealer->get_sprite()->set_animation("stand_s");
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1090)/* Originally: Shopkeep */ + TAG_END, GLOBALS->game_t->translate(1091)/* Originally: I deal in items. What can I get for you? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 10));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_CURE, 15));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 50));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 50));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_ITEM, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}

		return false;
	}

private:
	wedge::Map_Entity *dealer;
};

class Area_Hooks_Riverside_Equipment_Shop : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Riverside_Equipment_Shop(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(3, 7), util::Size<int>(1, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(34, 12));
		fz1.player_positions.push_back(util::Point<int>(34, 12));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Riverside_Equipment_Shop()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			weapons_dealer = new wedge::Map_Entity("weapons_dealer");
			weapons_dealer->start(area);
			weapons_dealer->set_position(util::Point<int>(2, 3));
			gfx::Sprite *weapons_dealer_sprite = new gfx::Sprite("riverside_weapons_dealer");
			weapons_dealer->set_sprite(weapons_dealer_sprite);
			area->add_entity(weapons_dealer);

			armour_dealer = new wedge::Map_Entity("armour_dealer");
			armour_dealer->start(area);
			armour_dealer->set_position(util::Point<int>(4, 3));
			gfx::Sprite *armour_dealer_sprite = new gfx::Sprite("riverside_armour_dealer");
			armour_dealer->set_sprite(armour_dealer_sprite);
			area->add_entity(armour_dealer);
		}
		else {
			weapons_dealer = area->find_entity("weapons_dealer");

			armour_dealer = area->find_entity("armour_dealer");
		}

		return true;
	}

	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> activator_pos = activator->get_position();
		util::Point<int> weapons_dealer_pos = weapons_dealer->get_position();
		util::Point<int> armour_dealer_pos = armour_dealer->get_position();
		if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && (activator_pos-weapons_dealer_pos) == util::Point<int>(0, 2)) {
			weapons_dealer->get_sprite()->set_animation("stand_s");
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1090)/* Originally: Shopkeep */ + TAG_END, GLOBALS->game_t->translate(1093)/* Originally: I sell weapons. What do you need? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_AXE, 40));
			items.push_back(OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_SHORT_SWORD, 50));
			items.push_back(OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_GHOUL_SWORD, 75));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_WEAPON, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}
		else if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && (activator_pos-armour_dealer_pos) == util::Point<int>(0, 2)) {
			armour_dealer->get_sprite()->set_animation("stand_s");
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1094)/* Originally: Spike */ + TAG_END, GLOBALS->game_t->translate(1095)/* Originally: I'm Spike. Do you need armour? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_ARMOUR, ARMOUR_HELMET, 50));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_ARMOUR, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}

		return false;
	}

private:
	wedge::Map_Entity *weapons_dealer;
	wedge::Map_Entity *armour_dealer;
};

class Area_Hooks_Lumberjacks_Dads : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Lumberjacks_Dads(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(5, 7), util::Size<int>(1, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(40, 14));
		fz1.player_positions.push_back(util::Point<int>(40, 14));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Lumberjacks_Dads()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			util::Point<int> dad_pos(4, 3);
			dad = new wedge::Map_Entity("dad");
			dad->set_position(dad_pos);
			dad->set_wanders(true, true, true, true, dad_pos, 2);
			dad->start(area);
			gfx::Sprite *dad_sprite = new gfx::Sprite("lumberjacks_dad");
			dad->set_sprite(dad_sprite);
			area->add_entity(dad);
		}
		else {
			dad = area->find_entity("dad");
		}


		return true;
	}

	virtual bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activated == dad) {
			Reset_Anim *ra = new Reset_Anim;
			ra->was_moving = dad->is_moving();
			ra->direction = dad->get_direction();
			ra->entity = dad;

			dad->face(AREA->get_player(ENY), false);

			if (INSTANCE->is_milestone_complete(MS_LEARNED_HEAL_FIRE) == false) {
				int num_rings = 0;
				if (INSTANCE->inventory.find(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_RING1, 1)) >= 0) {
					num_rings++;
				}
				if (INSTANCE->inventory.find(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_RING2, 1)) >= 0) {
					num_rings++;
				}
				if (num_rings == 0) {
					NEW_SYSTEM_AND_TASK(AREA)
					wedge::Generic_Callback_Step *step = new wedge::Generic_Callback_Step(reset_anim, ra, new_task);
					ADD_STEP(step)
					ADD_TASK(new_task)
					FINISH_SYSTEM(AREA)

					GLOBALS->do_dialogue(GLOBALS->game_t->translate(1096)/* Originally: Old Man */ + TAG_END, GLOBALS->game_t->translate(1097)/* Originally: My boys were working in the woods when the monsters came. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1098)/* Originally: Please find them for me! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, step);
				}
				else if (num_rings == 1) {
					NEW_SYSTEM_AND_TASK(AREA)
					wedge::Generic_Callback_Step *step = new wedge::Generic_Callback_Step(reset_anim, ra, new_task);
					ADD_STEP(step)
					ADD_TASK(new_task)
					FINISH_SYSTEM(AREA)

					GLOBALS->do_dialogue(GLOBALS->game_t->translate(1096)/* Originally: Old Man */ + TAG_END, GLOBALS->game_t->translate(1100)/* Originally: You found one of my boys in the woods? That's his ring! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1101)/* Originally: Please find my other son! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, step);
				}
				else { // 2
					INSTANCE->set_milestone_complete(MS_LEARNED_HEAL_FIRE, true);
					INSTANCE->inventory.remove(INSTANCE->inventory.find(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_RING1, 1)), 1);
					INSTANCE->inventory.remove(INSTANCE->inventory.find(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_RING2, 1)), 1);
					NEW_SYSTEM_AND_TASK(AREA)
					ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
					ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1096)/* Originally: Old Man */ + TAG_END, GLOBALS->game_t->translate(1103)/* Originally: Woe is me. You were the last people my boys saw before they died. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1104)/* Originally: Would you believe I know magic? */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1105)/* Originally: I was planning on teaching my boys, but I'll teach you instead. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1106)/* Originally: I'll trade you. One spell per ring. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
					ADD_STEP(new wedge::Give_Object_Step(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HEAL_SCROLL, 1), wedge::DIALOGUE_AUTO, new_task))
					ADD_STEP(new wedge::Give_Object_Step(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_FIRE_SCROLL, 1), wedge::DIALOGUE_AUTO, new_task))
					ADD_STEP(new Scroll_Help_Step(GLOBALS->game_t->translate(1096)/* Originally: Old Man */, new_task));
					ADD_STEP(new wedge::Generic_Immediate_Callback_Step(reset_anim, ra, new_task))
					ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
					ADD_TASK(new_task)
					FINISH_SYSTEM(AREA)
				}
			}
			else {
				NEW_SYSTEM_AND_TASK(AREA)
				wedge::Generic_Callback_Step *step = new wedge::Generic_Callback_Step(reset_anim, ra, new_task);
				ADD_STEP(step)
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)

				GLOBALS->do_dialogue(GLOBALS->game_t->translate(1096)/* Originally: Old Man */ + TAG_END, GLOBALS->game_t->translate(1109)/* Originally: I'm going to miss my boys... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, step);
			}
		}
		return false;
	}

private:
	wedge::Map_Entity *dad;
};

class Area_Hooks_Coros : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Coros(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(4, 6), util::Size<int>(1, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(32, 18));
		fz1.player_positions.push_back(util::Point<int>(32, 18));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Coros()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		return true;
	}
};

class Area_Hooks_Moryts : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Moryts(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(4, 6), util::Size<int>(1, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(13, 20));
		fz1.player_positions.push_back(util::Point<int>(13, 20));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Moryts()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			wedge::NPC *moryt = new wedge::NPC("Moryt", "Moryt", "moryt", "moryt");
			moryt->start(area);
			moryt->set_position(util::Point<int>(6, 2));
			area->add_entity(moryt);
			
			wedge::Chest *chest = new wedge::Chest("chest", "", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(6, 4));
			area->add_entity(chest);
		}

		return true;
	}
};

class Area_Hooks_Captains1 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Captains1(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		captain(NULL)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(6, 7), util::Size<int>(1, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(39, 23));
		fz1.player_positions.push_back(util::Point<int>(39, 23));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(1, 1), util::Size<int>(1, 1));
		fz2.area_name = "captains2";
		fz2.player_positions.push_back(util::Point<int>(1, 2));
		fz2.player_positions.push_back(util::Point<int>(1, 2));
		fz2.directions.push_back(wedge::DIR_E);
		fz2.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_Captains1()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			util::Point<int> captain_pos(9, 3);
			captain = new wedge::NPC("Captain", "Captain", "captain", "captain");
			captain->set_position(captain_pos);
			captain->set_wanders(true, true, true, true, captain_pos, 3);
			captain->start(area);
			captain->set_direction(wedge::DIR_S, true, false);
			captain->set_speed(captain->get_speed() * 0.25f);
			area->add_entity(captain);
		}
		else {
			captain = area->find_entity("Captain");
		}

		if (captain) {
			if (INSTANCE->is_milestone_complete(MS_PALLA_LEFT)) {
				area->remove_entity(captain, true);
				captain = NULL;
			}
		}

		return true;
	}

private:
	wedge::Map_Entity *captain;
};

class Area_Hooks_Captains2 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Captains2(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(1, 2), util::Size<int>(1, 1));
		fz1.area_name = "captains1";
		fz1.player_positions.push_back(util::Point<int>(1, 1));
		fz1.player_positions.push_back(util::Point<int>(1, 1));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Captains2()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			wedge::Chest *chest = new wedge::Chest("chest", "", 100);
			chest->start(area);
			chest->set_position(util::Point<int>(11, 2));
			area->add_entity(chest);
		}

		return true;
	}

	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> activator_pos = activator->get_position();
		if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && activator_pos == util::Point<int>(4, 3)) {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1726)/* Originally: Charts and maps... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
			return true;
		}

		return false;
	}
};

class Area_Hooks_Womans : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Womans(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(5, 8), util::Size<int>(1, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(5, 14));
		fz1.player_positions.push_back(util::Point<int>(5, 14));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Womans()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/village.mml");

		if (loaded == false) {
			util::Point<int> woman_pos(6, 3);
			wedge::NPC *woman = new wedge::NPC("Woman", "Woman", "woman", "woman");
			woman->set_position(woman_pos);
			woman->set_wanders(true, true, true, true, woman_pos, 4);
			woman->start(area);
			woman->set_direction(wedge::DIR_S, true, false);
			area->add_entity(woman);

			wedge::NPC *girl = new wedge::NPC("Girl", "Girl", "littlegirl", "littlegirl");
			girl->set_position(util::Point<int>(1, 2));
			girl->start(area);
			girl->set_direction(wedge::DIR_S, true, false);
			area->add_entity(girl);
		}


		return true;
	}
};

class Area_Hooks_Palace1 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Palace1(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(9, 19), util::Size<int>(2, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(22, 19));
		fz1.player_positions.push_back(util::Point<int>(23, 19));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(1, 10), util::Size<int>(1, 1));
		fz2.area_name = "palaceb1";
		fz2.player_positions.push_back(util::Point<int>(11, 2));
		fz2.player_positions.push_back(util::Point<int>(11, 2));
		fz2.directions.push_back(wedge::DIR_S);
		fz2.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz2);

		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(18, 10), util::Size<int>(1, 1));
		fz3.area_name = "palaceb2";
		fz3.player_positions.push_back(util::Point<int>(1, 2));
		fz3.player_positions.push_back(util::Point<int>(1, 2));
		fz3.directions.push_back(wedge::DIR_S);
		fz3.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz3);

		Fade_Zone fz4;
		fz4.zone = util::Rectangle<int>(util::Point<int>(9, 3), util::Size<int>(2, 1));
		fz4.area_name = "palace2";
		fz4.player_positions.push_back(util::Point<int>(7, 14));
		fz4.player_positions.push_back(util::Point<int>(8, 14));
		fz4.directions.push_back(wedge::DIR_N);
		fz4.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz4);
	}
	
	virtual ~Area_Hooks_Palace1()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/palace.mml");

		if (loaded == false) {
			wedge::NPC *soldier1 = new wedge::NPC("soldier1", "Soldier", "soldier", "soldier_palace1-1");
			soldier1->start(area);
			soldier1->set_position(util::Point<int>(7, 5));
			soldier1->set_direction(wedge::DIR_S, true, false);
			area->add_entity(soldier1);
			
			wedge::NPC *soldier2 = new wedge::NPC("soldier2", "Soldier", "soldier", "soldier_palace1-2");
			soldier2->start(area);
			soldier2->set_position(util::Point<int>(12, 5));
			soldier2->set_direction(wedge::DIR_S, true, false);
			area->add_entity(soldier2);
			
			wedge::NPC *soldier3 = new wedge::NPC("soldier3", "Soldier", "soldier", "soldier_palace1-3");
			soldier3->start(area);
			soldier3->set_position(util::Point<int>(7, 10));
			soldier3->set_direction(wedge::DIR_S, true, false);
			area->add_entity(soldier3);
			
			wedge::NPC *soldier4 = new wedge::NPC("soldier4", "Soldier", "soldier", "soldier_palace1-4");
			soldier4->start(area);
			soldier4->set_position(util::Point<int>(12, 10));
			soldier4->set_direction(wedge::DIR_S, true, false);
			area->add_entity(soldier4);
		}

		return true;
	}

	void started()
	{
		// fadeout colour might have been changed for palla leave scene, change it back
		static_cast<Monster_RPG_3_Area_Game *>(AREA)->set_next_fadeout_colour(shim::black);

		Area_Hooks_Monster_RPG_3::started();
	}
};

class Area_Hooks_PalaceB1 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_PalaceB1(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(11, 2), util::Size<int>(1, 1));
		fz1.area_name = "palace1";
		fz1.player_positions.push_back(util::Point<int>(1, 10));
		fz1.player_positions.push_back(util::Point<int>(1, 10));
		fz1.directions.push_back(wedge::DIR_E);
		fz1.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_PalaceB1()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/palace.mml");

		if (loaded == false) {	
			scroll_dealer = new wedge::Map_Entity("Scroll Dealer");
			scroll_dealer->start(area);
			scroll_dealer->set_position(util::Point<int>(6, 4));
			scroll_dealer->set_sprite(new gfx::Sprite("scroll_dealer"));
			area->add_entity(scroll_dealer);
		}
		else {
			scroll_dealer = area->find_entity("Scroll Dealer");
		}

		return true;
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data torch_anim;
		torch_anim.topleft = util::Point<int>(0, 14);
		torch_anim.size = util::Size<int>(1, 1);
		torch_anim.delay = 32;
		torch_anim.frames.push_back(util::Point<int>(1, 14));
		torch_anim.frames.push_back(util::Point<int>(2, 14));
		torch_anim.frames.push_back(util::Point<int>(3, 14));
		tilemap->add_animation_data(torch_anim);
	}
	
	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			SDL_Colour colour;
			colour.r = shim::palette[11].r * 0.25f;
			colour.g = shim::palette[11].g * 0.25f;
			colour.b = shim::palette[11].b * 0.25f;
			colour.a = 0.25f * 255;
			gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
			std::vector< util::Point<float> > beams;
			std::vector< util::Point<float> > torches;
			torches.push_back(util::Point<float>(4, 3));
			torches.push_back(util::Point<float>(8, 3));
			draw_torches(shim::palette[11], map_offset, torches, beams, false);
		}
	}
	
	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activated == scroll_dealer) {
			bool has_cure = false;
			bool has_heal_plus = false;
			wedge::Object o = OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_CURE_SCROLL, 1);
			int index = INSTANCE->inventory.find(o);
			if (index < 0) {
				std::vector<std::string> spells1 = INSTANCE->stats[ENY].base.get_spells();
				std::vector<std::string> spells2 = INSTANCE->stats[TIGGY].base.get_spells();
				has_cure = std::find(spells1.begin(), spells1.end(), "Cure") != spells1.end() ||
					std::find(spells2.begin(), spells2.end(), "Cure") != spells2.end();
			}
			else {
				has_cure = true;
			}
			o = OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HEAL_PLUS_SCROLL, 1);
			index = INSTANCE->inventory.find(o);
			if (index < 0) {
				std::vector<std::string> spells1 = INSTANCE->stats[ENY].base.get_spells();
				std::vector<std::string> spells2 = INSTANCE->stats[TIGGY].base.get_spells();
				has_heal_plus = std::find(spells1.begin(), spells1.end(), "Heal Plus") != spells1.end() ||
					std::find(spells2.begin(), spells2.end(), "Heal Plus") != spells2.end();
			}
			else {
				has_heal_plus = true;
			}
			if (has_cure) {
				if (has_heal_plus) {
					GLOBALS->do_dialogue(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1554)/* Originally: There's nothing more I can teach you... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
				}
				else {
					if (INSTANCE->is_milestone_complete(MS_BEAT_PALLA)) {
						NEW_SYSTEM_AND_TASK(AREA)
						Buy_Scroll_Step *bss = new Buy_Scroll_Step(ITEM_HEAL_PLUS_SCROLL, 250, new_task);
						ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
						ADD_STEP(bss)
						ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
						ADD_TASK(new_task)
						FINISH_SYSTEM(AREA)

						std::vector<std::string> choices;
						choices.push_back(GLOBALS->game_t->translate(1729)/* Originally: Yes */);
						choices.push_back(GLOBALS->game_t->translate(1730)/* Originally: No */);
						do_question(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1115)/* Originally: Would you like to buy a Heal Plus Scroll? Only 250 gold! */, wedge::DIALOGUE_SPEECH, choices, bss);
					}
					else {
						GLOBALS->do_dialogue(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1553)/* Originally: I'll have more inventory soon. Check back again sometime. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
					}
				}
			}
			else {
				NEW_SYSTEM_AND_TASK(AREA)
				Buy_Scroll_Step *bss = new Buy_Scroll_Step(ITEM_CURE_SCROLL, 100, new_task);
				ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
				ADD_STEP(bss)
				ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)

				std::vector<std::string> choices;
				choices.push_back(GLOBALS->game_t->translate(1729)/* Originally: Yes */);
				choices.push_back(GLOBALS->game_t->translate(1730)/* Originally: No */);
				do_question(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1120)/* Originally: Would you like to buy a Cure Scroll? Only 100 gold! */, wedge::DIALOGUE_SPEECH, choices, bss);
			}
			return true;
		}
		return false;
	}

private:

	wedge::Map_Entity *scroll_dealer;
};

class Area_Hooks_PalaceB2 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_PalaceB2(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(1, 2), util::Size<int>(1, 1));
		fz1.area_name = "palace1";
		fz1.player_positions.push_back(util::Point<int>(18, 10));
		fz1.player_positions.push_back(util::Point<int>(18, 10));
		fz1.directions.push_back(wedge::DIR_W);
		fz1.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_PalaceB2()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/palace.mml");

		if (loaded == false) {
			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_SHANK, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(8, 25));
			area->add_entity(chest);
			
			wedge::NPC *soldier = new wedge::NPC("soldier", "Soldier", "soldier", "soldier_palaceb2");
			soldier->start(area);
			soldier->set_position(util::Point<int>(4, 5));
			soldier->set_direction(wedge::DIR_S, true, false);
			area->add_entity(soldier);

			util::Point<int> prisoner1_pos(6, 3);
			prisoner1 = new wedge::Map_Entity("prisoner1");
			prisoner1->set_wanders(true, true, false, false, prisoner1_pos, 1);
			prisoner1->start(area);
			prisoner1->set_position(prisoner1_pos);
			gfx::Sprite *prisoner1_sprite = new gfx::Sprite("prisoner1");
			prisoner1->set_sprite(prisoner1_sprite);
			prisoner1->set_speed(prisoner1->get_speed() * 0.5f);
			area->add_entity(prisoner1);

			util::Point<int> prisoner2_pos(10, 3);
			prisoner2 = new wedge::Map_Entity("prisoner2");
			prisoner2->set_wanders(true, true, false, false, prisoner2_pos, 1);
			prisoner2->start(area);
			prisoner2->set_position(prisoner2_pos);
			gfx::Sprite *prisoner2_sprite = new gfx::Sprite("prisoner2");
			prisoner2->set_sprite(prisoner2_sprite);
			prisoner2->set_speed(prisoner2->get_speed() * 0.5f);
			area->add_entity(prisoner2);
		}
		else {
			prisoner1 = area->find_entity("prisoner1");
			prisoner2 = area->find_entity("prisoner2");
		}

		return true;
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data torch_anim;
		torch_anim.topleft = util::Point<int>(0, 14);
		torch_anim.size = util::Size<int>(1, 1);
		torch_anim.delay = 32;
		torch_anim.frames.push_back(util::Point<int>(1, 14));
		torch_anim.frames.push_back(util::Point<int>(2, 14));
		torch_anim.frames.push_back(util::Point<int>(3, 14));
		tilemap->add_animation_data(torch_anim);
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			SDL_Colour colour;
			colour.r = shim::palette[11].r * 0.25f;
			colour.g = shim::palette[11].g * 0.25f;
			colour.b = shim::palette[11].b * 0.25f;
			colour.a = 0.25f * 255;
			gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
			std::vector< util::Point<float> > beams;
			std::vector< util::Point<float> > torches;
			torches.push_back(util::Point<float>(4, 3));
			torches.push_back(util::Point<float>(8, 3));
			torches.push_back(util::Point<float>(12, 3));
			draw_torches(shim::palette[11], map_offset, torches, beams, false);
		}
	}
	
	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> activator_pos = activator->get_position();
		util::Point<int> prisoner1_pos = prisoner1->get_position();
		util::Point<int> prisoner2_pos = prisoner2->get_position();
		if (activator == AREA->get_player(ENY)) {
			if (activator->get_direction() == wedge::DIR_N) {
				if ((activator_pos-prisoner1_pos) == util::Point<int>(0, 2)) {
					Reset_Anim *ra = new Reset_Anim;
					ra->was_moving = prisoner1->is_moving();
					ra->direction = prisoner1->get_direction();
					ra->entity = prisoner1;
				
					NEW_SYSTEM_AND_TASK(AREA)
					wedge::Generic_Callback_Step *step = new wedge::Generic_Callback_Step(reset_anim, ra, new_task);
					ADD_STEP(step)
					ADD_TASK(new_task)
					FINISH_SYSTEM(AREA)

					prisoner1->set_direction(wedge::DIR_S, true, false);
					GLOBALS->do_dialogue(GLOBALS->game_t->translate(1121)/* Originally: Prisoner */ + TAG_END, GLOBALS->game_t->translate(1122)/* Originally: I could get outta here anytime I want... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, step);
					return true;
				}
				else if ((activator_pos-prisoner2_pos) == util::Point<int>(0, 2)) {
					Reset_Anim *ra = new Reset_Anim;
					ra->was_moving = prisoner2->is_moving();
					ra->direction = prisoner2->get_direction();
					ra->entity = prisoner2;
				
					NEW_SYSTEM_AND_TASK(AREA)
					wedge::Generic_Callback_Step *step = new wedge::Generic_Callback_Step(reset_anim, ra, new_task);
					ADD_STEP(step)
					ADD_TASK(new_task)
					FINISH_SYSTEM(AREA)

					prisoner2->set_direction(wedge::DIR_S, true, false);
					GLOBALS->do_dialogue(GLOBALS->game_t->translate(1121)/* Originally: Prisoner */ + TAG_END, GLOBALS->game_t->translate(1124)/* Originally: Want to meet my pet rat? Hahaha! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, step);
					return true;
				}
			}
		}
		return false;
	}

private:
	wedge::Map_Entity *prisoner1;
	wedge::Map_Entity *prisoner2;
};

class Area_Hooks_Palace2 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Palace2(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;

		fz1.zone = util::Rectangle<int>(util::Point<int>(7, 14), util::Size<int>(2, 1));
		if (INSTANCE->is_milestone_complete(MS_MAYOR_REPENTED) == false && INSTANCE->is_milestone_complete(MS_BEAT_PALLA) == true) {
			fz1.area_name = "hh";
		}
		else {
			fz1.area_name = "palace1";
		}
		fz1.player_positions.push_back(util::Point<int>(9, 3));
		fz1.player_positions.push_back(util::Point<int>(10, 3));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Palace2()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/palace.mml");

		if (loaded == false) {
			wedge::NPC *soldier1 = new wedge::NPC("soldier1", "Soldier", "soldier", "soldier_palace2-1");
			soldier1->start(area);
			soldier1->set_position(util::Point<int>(3, 8));
			soldier1->set_direction(wedge::DIR_S, true, false);
			area->add_entity(soldier1);
			
			wedge::NPC *soldier2 = new wedge::NPC("soldier2", "Soldier", "soldier", "soldier_palace2-2");
			soldier2->start(area);
			soldier2->set_position(util::Point<int>(12, 8));
			soldier2->set_direction(wedge::DIR_S, true, false);
			area->add_entity(soldier2);
			
			mayor = new wedge::NPC("mayor", "Mayor", "mayor", "mayor");
			mayor->start(area);
			mayor->set_position(util::Point<int>(5, 5));
			mayor->set_direction(wedge::DIR_S, true, false);
			area->add_entity(mayor);
		}
		else {
			mayor = area->find_entity("mayor");
		}

		return true;
	}

	void started()
	{
		if (INSTANCE->is_milestone_complete(MS_TALKED_TO_MAYOR) == false) {
			INSTANCE->set_milestone_complete(MS_TALKED_TO_MAYOR, true);

			wedge::Map_Entity *eny = AREA->get_player(ENY);
			wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);

			eny->get_input_step()->set_stash(false, false, false, false);

			INSTANCE->party_following_player = false;

			util::Point<int> eny_walk_pos(5, 7);
			util::Point<int> tiggy_walk_pos(6, 7);
			
			NEW_SYSTEM_AND_TASK(AREA)

			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new wedge::A_Star_Step(eny, eny_walk_pos, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(eny);
			entities.push_back(tiggy);
			std::vector< util::Point<int> > positions;
			positions.push_back(eny_walk_pos);
			positions.push_back(tiggy_walk_pos);
			ADD_STEP(new wedge::Check_Positions_Step(entities, positions, true, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1126)/* Originally: You wanted to see us, sir? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1128)/* Originally: Ahhh, Eny and Tiggy.^Allow me to get this important matter out of the way first...^Why are you barefoot??? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1130)/* Originally: We were collecting fiddleheads, sir. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1132)/* Originally: Ahhh, that explains it... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1134)/* Originally: Well, as you know, there has been a monster outbreak.^My guards really are just for show... sure, they can handle Level 1 monsters, but anything beyond that and they're useless.^Seeing as you two are the only heroes in town with real experience, I have a mission for you. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1136)/* Originally: We don't do \"missions\" anymore, sir. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1138)/* Originally: Do you do \"prisons\"? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1140)/* Originally: What was that mission, sir? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1142)/* Originally: I thought so.^We are all familiar with the haunted mansion across the river in the South East, no?^I have reason to believe it is the source of the outbreak.^Once you've made preparations, head there and put an end to this madness. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1144)/* Originally: But isn't it... haunted? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1146)/* Originally: I hear the prison has a few ghosts of its own... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1148)/* Originally: Very well sir, we'll be on our way soon... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1150)/* Originally: Excellent.^Do stop by the basement and see if the Scroll Dealer has anything you can use before you go... */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1151)/* Originally: And bring plenty of HOLY WATER and ELIXIR. It's a tad dangerous. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(set_following, (void *)1, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			ADD_STEP(new wedge::A_Star_Step(tiggy, tiggy_walk_pos, new_task))
			ADD_TASK(new_task)

			FINISH_SYSTEM(AREA)
		}
		else if (INSTANCE->is_milestone_complete(MS_MAYOR_REPENTED) == false && INSTANCE->is_milestone_complete(MS_BEAT_PALLA) == true) {
			INSTANCE->set_milestone_complete(MS_MAYOR_REPENTED, true);

			M3_INSTANCE->add_vampire("vBolt");

			wedge::Map_Entity *eny = AREA->get_player(ENY);
			wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);

			eny->get_input_step()->set_stash(false, false, false, false);

			INSTANCE->party_following_player = false;

			gfx::Sprite *eny_sprite = eny->get_sprite();
			gfx::Sprite *tiggy_sprite = tiggy->get_sprite();
			gfx::Sprite *mayor_sprite = mayor->get_sprite();

			util::Point<int> eny_walk_pos(5, 7);
			util::Point<int> tiggy_walk_pos(6, 7);
			
			NEW_SYSTEM_AND_TASK(AREA)

			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new wedge::A_Star_Step(eny, eny_walk_pos, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(eny);
			entities.push_back(tiggy);
			std::vector< util::Point<int> > positions;
			positions.push_back(eny_walk_pos);
			positions.push_back(tiggy_walk_pos);
			ADD_STEP(new wedge::Check_Positions_Step(entities, positions, true, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1153)/* Originally: You're back! I trust you've put an end to the menace? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1155)/* Originally: The only menace here is YOU! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1157)/* Originally: We met your brother. He told us all about the Pendant you're using to drain his power! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1159)/* Originally: Well... heh, uh, I wouldn't say drain. *gulp* */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1160)/* Originally: Think of it as borrowing! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			Dialogue_Step *d1 = new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1162)/* Originally: You little snake! How could you do that to your own brother? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task);
			ADD_STEP(d1)
			wedge::A_Star_Step *astar = new wedge::A_Star_Step(eny, util::Point<int>(4, 7), new_task);
			ADD_STEP(astar)
			ADD_STEP(new wedge::Play_Animation_Step(eny_sprite, "stand_s", new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			ADD_STEP(new wedge::A_Star_Step(tiggy, tiggy_walk_pos, new_task))
			wedge::Wait_Step *wait1 = new wedge::Wait_Step(new_task);
			astar->add_monitor(wait1);
			ADD_STEP(wait1)
			ADD_STEP(new wedge::Play_Animation_Step(tiggy_sprite, "stand_s", new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			wedge::Wait_Step *wait2 = new wedge::Wait_Step(new_task);
			d1->add_monitor(wait2);
			ADD_STEP(wait2)
			ADD_STEP(new wedge::A_Star_Step(mayor, util::Point<int>(5, 8), new_task))
			ADD_STEP(new wedge::Play_Animation_Step(mayor_sprite, "cry", new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1164)/* Originally: Waaa, why me? I only wanted to be mayor! Is that too much to ask? I never got to do anything my whole life! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1166)/* Originally: Hold up! You are NOT the victim here! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1167)/* Originally: And none of this changes the fact that there are MONSTERS roaming around again! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1169)/* Originally: Ohhh my poor soul! What do you want from me? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1171)/* Originally: We'll be needing a ship... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1173)/* Originally: Fiiine... tell the Captain his ship is at your mercy! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1174)/* Originally: Take whatever you need! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1176)/* Originally: You got it! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::A_Star_Step(tiggy, util::Point<int>(6, 8), new_task))
			ADD_STEP(new wedge::Play_Animation_Step(tiggy_sprite, "stand_w", new_task))
			ADD_STEP(new wedge::Give_Object_Step(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_VAMPIRE2, 1), wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1178)/* Originally: Be careful with that! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1179)/* Originally: Whoever uses it drains the power of the person wearing the other one! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1181)/* Originally: Yeah, we kind of got that. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Play_Sound_Step(GLOBALS->levelup, false, false, new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(achieve_vampire, NULL, new_task))
			ADD_STEP(new Dialogue_Step("", GLOBALS->game_t->translate(1182)/* Originally: Learned vBolt! */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step("", GLOBALS->game_t->translate(1183)/* Originally: (Use Vampires in battle to deal big damage!) */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1185)/* Originally: Where did you find these anyway? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(90)/* Originally: Mayor */ + TAG_END, GLOBALS->game_t->translate(1187)/* Originally: In a cave. *sniff* */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1188)/* Originally: In the mountains in the Eastern Kingdom... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1190)/* Originally: Maybe we'll check it out while we're there. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_TOP, new_task))
			ADD_STEP(new wedge::A_Star_Step(tiggy, util::Point<int>(4, 6), new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(set_following, (void *)1, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)

			FINISH_SYSTEM(AREA)
		}

		Area_Hooks_Monster_RPG_3::started();
	}

private:
	wedge::Map_Entity *mayor;
};

static void end_palla_leave(void *data);

class Area_Hooks_HH : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_HH(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		palla(NULL)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(3, 0), util::Size<int>(8, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(31, 51));
		fz1.player_positions.push_back(util::Point<int>(32, 51));
		fz1.directions.push_back(wedge::DIR_N);
		fz1.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz1);
		
		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(67, 41), util::Size<int>(2, 1));
		fz2.area_name = "hh1";
		fz2.player_positions.push_back(util::Point<int>(24, 30));
		fz2.player_positions.push_back(util::Point<int>(25, 30));
		fz2.directions.push_back(wedge::DIR_N);
		fz2.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz2);
		
		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(86, 13), util::Size<int>(1, 1));
		fz3.area_name = "hh3";
		fz3.player_positions.push_back(util::Point<int>(49, 1));
		fz3.player_positions.push_back(util::Point<int>(49, 1));
		fz3.directions.push_back(wedge::DIR_S);
		fz3.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz3);
		
		Fade_Zone fz4;
		fz4.zone = util::Rectangle<int>(util::Point<int>(49, 13), util::Size<int>(1, 1));
		fz4.area_name = "hh_stairs";
		fz4.player_positions.push_back(util::Point<int>(1, 1));
		fz4.player_positions.push_back(util::Point<int>(1, 1));
		fz4.directions.push_back(wedge::DIR_S);
		fz4.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz4);
		
		Fade_Zone fz5;
		fz5.zone = util::Rectangle<int>(util::Point<int>(67, 10), util::Size<int>(2, 1));
		fz5.area_name = "hh_basement";
		fz5.player_positions.push_back(util::Point<int>(19, 1));
		fz5.player_positions.push_back(util::Point<int>(20, 1));
		fz5.directions.push_back(wedge::DIR_S);
		fz5.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz5);
		
		Fade_Zone fz6;
		fz6.zone = util::Rectangle<int>(util::Point<int>(67, 31), util::Size<int>(2, 1));
		fz6.area_name = "hh3";
		fz6.player_positions.push_back(util::Point<int>(25, 28));
		fz6.player_positions.push_back(util::Point<int>(25, 28));
		fz6.directions.push_back(wedge::DIR_N);
		fz6.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz6);
		
		Fade_Zone fz7;
		fz7.zone = util::Rectangle<int>(util::Point<int>(56, 36), util::Size<int>(2, 1));
		fz7.area_name = "hh2";
		fz7.player_positions.push_back(util::Point<int>(12, 28));
		fz7.player_positions.push_back(util::Point<int>(13, 28));
		fz7.directions.push_back(wedge::DIR_N);
		fz7.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz7);
		
		Fade_Zone fz8;
		fz8.zone = util::Rectangle<int>(util::Point<int>(78, 36), util::Size<int>(2, 1));
		fz8.area_name = "hh2";
		fz8.player_positions.push_back(util::Point<int>(37, 28));
		fz8.player_positions.push_back(util::Point<int>(38, 28));
		fz8.directions.push_back(wedge::DIR_N);
		fz8.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz8);
	}
	
	virtual ~Area_Hooks_HH()
	{
		static_cast<Monster_RPG_3_Area_Game *>(AREA)->set_use_camera(false);
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/hh.mml");

		if (loaded == false) {
			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_BOLT_SCROLL, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(86, 32));
			area->add_entity(chest);
		}

		if (area->find_entity("chest2") == NULL) {
			wedge::Chest *chest2 = new wedge::Chest("chest2", "chest", OBJECT->make_object(wedge::OBJECT_ARMOUR, ARMOUR_RUSTY_CHAIN_MAIL, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(86, 43));
			area->add_entity(chest2);
		}

		if (area->find_entity("chest3") == NULL) {
			wedge::Chest *chest3 = new wedge::Chest("chest3", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 1));
			chest3->start(area);
			chest3->set_position(util::Point<int>(49, 21));
			area->add_entity(chest3);
		}

		if (INSTANCE->is_milestone_complete(MS_MAYOR_REPENTED) == true && INSTANCE->is_milestone_complete(MS_PALLA_LEFT) == false) {
			INSTANCE->set_milestone_complete(MS_PALLA_LEFT, true);
			palla = new wedge::Map_Entity("palla");
			palla->start(area);
			palla->set_position(util::Point<int>(67, 41));
			palla->set_sprite(new gfx::Sprite("palla"));
			palla->set_direction(wedge::DIR_S, true, false);
			area->add_entity(palla);
		}

		return true;
	}

	void started()
	{
		if (palla) {
			static_cast<Monster_RPG_3_Area_Game *>(AREA)->set_use_camera(true);
			centre_on_palla();
		}

		// change some solids for a little better feel if player starting in backyard
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		if (eny->get_position() == util::Point<int>(67, 9)) {
			gfx::Tilemap *tilemap = area->get_tilemap();
			tilemap->set_solid(-1, util::Point<int>(49, 13), util::Size<int>(86-49+1, 1), true);
			tilemap->set_solid(-1, util::Point<int>(49, 12), util::Size<int>(65-49+1, 1), false);
			tilemap->set_solid(-1, util::Point<int>(70, 12), util::Size<int>(86-70+1, 1), false);
		}

		if (palla) {
			gfx::Sprite *sprite = palla->get_sprite();
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(palla, util::Point<int>(67, 42), new_task));
			ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->jump, false, false, new_task))
			ADD_STEP(new wedge::Play_Animation_Step(sprite, "jump", new_task));
			ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->jump, false, false, new_task))
			ADD_STEP(new wedge::Play_Animation_Step(sprite, "jump", new_task));
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1192)/* Originally: Woot woot! Good as new! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(::end_palla_leave, this, new_task))
			ADD_STEP(new wedge::A_Star_Step(palla, util::Point<int>(67, 50), new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
		}

		GLOBALS->max_battle_steps = Monster_RPG_3_Globals::MAX_BATTLE_STEPS;
		GLOBALS->min_battle_steps = Monster_RPG_3_Globals::MIN_BATTLE_STEPS;

		Area_Hooks_Monster_RPG_3::started();
	}

	void end()
	{
		if (palla) {
			area->remove_entity(palla, true);
			palla = NULL;
		}
	}

	void run()
	{
		centre_on_palla();
	}
	
	bool try_tile(wedge::Map_Entity *entity, util::Point<int> tile_pos)
	{
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);

		if (entity != eny) {
			return false;
		}

		if (tile_pos == util::Point<int>(49, 7)) {
			INSTANCE->party_following_player = false;

			wedge::pause_presses(true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Set_Solid_Step(eny, false, new_task))
			ADD_STEP(new wedge::A_Star_Step(eny, util::Point<int>(50, 7), new_task))
			ADD_STEP(new Jump_Step(eny, util::Point<float>(-2, -1), 5, util::Point<int>(47, 7), new_task))
			ADD_STEP(new wedge::A_Star_Step(eny, util::Point<int>(47, 8), new_task))
			ADD_STEP(new wedge::Set_Solid_Step(eny, true, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, true, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			ADD_STEP(new wedge::Set_Solid_Step(tiggy, false, new_task))
			ADD_STEP(new wedge::A_Star_Step(tiggy, util::Point<int>(50, 7), new_task))
			ADD_STEP(new Jump_Step(tiggy, util::Point<float>(-2, -1), 5, util::Point<int>(47, 7), new_task))
			ADD_STEP(new wedge::Set_Solid_Step(tiggy, true, new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(set_following, (void *)1, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			return true;
		}
		else {
			return false;
		}
	}

	void end_palla_leave()
	{
		std::map<std::string, std::string>::iterator it;
		SDL_RWops *file = NULL;
		util::JSON *json = NULL;
		wedge::Area *basement = NULL;
		if ((it = INSTANCE->saved_levels.find("hh_basement")) != INSTANCE->saved_levels.end()) {
			area->disconnect_player_input_step();
			std::pair<std::string, std::string> p = *it;
			std::string save = "{ " + p.second + " }";
			file = SDL_RWFromMem((void *)save.c_str(), (int)save.length());
			json = new util::JSON(file);
			util::JSON::Node *n = json->get_root()->find("\"hh_basement\"");
			basement = AREA->create_area(n);
			basement->start();
			basement->disconnect_player_input_step();
		}

		if (basement != NULL) {
			wedge::Map_Entity *palla = basement->find_entity("palla");
			basement->remove_entity(palla, true);
			INSTANCE->saved_levels["hh_basement"] =	"\"hh_basement\": " + basement->save(false);
			delete json;
			SDL_RWclose(file);
			delete basement;

			// loading a new area disconnect and reset the player's movement system, so restore it
			wedge::Map_Entity_Input_Step *meis = AREA->get_player(0)->get_input_step();
			if (meis) {
				meis->reset(area->get_entity_movement_system());
			}
		}

		// Copied these numbers directly from Palace2 fade zone, goes back to before the palla leave scene started
		std::vector< util::Point<int> > positions;
		positions.push_back(util::Point<int>(9, 3));
		positions.push_back(util::Point<int>(10 ,3));
		std::vector<wedge::Direction> directions;
		directions.push_back(wedge::DIR_S);
		directions.push_back(wedge::DIR_S);
		static_cast<Monster_RPG_3_Area_Game *>(AREA)->set_next_fadeout_colour(shim::white);
		AREA->set_pause_entity_movement_on_next_area_change(false);
		area->set_next_area("palace1", positions, directions);
	}

private:
	void centre_on_palla()
	{
		if (palla) {
			util::Point<int> palla_pos = palla->get_position();
			util::Point<float> palla_offset = palla->get_offset();
			util::Size<int> palla_size = palla->get_size();
			util::Point<float> sz(palla_size.w / 2.0f, 1.0f - palla_size.h / 2.0f);
			wedge::add_tiles(palla_pos, palla_offset, sz);
			util::Point<float> curr_offset = area->get_centred_offset(palla_pos, palla_offset, true);
			static_cast<Monster_RPG_3_Area_Game *>(AREA)->set_camera(curr_offset);
		}
	}

	wedge::Map_Entity *palla;
};

static void end_palla_leave(void *data)
{
	Area_Hooks_HH *hooks = static_cast<Area_Hooks_HH *>(data);
	hooks->end_palla_leave();
}

static gfx::Image *gen_hh_darkness(int seed, float alpha1, float alpha2, SDL_Colour tint)
{
	// set a seed so it's always the same
	util::srand(seed);

	// initial size
	int w = 32;
	int h = 16;
	// double the size reps times
	int reps = 4;
	std::vector< std::vector< int > > v(h, std::vector<int>(w, 0));
	// start cell random (everything is stretched twice horizontally for a better aspect)
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w/2; x++) {
			int r = util::rand(0, 1);
			v[y][x*2] = r;
			v[y][x*2+1] = r; // repeat so it has a nice wide look
		}
	}
	// make it tilable by randomly copying tiles on the borders
	int r = util::rand(0, 3);
	int val;
	if (r == 0) {
		val = v[0][0];
	}
	else if (r == 1) {
		val = v[0][w-1];
	}
	else if (r == 3) {
		val = v[h-1][w-1];
	}
	else {
		val = v[h-1][0];
	}
	v[0][0] = v[0][w-1] = v[h-1][w-1] = v[h-1][0] = val;
	for (int y = 1; y < h-1; y++) {
		int val;
		if (util::rand(0, 1) == 0) {
			val = v[y][0];
		}
		else {
			val = v[y][w-1];
		}
		v[y][0] = v[y][w-1] = val;
	}
	for (int x = 1; x < w-1; x++) {
		int val;
		if (util::rand(0, 1) == 0) {
			val = v[0][x];
		}
		else {
			val = v[h-1][x];
		}
		v[0][x] = v[h-1][x] = val;
	}
	// expand the size, getting an average of surrounding tiles each time
	for (int i = 0; i < reps; i++) {
		int new_w = w * 2;
		int new_h = h * 2;
		std::vector< std::vector<int> > v2(new_h, std::vector<int>(new_w, 0));
		for (int y = 0; y < new_h; y++) {
			for (int x = 0; x < new_w; x++) {
				int y1 = ((y - 1 + new_h) % new_h) / 2;
				int y2 = y / 2;
				int y3 = ((y + 1) % new_h) / 2;
				int x1 = ((x - 1 + new_w) % new_w) / 2;
				int x2 = x / 2;
				int x3 = ((x + 1) % new_w) / 2;
				int val = 0;
				val += v[y1][x1];
				val += v[y1][x2];
				val += v[y1][x3];
				val += v[y2][x1];
				val += v[y2][x2];
				val += v[y2][x3];
				val += v[y3][x1];
				val += v[y3][x2];
				val += v[y3][x3];
				v2[y][x] = val;
			}
		}
		v = v2;
		w = new_w;
		h = new_h;
	}

	// reset the seed
	util::srand((uint32_t)time(NULL));

	Uint8 *data = new Uint8[w*h*4];
	Uint8 *p = data;

	// number of bands of colour to use to give a 16-bit look
	const int ncolours = 3;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int val = v[y][x];
			float f = val / (powf(9, reps)+1);
			int c = f * ncolours;
			int a = c * (255/(ncolours-1));
			// alpha1 and alpha2 control how dark the bands are
			a *= alpha1;
			a += alpha2 * 255;
			float af = a / 255.0f;
			SDL_Colour colour;
			colour.r = tint.r * af;
			colour.g = tint.g * af;
			colour.b = tint.b * af;
			colour.a = a;
			*p++ = colour.r;
			*p++ = colour.g;
			*p++ = colour.b;
			*p++ = colour.a;
		}
	}

	gfx::Image *img = new gfx::Image(data, util::Size<int>(w, h), true);

	return img;
}

class Area_Hooks_HH1 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_HH1(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(24, 30), util::Size<int>(2, 1));
		fz1.area_name = "hh";
		fz1.player_positions.push_back(util::Point<int>(67, 41));
		fz1.player_positions.push_back(util::Point<int>(68, 41));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
		
		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(48, 1), util::Size<int>(1, 1));
		fz2.area_name = "hh2";
		fz2.player_positions.push_back(util::Point<int>(49, 2));
		fz2.player_positions.push_back(util::Point<int>(49, 2));
		fz2.directions.push_back(wedge::DIR_W);
		fz2.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz2);
		
		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(1, 1), util::Size<int>(1, 1));
		fz3.area_name = "hh2";
		fz3.player_positions.push_back(util::Point<int>(1, 2));
		fz3.player_positions.push_back(util::Point<int>(1, 2));
		fz3.directions.push_back(wedge::DIR_E);
		fz3.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz3);
		
		Fade_Zone fz4;
		fz4.zone = util::Rectangle<int>(util::Point<int>(3, 2), util::Size<int>(1, 1));
		fz4.area_name = "hh_basement";
		fz4.player_positions.push_back(util::Point<int>(3, 1));
		fz4.player_positions.push_back(util::Point<int>(3, 1));
		fz4.directions.push_back(wedge::DIR_S);
		fz4.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz4);
	}
	
	virtual ~Area_Hooks_HH1()
	{
		delete darkness_image1;
		delete darkness_image2;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/hh.mml");

		darkness_image1 = gen_hh_darkness(1, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(2, 0.5f, 0.0f, shim::black);
		darkness_offset1 = util::Point<float>(0, 0);
		darkness_offset2 = util::Point<float>(128, 64);

		if (loaded == false) {
			wedge::Chest *chest1 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 3));
			chest1->start(area);
			chest1->set_position(util::Point<int>(7, 5));
			area->add_entity(chest1);

			wedge::Chest *chest2 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(9, 5));
			area->add_entity(chest2);

			wedge::Chest *chest3 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 2));
			chest3->start(area);
			chest3->set_position(util::Point<int>(11, 5));
			area->add_entity(chest3);

			wedge::Chest *chest4 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ARMOUR, ARMOUR_CHAIN_MAIL, 1));
			chest4->start(area);
			chest4->set_position(util::Point<int>(38, 16));
			area->add_entity(chest4);

			wedge::Chest *chest5 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ARMOUR, ARMOUR_CHAIN_MAIL, 1));
			chest5->start(area);
			chest5->set_position(util::Point<int>(40, 16));
			area->add_entity(chest5);

			wedge::Chest *chest6 = new wedge::Chest("chest", "", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 1));
			chest6->start(area);
			chest6->set_position(util::Point<int>(6, 2));
			area->add_entity(chest6);

			wedge::Chest *chest7 = new wedge::Chest("chest", "", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 1));
			chest7->start(area);
			chest7->set_position(util::Point<int>(39, 14));
			area->add_entity(chest7);
		}

		return true;
	}

	void started()
	{
		if (INSTANCE->is_milestone_complete(MS_ENTERED_HAUNTED_HOUSE) == false) {
			INSTANCE->set_milestone_complete(MS_ENTERED_HAUNTED_HOUSE, true);
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1194)/* Originally: Let's take a look around. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
		}

		GLOBALS->max_battle_steps = Monster_RPG_3_Globals::MAX_BATTLE_STEPS * 1.25f;
		GLOBALS->min_battle_steps = Monster_RPG_3_Globals::MIN_BATTLE_STEPS * 1.25f;

		Area_Hooks_Monster_RPG_3::started();
	}

	void run()
	{
		darkness_offset1.x += 0.04f;
		darkness_offset1.y += 0.02f;
		darkness_offset2.x -= 0.04f;
		darkness_offset2.y -= 0.02f;
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			draw_darkness(map_offset);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete darkness_image1;
		darkness_image1 = NULL;
		delete darkness_image2;
		darkness_image2 = NULL;
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		darkness_image1 = gen_hh_darkness(1, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(2, 0.5f, 0.0f, shim::black);
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table.back();
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_3Sludge();
		}
		else if (type == 2) {
			return new Battle_2Ghastly();
		}
		else if (type == 1) {
			return new Battle_1Ghastly2Sludge();
		}
		else {
			return new Battle_2Werewolf();
		}
	}

	bool has_battles()
	{
		return true;
	}
};

class Area_Hooks_HH2 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_HH2(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(49, 2), util::Size<int>(1, 1));
		fz1.area_name = "hh1";
		fz1.player_positions.push_back(util::Point<int>(48, 1));
		fz1.player_positions.push_back(util::Point<int>(48, 1));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(24, 13), util::Size<int>(2, 1));
		fz2.area_name = "hh_secret";
		fz2.player_positions.push_back(util::Point<int>(4, 0));
		fz2.player_positions.push_back(util::Point<int>(5, 0));
		fz2.directions.push_back(wedge::DIR_S);
		fz2.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz2);

		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(5, 1), util::Size<int>(1, 1));
		fz3.area_name = "hh_save";
		fz3.player_positions.push_back(util::Point<int>(1, 2));
		fz3.player_positions.push_back(util::Point<int>(1, 2));
		fz3.directions.push_back(wedge::DIR_E);
		fz3.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz3);

		Fade_Zone fz4;
		fz4.zone = util::Rectangle<int>(util::Point<int>(3, 1), util::Size<int>(1, 1));
		fz4.area_name = "hh_stairs";
		fz4.player_positions.push_back(util::Point<int>(3, 2));
		fz4.player_positions.push_back(util::Point<int>(3, 2));
		fz4.directions.push_back(wedge::DIR_W);
		fz4.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz4);

		Fade_Zone fz5;
		fz5.zone = util::Rectangle<int>(util::Point<int>(1, 2), util::Size<int>(1, 1));
		fz5.area_name = "hh1";
		fz5.player_positions.push_back(util::Point<int>(1, 1));
		fz5.player_positions.push_back(util::Point<int>(1, 1));
		fz5.directions.push_back(wedge::DIR_S);
		fz5.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz5);

		Fade_Zone fz6;
		fz6.zone = util::Rectangle<int>(util::Point<int>(12, 30), util::Size<int>(2, 1));
		fz6.area_name = "hh";
		fz6.player_positions.push_back(util::Point<int>(56, 36));
		fz6.player_positions.push_back(util::Point<int>(57, 36));
		fz6.directions.push_back(wedge::DIR_S);
		fz6.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz6);

		Fade_Zone fz7;
		fz7.zone = util::Rectangle<int>(util::Point<int>(37, 30), util::Size<int>(2, 1));
		fz7.area_name = "hh";
		fz7.player_positions.push_back(util::Point<int>(78, 36));
		fz7.player_positions.push_back(util::Point<int>(79, 36));
		fz7.directions.push_back(wedge::DIR_S);
		fz7.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz7);
	}
	
	virtual ~Area_Hooks_HH2()
	{
		delete darkness_image1;
		delete darkness_image2;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/hh.mml");

		darkness_image1 = gen_hh_darkness(3, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(4, 0.5f, 0.0f, shim::black);
		darkness_offset1 = util::Point<float>(0, 0);
		darkness_offset2 = util::Point<float>(128, 64);

		if (loaded == false) {
			wedge::Chest *chest1 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 1));
			chest1->start(area);
			chest1->set_position(util::Point<int>(9, 5));
			area->add_entity(chest1);

			wedge::Chest *chest2 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(12, 5));
			area->add_entity(chest2);

			wedge::Chest *chest3 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 1));
			chest3->start(area);
			chest3->set_position(util::Point<int>(38, 3));
			area->add_entity(chest3);

			wedge::Chest *chest4 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 5));
			chest4->start(area);
			chest4->set_position(util::Point<int>(40, 3));
			area->add_entity(chest4);
		}

		return true;
	}

	void started()
	{
		GLOBALS->max_battle_steps = Monster_RPG_3_Globals::MAX_BATTLE_STEPS * 1.25f;
		GLOBALS->min_battle_steps = Monster_RPG_3_Globals::MIN_BATTLE_STEPS * 1.25f;

		Area_Hooks_Monster_RPG_3::started();
	}

	void run()
	{
		darkness_offset1.x += 0.04f;
		darkness_offset1.y += 0.02f;
		darkness_offset2.x -= 0.04f;
		darkness_offset2.y -= 0.02f;
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			draw_darkness(map_offset);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete darkness_image1;
		darkness_image1 = NULL;
		delete darkness_image2;
		darkness_image2 = NULL;
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		darkness_image1 = gen_hh_darkness(3, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(4, 0.5f, 0.0f, shim::black);
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(3);
		}

		int type = rand_battle_table.back();
		rand_battle_table.pop_back();
	
		if (type == 2) {
			return new Battle_2Knightly();
		}
		else if (type == 1) {
			return new Battle_2Ghastly();
		}
		else {
			return new Battle_2Werewolf();
		}
	}

	bool has_battles()
	{
		return true;
	}
};

class Area_Hooks_HH_Secret : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_HH_Secret(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(4, 0), util::Size<int>(2, 1));
		fz1.area_name = "hh2";
		fz1.player_positions.push_back(util::Point<int>(24, 13));
		fz1.player_positions.push_back(util::Point<int>(25, 13));
		fz1.directions.push_back(wedge::DIR_N);
		fz1.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_HH_Secret()
	{
		delete darkness_image1;
		delete darkness_image2;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/hh.mml");

		darkness_image1 = gen_hh_darkness(5, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(6, 0.5f, 0.0f, shim::black);
		darkness_offset1 = util::Point<float>(0, 0);
		darkness_offset2 = util::Point<float>(128, 64);

		if (loaded == false) {
			wedge::Chest *chest1 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_TOMAHAWK, 1));
			chest1->start(area);
			chest1->set_position(util::Point<int>(3, 4));
			area->add_entity(chest1);

			wedge::Chest *chest2 = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_TOMAHAWK, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(6, 4));
			area->add_entity(chest2);
		}

		return true;
	}

	void run()
	{
		darkness_offset1.x += 0.04f;
		darkness_offset1.y += 0.02f;
		darkness_offset2.x -= 0.04f;
		darkness_offset2.y -= 0.02f;
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			draw_darkness(map_offset);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete darkness_image1;
		darkness_image1 = NULL;
		delete darkness_image2;
		darkness_image2 = NULL;
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		darkness_image1 = gen_hh_darkness(5, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(6, 0.5f, 0.0f, shim::black);
	}
};

class Area_Hooks_HH_Save : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_HH_Save(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(1, 2), util::Size<int>(1, 1));
		fz1.area_name = "hh2";
		fz1.player_positions.push_back(util::Point<int>(5, 1));
		fz1.player_positions.push_back(util::Point<int>(5, 1));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
		
		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(9, 4), util::Size<int>(1, 3));
		fz2.area_name = "hh3";
		fz2.player_positions.push_back(util::Point<int>(16, 5));
		fz2.player_positions.push_back(util::Point<int>(15, 5));
		fz2.directions.push_back(wedge::DIR_E);
		fz2.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_HH_Save()
	{
		delete darkness_image1;
		delete darkness_image2;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/hh.mml");

		darkness_image1 = gen_hh_darkness(7, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(8, 0.5f, 0.0f, shim::black);
		darkness_offset1 = util::Point<float>(0, 0);
		darkness_offset2 = util::Point<float>(128, 64);

		if (loaded == false) {
			Revive_Entity *revive1 = new Revive_Entity();
			revive1->start(area);
			revive1->set_position(util::Point<int>(8, 3));
			area->add_entity(revive1);
			
			Revive_Entity *revive2 = new Revive_Entity();
			revive2->start(area);
			revive2->set_position(util::Point<int>(8, 7));
			area->add_entity(revive2);
		}

		return true;
	}

	void run()
	{
		darkness_offset1.x += 0.04f;
		darkness_offset1.y += 0.02f;
		darkness_offset2.x -= 0.04f;
		darkness_offset2.y -= 0.02f;
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			SDL_Colour colour;
			colour.r = shim::palette[18].r * 0.1f;
			colour.g = shim::palette[18].g * 0.1f;
			colour.b = shim::palette[18].b * 0.1f;
			colour.a = 255 * 0.1f;
			gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
			std::vector< util::Point<float> > beams;
			std::vector< util::Point<float> > torches;
			torches.push_back(util::Point<float>(8, 2.5f));
			torches.push_back(util::Point<float>(8, 6.5f));
			torch_size = shim::tile_size * 1.5f;
			flicker_size = 1.5f;
			draw_torches(shim::transparent, map_offset, torches, beams, true);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete darkness_image1;
		darkness_image1 = NULL;
		delete darkness_image2;
		darkness_image2 = NULL;
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		darkness_image1 = gen_hh_darkness(7, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(8, 0.5f, 0.0f, shim::black);
	}

	bool can_save()
	{
		return true;
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data savezone_anim;
		savezone_anim.topleft = util::Point<int>(9, 0);
		savezone_anim.size = util::Size<int>(2, 2);
		savezone_anim.delay = 128;
		savezone_anim.frames.push_back(util::Point<int>(9, 2));
		savezone_anim.frames.push_back(util::Point<int>(9, 4));
		savezone_anim.frames.push_back(util::Point<int>(9, 6));
		savezone_anim.frames.push_back(util::Point<int>(9, 8));
		savezone_anim.frames.push_back(util::Point<int>(9, 10));
		savezone_anim.frames.push_back(util::Point<int>(9, 12));
		savezone_anim.frames.push_back(util::Point<int>(9, 14));
		savezone_anim.frames.push_back(util::Point<int>(9, 16));
		savezone_anim.frames.push_back(util::Point<int>(9, 18));
		savezone_anim.frames.push_back(util::Point<int>(9, 20));
		savezone_anim.frames.push_back(util::Point<int>(9, 22));
		tilemap->add_animation_data(savezone_anim);

		gfx::Tilemap::Animation_Data blueflame_anim;
		blueflame_anim.topleft = util::Point<int>(0, 25);
		blueflame_anim.size = util::Size<int>(1, 2);
		blueflame_anim.delay = 64;
		blueflame_anim.frames.push_back(util::Point<int>(1, 25));
		blueflame_anim.frames.push_back(util::Point<int>(2, 25));
		blueflame_anim.frames.push_back(util::Point<int>(3, 25));
		blueflame_anim.frames.push_back(util::Point<int>(4, 25));
		tilemap->add_animation_data(blueflame_anim);
	}
};

class Area_Hooks_HH3 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_HH3(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		ignore_next_platform_move(false)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(15, 4), util::Size<int>(1, 3));
		fz1.area_name = "hh_save";
		fz1.player_positions.push_back(util::Point<int>(9, 4));
		fz1.player_positions.push_back(util::Point<int>(9, 5));
		fz1.directions.push_back(wedge::DIR_W);
		fz1.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(49, 1), util::Size<int>(1, 1));
		fz2.area_name = "hh";
		fz2.player_positions.push_back(util::Point<int>(86, 13));
		fz2.player_positions.push_back(util::Point<int>(86, 13));
		fz2.directions.push_back(wedge::DIR_W);
		fz2.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz2);

		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(24, 30), util::Size<int>(3, 1));
		fz3.area_name = "hh";
		fz3.player_positions.push_back(util::Point<int>(67, 31));
		fz3.player_positions.push_back(util::Point<int>(68, 31));
		fz3.directions.push_back(wedge::DIR_S);
		fz3.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz3);

		draw_arrow = 0;
	}
	
	virtual ~Area_Hooks_HH3()
	{
		delete darkness_image1;
		delete darkness_image2;
		delete big_down_arrow;
		delete platform_sound;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/hh.mml");

		darkness_image1 = gen_hh_darkness(9, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(10, 0.5f, 0.0f, shim::black);
		darkness_offset1 = util::Point<float>(0, 0);
		darkness_offset2 = util::Point<float>(128, 64);

		big_down_arrow = new gfx::Image("misc/big_down_arrow.tga");

		platform_sound = new audio::MML("sfx/platform.mml");

		util::Point<int> platform_start(19, 7);

		if (loaded == false) {
			platform = new wedge::Map_Entity("platform");
			platform->start(area);
			platform->set_position(platform_start);
			platform->set_sprite(new gfx::Sprite("platform"));
			platform->set_solid(false);
			platform->set_layer(0);
			area->add_entity(platform);
			
			Revive_Entity *revive1 = new Revive_Entity();
			revive1->start(area);
			revive1->set_position(util::Point<int>(40, 2));
			area->add_entity(revive1);
			
			Revive_Entity *revive2 = new Revive_Entity();
			revive2->start(area);
			revive2->set_position(util::Point<int>(43, 2));
			area->add_entity(revive2);
			
			Revive_Entity *revive3 = new Revive_Entity();
			revive3->start(area);
			revive3->set_position(util::Point<int>(46, 2));
			area->add_entity(revive3);
			
			Revive_Entity *revive4 = new Revive_Entity();
			revive4->start(area);
			revive4->set_position(util::Point<int>(40, 8));
			area->add_entity(revive4);
			
			Revive_Entity *revive5 = new Revive_Entity();
			revive5->start(area);
			revive5->set_position(util::Point<int>(43, 8));
			area->add_entity(revive5);
			
			Revive_Entity *revive6 = new Revive_Entity();
			revive6->start(area);
			revive6->set_position(util::Point<int>(46, 8));
			area->add_entity(revive6);
		}
		else {
			platform = area->find_entity("platform");

			// depending where you enter from, that's where the platform will be
			wedge::Map_Entity *eny = AREA->get_player(ENY);
			util::Point<int> eny_pos = eny->get_position();
			if (eny_pos.y < 12) {
				if (eny_pos.x < 25) {
					platform->set_position(platform_start);
				}
				else {
					platform->set_position(platform_start+util::Point<int>(10, 0));
				}
			}
		}
		
		arrow_yo = shim::screen_size.h * 0.05f;

		return true;
	}

	void run()
	{
		darkness_offset1.x += 0.04f;
		darkness_offset1.y += 0.02f;
		darkness_offset2.x -= 0.04f;
		darkness_offset2.y -= 0.02f;

		if (draw_arrow) {
			wedge::Map_Entity *eny = AREA->get_player(ENY);
			util::Point<int> pos = eny->get_position();
			if (pos.x >= 25) {
				draw_arrow = 0;
			}
		}
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			if (draw_arrow) {
				Uint32 now = GET_TICKS();
				Uint32 t = now % 1000;
				if (t < 500) {
					big_down_arrow->draw(util::Point<int>(shim::screen_size.w/2-big_down_arrow->size.w/2, shim::screen_size.h-arrow_yo-big_down_arrow->size.h));
				}
			}
			std::vector< util::Point<float> > torches;
			torches.push_back(util::Point<float>(40, 1.5f));
			torches.push_back(util::Point<float>(43, 1.5f));
			torches.push_back(util::Point<float>(46, 1.5f));
			torches.push_back(util::Point<float>(40, 7.5f));
			torches.push_back(util::Point<float>(43, 7.5f));
			torches.push_back(util::Point<float>(46, 7.5f));
			torch_size = shim::tile_size * 1.5f;
			flicker_size = 1.5f;
			std::vector< util::Point<float> > beams;
			beams.push_back(util::Point<float>(29, 10));
			beams.push_back(util::Point<float>(18, 21));
			draw_torches(shim::transparent, map_offset, torches, beams, true, 0, true);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete darkness_image1;
		darkness_image1 = NULL;
		delete darkness_image2;
		darkness_image2 = NULL;
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		darkness_image1 = gen_hh_darkness(9, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(10, 0.5f, 0.0f, shim::black);
		arrow_yo = shim::screen_size.h * 0.05f;
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data blueflame_anim;
		blueflame_anim.topleft = util::Point<int>(0, 25);
		blueflame_anim.size = util::Size<int>(1, 2);
		blueflame_anim.delay = 64;
		blueflame_anim.frames.push_back(util::Point<int>(1, 25));
		blueflame_anim.frames.push_back(util::Point<int>(2, 25));
		blueflame_anim.frames.push_back(util::Point<int>(3, 25));
		blueflame_anim.frames.push_back(util::Point<int>(4, 25));
		tilemap->add_animation_data(blueflame_anim);
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		wedge::Map_Entity *eny = AREA->get_player(ENY);
		wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);

		if (entity != eny) {
			return false;
		}

		util::Point<int> pos = entity->get_position();

		util::Point<int> platform_pos = platform->get_position();
		const float platform_speed = 1.0f/shim::logic_rate*shim::tile_size*2;

		if (pos == util::Point<int>(19, 5)) {
			if (ignore_next_platform_move) {
				ignore_next_platform_move = false;
				return false;
			}

			arrow_check = 0;

			util::Point<int> eny_start(20, 5);
			util::Point<int> tiggy_start(19, 5);

			util::Point<int> eny_mid(25, 5);
			util::Point<int> tiggy_mid(24, 5);

			util::Point<int> eny_end(30, 5);
			util::Point<int> tiggy_end(29, 5);
			
			wedge::pause_presses(true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(eny, eny_start, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(eny);
			entities.push_back(tiggy);
			std::vector< util::Point<int> > positions;
			positions.push_back(eny_start);
			positions.push_back(tiggy_start);
			wedge::Check_Positions_Step *cps = new wedge::Check_Positions_Step(entities, positions, true, new_task);
			ADD_STEP(cps);
			ADD_STEP(new wedge::Play_Sound_Step(platform_sound, false, true, new_task))
			wedge::Set_Integer_Step *sis = new wedge::Set_Integer_Step(&draw_arrow, 1, new_task);
			ADD_STEP(sis)
			ADD_STEP(new Custom_Slide_Entity_Step(eny, eny_mid, platform_speed, &arrow_check, new_task))

			std::vector< std::vector< std::vector< wedge::Step *> > > steps;
			steps.push_back(std::vector< std::vector< wedge::Step * > >());
			steps.push_back(std::vector< std::vector< wedge::Step * > >());

			std::vector<wedge::Step *> v1, v2, v3;
			v1.push_back(new wedge::Slide_Entity_Step(eny, eny_end, platform_speed, new_task));
			v1.push_back(new wedge::Stop_Sound_Step(platform_sound, new_task));
			v1.push_back(new wedge::A_Star_Step(eny, eny_end+util::Point<int>(3, 0), new_task));
			v1.push_back(new wedge::Pause_Presses_Step(false, true, new_task));
			v2.push_back(new wedge::Slide_Entity_Step(tiggy, tiggy_end, platform_speed, new_task));
			v3.push_back(new wedge::Slide_Entity_Step(platform, platform_pos+util::Point<int>(10, 0), platform_speed, new_task));

			steps[0].push_back(v1);
			steps[0].push_back(v2);
			steps[0].push_back(v3);

			v1.clear();
			v2.clear();
			v3.clear();

			v1.push_back(new wedge::Set_Direction_Step(eny, wedge::DIR_S, true, false, new_task));
			v1.push_back(new wedge::Slide_Entity_Step(eny, eny_mid+util::Point<int>(0, 17), platform_speed, new_task));
			//v1.push_back(new wedge::Slide_Entity_Step(eny, eny_mid+util::Point<int>(0, 17), platform_speed, new_task));
			v1.push_back(new wedge::Stop_Sound_Step(platform_sound, new_task));
			v1.push_back(new wedge::A_Star_Step(eny, eny_mid+util::Point<int>(0, 20), new_task));
			v1.push_back(new wedge::Pause_Presses_Step(false, true, new_task));
			v2.push_back(new wedge::Set_Direction_Step(tiggy, wedge::DIR_S, true, false, new_task));
			v2.push_back(new wedge::Slide_Entity_Step(tiggy, tiggy_mid+util::Point<int>(0, 17), platform_speed, new_task));
			v3.push_back(new wedge::Slide_Entity_Step(platform, platform_pos+util::Point<int>(5, 17), platform_speed, new_task));

			steps[1].push_back(v1);
			steps[1].push_back(v2);
			steps[1].push_back(v3);

			ADD_STEP(new wedge::Branch_Step(&arrow_check, AREA, steps, new_task));

			ADD_TASK(new_task)

			ANOTHER_TASK
			wedge::Wait_Step *tig_wait = new wedge::Wait_Step(new_task);
			sis->add_monitor(tig_wait);
			ADD_STEP(tig_wait);
			ADD_STEP(new Custom_Slide_Entity_Step(tiggy, tiggy_mid, platform_speed, &arrow_check, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			wedge::Wait_Step *platform_wait = new wedge::Wait_Step(new_task);
			sis->add_monitor(platform_wait);
			ADD_STEP(platform_wait);
			ADD_STEP(new Custom_Slide_Entity_Step(platform, platform_pos + util::Point<int>(5, 0), platform_speed, &arrow_check, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			ignore_next_platform_move = true;

			return true;
		}
		else if (pos == util::Point<int>(25, 23)) {
			if (ignore_next_platform_move) {
				ignore_next_platform_move = false;
				return false;
			}

			util::Point<int> eny_start(25, 22);
			util::Point<int> tiggy_start(25, 23);

			util::Point<int> eny_mid(25, 5);
			util::Point<int> tiggy_mid(25, 6);

			util::Point<int> eny_end(30, 5);
			util::Point<int> tiggy_end(30, 6);
			
			wedge::pause_presses(true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(eny, eny_start, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(eny);
			entities.push_back(tiggy);
			std::vector< util::Point<int> > positions;
			positions.push_back(eny_start);
			positions.push_back(tiggy_start);
			wedge::Check_Positions_Step *cps = new wedge::Check_Positions_Step(entities, positions, true, new_task);
			ADD_STEP(cps);
			ADD_STEP(new wedge::Play_Sound_Step(platform_sound, false, true, new_task))
			ADD_STEP(new wedge::Slide_Entity_Step(eny, eny_mid, platform_speed, new_task))
			ADD_STEP(new wedge::Set_Direction_Step(eny, wedge::DIR_E, true, false, new_task))
			ADD_STEP(new wedge::Slide_Entity_Step(eny, eny_end, platform_speed, new_task))
			ADD_STEP(new wedge::Stop_Sound_Step(platform_sound, new_task))
			ADD_STEP(new wedge::A_Star_Step(eny, eny_end+util::Point<int>(3, 0), new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, true, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			wedge::Wait_Step *tig_wait = new wedge::Wait_Step(new_task);
			cps->add_monitor(tig_wait);
			ADD_STEP(tig_wait);
			ADD_STEP(new wedge::Slide_Entity_Step(tiggy, tiggy_mid, platform_speed, new_task))
			ADD_STEP(new wedge::Set_Direction_Step(tiggy, wedge::DIR_E, true, false, new_task))
			ADD_STEP(new wedge::Slide_Entity_Step(tiggy, tiggy_end, platform_speed, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			wedge::Wait_Step *platform_wait = new wedge::Wait_Step(new_task);
			cps->add_monitor(platform_wait);
			ADD_STEP(platform_wait);
			ADD_STEP(new wedge::Slide_Entity_Step(platform, platform_pos + util::Point<int>(0, -17), platform_speed, new_task))
			ADD_STEP(new wedge::Slide_Entity_Step(platform, platform_pos + util::Point<int>(5, -17), platform_speed, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			ignore_next_platform_move = true;

			return true;
		}
		else if (pos == util::Point<int>(31, 5)) {
			if (ignore_next_platform_move) {
				ignore_next_platform_move = false;
				return false;
			}

			util::Point<int> eny_start(30, 5);
			util::Point<int> tiggy_start(31, 5);

			util::Point<int> eny_end(20, 5);
			util::Point<int> tiggy_end(21, 5);
			
			wedge::pause_presses(true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(eny, eny_start, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(eny);
			entities.push_back(tiggy);
			std::vector< util::Point<int> > positions;
			positions.push_back(eny_start);
			positions.push_back(tiggy_start);
			wedge::Check_Positions_Step *cps = new wedge::Check_Positions_Step(entities, positions, true, new_task);
			ADD_STEP(cps);
			ADD_STEP(new wedge::Play_Sound_Step(platform_sound, false, true, new_task))
			ADD_STEP(new wedge::Slide_Entity_Step(eny, eny_end, platform_speed, new_task))
			ADD_STEP(new wedge::Stop_Sound_Step(platform_sound, new_task))
			ADD_STEP(new wedge::A_Star_Step(eny, eny_end-util::Point<int>(3, 0), new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, true, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			wedge::Wait_Step *tig_wait = new wedge::Wait_Step(new_task);
			cps->add_monitor(tig_wait);
			ADD_STEP(tig_wait);
			ADD_STEP(new wedge::Slide_Entity_Step(tiggy, tiggy_end, platform_speed, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			wedge::Wait_Step *platform_wait = new wedge::Wait_Step(new_task);
			cps->add_monitor(platform_wait);
			ADD_STEP(platform_wait);
			ADD_STEP(new wedge::Slide_Entity_Step(platform, platform_pos + util::Point<int>(-10, 0), platform_speed, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			ignore_next_platform_move = true;

			return true;
		}

		return false;
	}

	void handle_event(TGUI_Event *event)
	{
		if (event->type == TGUI_FOCUS && event->focus.type == TGUI_FOCUS_DOWN) {
			arrow_check = 1;
		}
		else if (event->type == TGUI_MOUSE_DOWN && cd::box_box(util::Point<int>(shim::screen_size.w/2-big_down_arrow->size.w/2, shim::screen_size.h-big_down_arrow->size.h-arrow_yo), big_down_arrow->size, util::Point<int>(event->mouse.x, event->mouse.y), util::Size<int>(1, 1))) {
			arrow_check = 1;
		}
	}

private:
	static int arrow_check;
	static int draw_arrow;

	wedge::Map_Entity *platform;

	gfx::Image *big_down_arrow;

	audio::MML *platform_sound;

	bool ignore_next_platform_move;

	int arrow_yo;
};

int Area_Hooks_HH3::arrow_check = 0;
int Area_Hooks_HH3::draw_arrow = 0;

class Area_Hooks_HH_Stairs : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_HH_Stairs(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(1, 1), util::Size<int>(1, 1));
		fz1.area_name = "hh";
		fz1.player_positions.push_back(util::Point<int>(49, 13));
		fz1.player_positions.push_back(util::Point<int>(49, 13));
		fz1.directions.push_back(wedge::DIR_E);
		fz1.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(3, 2), util::Size<int>(1, 1));
		fz2.area_name = "hh2";
		fz2.player_positions.push_back(util::Point<int>(3, 1));
		fz2.player_positions.push_back(util::Point<int>(3, 1));
		fz2.directions.push_back(wedge::DIR_S);
		fz2.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_HH_Stairs()
	{
		delete darkness_image1;
		delete darkness_image2;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/hh.mml");

		darkness_image1 = gen_hh_darkness(11, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(12, 0.5f, 0.0f, shim::black);
		darkness_offset1 = util::Point<float>(0, 0);
		darkness_offset2 = util::Point<float>(128, 64);

		return true;
	}

	void run()
	{
		darkness_offset1.x += 0.04f;
		darkness_offset1.y += 0.02f;
		darkness_offset2.x -= 0.04f;
		darkness_offset2.y -= 0.02f;
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			draw_darkness(map_offset);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete darkness_image1;
		darkness_image1 = NULL;
		delete darkness_image2;
		darkness_image2 = NULL;
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		darkness_image1 = gen_hh_darkness(11, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(12, 0.5f, 0.0f, shim::black);
	}
};

static int palla_battle_done = 0;

class Area_Hooks_HH_Basement : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_HH_Basement(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(19, 1), util::Size<int>(2, 1));
		fz1.area_name = "hh";
		fz1.player_positions.push_back(util::Point<int>(67, 9));
		fz1.player_positions.push_back(util::Point<int>(68, 9));
		fz1.directions.push_back(wedge::DIR_N);
		fz1.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(3, 1), util::Size<int>(1, 1));
		fz2.area_name = "hh1";
		fz2.player_positions.push_back(util::Point<int>(3, 2));
		fz2.player_positions.push_back(util::Point<int>(3, 2));
		fz2.directions.push_back(wedge::DIR_W);
		fz2.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_HH_Basement()
	{
		delete darkness_image1;
		delete darkness_image2;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/hh.mml");

		darkness_image1 = gen_hh_darkness(13, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(14, 0.5f, 0.0f, shim::black);
		darkness_offset1 = util::Point<float>(0, 0);
		darkness_offset2 = util::Point<float>(128, 64);

		if (loaded == false) {
			palla = new wedge::NPC("palla", "Palla", "palla", "palla");
			palla->start(area);
			palla->set_position(util::Point<int>(19, 8));
			palla->set_size(util::Size<int>(2, 1));
			area->add_entity(palla);
			
			Revive_Entity *revive1 = new Revive_Entity();
			revive1->start(area);
			revive1->set_position(util::Point<int>(1, 2));
			area->add_entity(revive1);
			
			Revive_Entity *revive2 = new Revive_Entity();
			revive2->start(area);
			revive2->set_position(util::Point<int>(18, 2));
			area->add_entity(revive2);
			
			Revive_Entity *revive3 = new Revive_Entity();
			revive3->start(area);
			revive3->set_position(util::Point<int>(21, 2));
			area->add_entity(revive3);
			
			Revive_Entity *revive4 = new Revive_Entity();
			revive4->start(area);
			revive4->set_position(util::Point<int>(17, 17));
			area->add_entity(revive4);
			
			Revive_Entity *revive5 = new Revive_Entity();
			revive5->start(area);
			revive5->set_position(util::Point<int>(17, 19));
			area->add_entity(revive5);
			
			Revive_Entity *revive6 = new Revive_Entity();
			revive6->start(area);
			revive6->set_position(util::Point<int>(17, 21));
			area->add_entity(revive6);
			
			Revive_Entity *revive7 = new Revive_Entity();
			revive7->start(area);
			revive7->set_position(util::Point<int>(22, 17));
			area->add_entity(revive7);
			
			Revive_Entity *revive8 = new Revive_Entity();
			revive8->start(area);
			revive8->set_position(util::Point<int>(22, 19));
			area->add_entity(revive8);
			
			Revive_Entity *revive9 = new Revive_Entity();
			revive9->start(area);
			revive9->set_position(util::Point<int>(22, 21));
			area->add_entity(revive9);
		}
		else {
			palla = area->find_entity("palla");
			if (palla != NULL) {
				if (INSTANCE->is_milestone_complete(MS_BEAT_PALLA) == true) {
					palla->get_sprite()->set_animation("lay");
				}
			}
		}

		return true;
	}

	void run()
	{
		darkness_offset1.x += 0.04f;
		darkness_offset1.y += 0.02f;
		darkness_offset2.x -= 0.04f;
		darkness_offset2.y -= 0.02f;
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			SDL_Colour colour;
			colour.r = shim::palette[18].r * 0.1f;
			colour.g = shim::palette[18].g * 0.1f;
			colour.b = shim::palette[18].b * 0.1f;
			colour.a = 255 * 0.1f;
			gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
			std::vector< util::Point<float> > beams;
			std::vector< util::Point<float> > torches;
			torches.push_back(util::Point<float>(17, 16.5f));
			torches.push_back(util::Point<float>(17, 18.5f));
			torches.push_back(util::Point<float>(17, 20.5f));
			torches.push_back(util::Point<float>(22, 16.5f));
			torches.push_back(util::Point<float>(22, 18.5f));
			torches.push_back(util::Point<float>(22, 20.5f));
			torches.push_back(util::Point<float>(18, 1.5f));
			torches.push_back(util::Point<float>(21, 1.5f));
			torches.push_back(util::Point<float>(1, 1.5f));
			torch_size = shim::tile_size * 1.5f;
			flicker_size = 1.5f;
			draw_torches(shim::transparent, map_offset, torches, beams, true);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete darkness_image1;
		darkness_image1 = NULL;
		delete darkness_image2;
		darkness_image2 = NULL;
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		darkness_image1 = gen_hh_darkness(13, 0.5f, 0.25f, shim::black);
		darkness_image2 = gen_hh_darkness(14, 0.5f, 0.0f, shim::black);
	}
	
	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data blueflame_anim;
		blueflame_anim.topleft = util::Point<int>(0, 25);
		blueflame_anim.size = util::Size<int>(1, 2);
		blueflame_anim.delay = 64;
		blueflame_anim.frames.push_back(util::Point<int>(1, 25));
		blueflame_anim.frames.push_back(util::Point<int>(2, 25));
		blueflame_anim.frames.push_back(util::Point<int>(3, 25));
		blueflame_anim.frames.push_back(util::Point<int>(4, 25));
		tilemap->add_animation_data(blueflame_anim);
	}
	
	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		if (INSTANCE->is_milestone_complete(MS_BEAT_PALLA) == true) {
			return false;
		}

		wedge::Map_Entity *eny = AREA->get_player(ENY);
		wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);

		util::Point<int> pos = entity->get_position();

		util::Point<int> eny_pos(19, 10);
		util::Point<int> tiggy_pos(20, 10);
			
		if (entity == eny && pos.x >= 18 && pos.x <= 21 && pos.y == 13) {
			INSTANCE->set_milestone_complete(MS_PALLA_STEP, true);

			palla_battle_done = 0;

			INSTANCE->party_following_player = false;
			
			wedge::pause_presses(true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(eny, eny_pos, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(eny);
			entities.push_back(tiggy);
			std::vector< util::Point<int> > positions;
			positions.push_back(eny_pos);
			positions.push_back(tiggy_pos);
			ADD_STEP(new wedge::Check_Positions_Step(entities, positions, true, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1196)/* Originally: Hey! You! Who are you? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1198)/* Originally: Me? I live here. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1200)/* Originally: Then you're the one spawning these monsters! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1202)/* Originally: Who sent you here? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1204)/* Originally: The mayor sent us. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1205)/* Originally: We're here to put an end to the monster outbreak! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			gfx::Sprite *sprite = palla->get_sprite();
			ADD_STEP(new wedge::Play_Animation_Step(sprite, "point_sword", new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1207)/* Originally: Oh, you're friends of the mayor... */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1208)/* Originally: Well, any friend of the mayor is an enemy of mine! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1209)/* Originally: Prepare to meet your doom! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Start_Battle_Step(new Battle_Palla(), new_task))
			ADD_STEP(new wedge::Wait_For_Integer_Step(&palla_battle_done, 1, new_task))
			ADD_STEP(new wedge::Play_Animation_Step(sprite, "lay", new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1211)/* Originally: Ugh... why must you torment my soul further? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1213)/* Originally: Torment YOU?! You're putting everyone in danger by spawning monsters! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1215)/* Originally: I'm afraid you're mistaken... */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1216)/* Originally: A deeper sorrow than my own is spawning these monsters. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1217)/* Originally: Here, take this. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Give_Object_Step(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_VAMPIRE1, 1), wedge::DIALOGUE_AUTO, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1219)/* Originally: What's this? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1221)/* Originally: The mayor... is my brother. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1222)/* Originally: When we were children, we found these Vampire Pendants. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1223)/* Originally: The one using them gains great power... */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1224)/* Originally: At the expense of the other... */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1225)/* Originally: My brother gained power at my expense... */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1226)/* Originally: I refused to use my pendant. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1228)/* Originally: That little snake! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1229)/* Originally: And he sent us here to kill you! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1231)/* Originally: We're really sorry Mister! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1232)/* Originally: Eny, let's pay the mayor a visit! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1234)/* Originally: Definitely! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1236)/* Originally: With all my suffering, I've developed very... keen senses... I sense a great sorrow... in the Eastern Kingdom. */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1237)/* Originally: That's... where the monsters are coming from! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1238)/* Originally: But now... */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1239)/* Originally: I'm afraid I can't... I can't get up... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1241)/* Originally: Don't worry sir, we'll get the other Pendant from the mayor! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(91)/* Originally: Palla */ + TAG_END, GLOBALS->game_t->translate(1243)/* Originally: Thank you, but do hurry. I feel... very... weak... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_STEP(new Autosave_Step(new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			return true;
		}

		if (entity == tiggy && INSTANCE->is_milestone_complete(MS_PALLA_STEP) && INSTANCE->is_milestone_complete(MS_PALLA_STEP2) == false) {
			INSTANCE->set_milestone_complete(MS_PALLA_STEP2, true);
			INSTANCE->set_milestone_complete(MS_BEAT_PALLA, true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(tiggy, tiggy_pos, new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(set_following, (void *)1, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			return true;
		}

		return false;
	}

	bool try_tile(wedge::Map_Entity *entity, util::Point<int> tile_pos)
	{
		if (INSTANCE->is_milestone_complete(MS_BEAT_PALLA) == true) {
			return false;
		}

		if (GLOBALS->retried_boss) {
			return false;
		}

		wedge::Map_Entity *eny = AREA->get_player(ENY);

		if (entity != eny) {
			return false;
		}

		if (tile_pos.x >= 18 && tile_pos.x <= 21 && tile_pos.y == 13) {
			GLOBALS->boss_save = wedge::save();
			GLOBALS->boss_press = wedge::DIR_N;
		}

		return false;
	}

private:
	wedge::Map_Entity *palla;
};

static void beach_w_callback(void *data);

class Area_Hooks_Beach_W : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Beach_W(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(10, 0), util::Size<int>(11, 1));
		fz1.area_name = "riverside";
		fz1.player_positions.push_back(util::Point<int>(15, 51));
		fz1.player_positions.push_back(util::Point<int>(16, 51));
		fz1.directions.push_back(wedge::DIR_N);
		fz1.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(12, 44), util::Size<int>(1, 1));
		fz2.area_name = "below_deck";
		fz2.player_positions.push_back(util::Point<int>(1, 5));
		fz2.player_positions.push_back(util::Point<int>(1, 5));
		fz2.directions.push_back(wedge::DIR_E);
		fz2.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_Beach_W()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/boatin.mml");

		if (loaded == false) {
			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(27, 18));
			area->add_entity(chest);
		}

		if (INSTANCE->is_milestone_complete(MS_PALLA_LEFT) == true) {
			if (area->find_entity("sailship_low") == NULL) {
				Sail_Ship *sailship_low = new Sail_Ship("sailship_low");
				sailship_low->start(area);
				sailship_low->set_position(util::Point<int>(3, 48));
				sailship_low->set_sprite(new gfx::Sprite("sailship_low"));
				sailship_low->set_layer(1);
				area->add_entity(sailship_low);
				
				Sail_Ship *sailship_high = new Sail_Ship("sailship_high");
				sailship_high->start(area);
				sailship_high->set_position(util::Point<int>(3, 48));
				sailship_high->set_sprite(new gfx::Sprite("sailship_high"));
				sailship_high->set_layer(3);
				area->add_entity(sailship_high);
			
				captain = new Sailor("captain");
				captain->start(area);
				captain->set_position(util::Point<int>(6, 43));
				captain->set_sprite(new gfx::Sprite("captain"));
				captain->set_direction(wedge::DIR_E, true, false);
				area->add_entity(captain);

				std::vector<wedge::Direction> dont_face1, dont_face2, dont_face3, dont_face4, dont_face5, dont_face6;
				dont_face3.push_back(wedge::DIR_E);
				dont_face4.push_back(wedge::DIR_E);
				dont_face5.push_back(wedge::DIR_E);
				dont_face5.push_back(wedge::DIR_N);
			
				Sailor *sailor1 = new Sailor("sailor1");
				sailor1->set_looks_around(dont_face1);
				sailor1->start(area);
				sailor1->set_position(util::Point<int>(25, 42));
				sailor1->set_sprite(new gfx::Sprite("sailor"));
				sailor1->set_direction(wedge::DIR_E, true, false);
				area->add_entity(sailor1);
				
				Sailor *sailor2 = new Sailor("sailor2");
				sailor2->set_looks_around(dont_face2);
				sailor2->start(area);
				sailor2->set_position(util::Point<int>(25, 45));
				sailor2->set_sprite(new gfx::Sprite("sailor"));
				sailor2->set_direction(wedge::DIR_E, true, false);
				sailor1->set_looks_around(dont_face1);
				area->add_entity(sailor2);

				Sailor_NPC *sailor3 = new Sailor_NPC("sailor3", "Sailor", "sailor", "sailor3");
				sailor3->set_looks_around(dont_face3);
				sailor3->start(area);
				sailor3->set_position(util::Point<int>(18, 43));
				sailor3->set_direction(wedge::DIR_W, true, false);
				area->add_entity(sailor3);
				
				Sailor_NPC *sailor4 = new Sailor_NPC("sailor4", "Sailor", "sailor", "sailor4");
				sailor4->set_looks_around(dont_face4);
				sailor4->start(area);
				sailor4->set_position(util::Point<int>(18, 44));
				sailor4->set_direction(wedge::DIR_W, true, false);
				area->add_entity(sailor4);

				Sailor *sailor5 = new Sailor("sailor5");
				sailor5->set_looks_around(dont_face5);
				sailor5->start(area);
				sailor5->set_position(util::Point<int>(7, 42));
				sailor5->set_sprite(new gfx::Sprite("sailor"));
				sailor5->set_direction(wedge::DIR_S, true, false);
				area->add_entity(sailor5);
				
				Sailor_NPC *sailor6 = new Sailor_NPC("sailor6", "Sailor", "sailor", "sailor6");
				sailor6->set_looks_around(dont_face6);
				sailor6->start(area);
				sailor6->set_position(util::Point<int>(6, 45));
				sailor6->set_direction(wedge::DIR_N, true, false);
				area->add_entity(sailor6);
			}
			else {
				captain = area->find_entity("captain");
			}
		}
		else {
			gfx::Tilemap *tilemap = area->get_tilemap();
			util::Size<int> sz = tilemap->get_size();
			// hide the ship solids...
			tilemap->set_solid(0, util::Point<int>(0, 41), util::Size<int>(sz.w, sz.h-41), false);
			for (int y = 41; y < sz.h; y++) {
				for (int x = 0; x < sz.w; x++) {
				}
			}
			// make plank solid
			tilemap->set_solid(-1, util::Point<int>(13, 41), util::Size<int>(1, 1), true);

			captain = NULL;
		}

		if (captain) {
			captain->set_direction(wedge::DIR_E, true, false);
		}

		return true;
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data water_anim;
		water_anim.topleft = util::Point<int>(3, 27);
		water_anim.size = util::Size<int>(4, 1);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(3, 28));
		water_anim.frames.push_back(util::Point<int>(3, 29));
		tilemap->add_animation_data(water_anim);
		water_anim.topleft = util::Point<int>(10, 27);
		water_anim.size = util::Size<int>(3, 1);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(10, 28));
		water_anim.frames.push_back(util::Point<int>(10, 29));
		tilemap->add_animation_data(water_anim);
	}
	
	void run()
	{
		wedge::Map_Entity *sailship = area->find_entity("sailship_low");
		if (sailship == NULL) {
			return;
		}
		gfx::Sprite *sprite = sailship->get_sprite();
		gfx::Image *img = sprite->get_current_image();
		util::Point<float> sailship_centre = sailship->get_position() * shim::tile_size - util::Point<int>(0, (img->size.h - shim::tile_size)) + util::Point<float>(img->size.w/2.0f, img->size.h*4.0f/5.0f);

		wedge::Map_Entity_List &entities = area->get_entities();

		for (wedge::Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
			wedge::Map_Entity *e = *it;
			Sailor *s = dynamic_cast<Sailor *>(e);
			if (s) {
				util::Point<int> position = s->get_position();
				if (position.y >= 41) {
					s->set_use_pivot(true);
					s->set_pivot(sailship_centre);
				}
				else {
					s->set_use_pivot(false);
				}
			}
			else {
				Sailor_NPC *s = dynamic_cast<Sailor_NPC *>(e);
				if (s) {
					util::Point<int> position = s->get_position();
					if (position.y >= 41) {
						s->set_use_pivot(true);
						s->set_pivot(sailship_centre);
					}
					else {
						s->set_use_pivot(false);
					}
				}
			}
		}
	}

	void end()
	{
		std::vector<wedge::Map_Entity *> players = AREA->get_players();
		for (size_t i = 0; i < players.size(); i++) {
			Sailor *s = dynamic_cast<Sailor *>(players[i]);
			if (s) {
				s->set_use_pivot(false);
			}
		}
	}
	
	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activator == AREA->get_player(ENY) && activated == captain) {
			captain->set_direction(wedge::DIR_S, true, false);

			if (INSTANCE->is_milestone_complete(MS_TALKED_ABOUT_BOATIN) == false) {
				INSTANCE->set_milestone_complete(MS_TALKED_ABOUT_BOATIN, true);

				NEW_SYSTEM_AND_TASK(AREA)
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1245)/* Originally: Captain! We need to talk! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1247)/* Originally: What'll it be young lady? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1249)/* Originally: The mayor said we can use your ship. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1251)/* Originally: What d'ya need my ship for? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1253)/* Originally: We need to get to the Eastern Kingdom.^We think the monsters are coming from there. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1255)/* Originally: Aye, but the seas are fierce!^You best be equipped for battle! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1257)/* Originally: We can handle it... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new wedge::Generic_Immediate_Callback_Step(beach_w_callback, this, new_task))
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)
			}
			else {
				ask_about_boatin();
			}
			return true;
		}
		return false;
	}

	void ask_about_boatin()
	{
		NEW_SYSTEM_AND_TASK(AREA)
		Captain_Step *cs = new Captain_Step(new_task);
		ADD_STEP(cs)
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)

		std::vector<std::string> choices;
		choices.push_back(GLOBALS->game_t->translate(1258)/* Originally: Head East! */);
		choices.push_back(GLOBALS->game_t->translate(1259)/* Originally: Maybe later... */);
		do_question(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1261)/* Originally: At your command, miss! */, wedge::DIALOGUE_SPEECH, choices, cs);
	}

	bool is_corner_portal(util::Point<int> pos)
	{
		return pos == util::Point<int>(12, 44);
	}

private:
	wedge::Map_Entity *captain;
};

static void beach_w_callback(void *data)
{
	Area_Hooks_Beach_W *hooks = static_cast<Area_Hooks_Beach_W *>(data);
	hooks->ask_about_boatin();
}

class Area_Hooks_Below_Deck : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Below_Deck(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		on_ship = true;

		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(1, 5), util::Size<int>(1, 1));
		if (M3_INSTANCE->boatin) {
			fz1.area_name = "at_sea";
			fz1.player_positions.push_back(util::Point<int>(12, 14));
			fz1.player_positions.push_back(util::Point<int>(12, 14));
		}
		else {
			if (M3_INSTANCE->boat_w) {
				fz1.area_name = "beach_w";
				fz1.player_positions.push_back(util::Point<int>(12, 44));
				fz1.player_positions.push_back(util::Point<int>(12, 44));
			}
			else {
				fz1.area_name = "beach_e";
				fz1.player_positions.push_back(util::Point<int>(12, 24));
				fz1.player_positions.push_back(util::Point<int>(12, 24));
			}
		}
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(7, 1), util::Size<int>(1, 1));
		fz2.area_name = "captains_quarters";
		fz2.player_positions.push_back(util::Point<int>(2, 6));
		fz2.player_positions.push_back(util::Point<int>(2, 6));
		fz2.directions.push_back(wedge::DIR_N);
		fz2.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_Below_Deck()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/boatin.mml");

		if (loaded == false) {
			item_merchant = new Sailor("item_merchant");
			item_merchant->start(area);
			item_merchant->set_position(util::Point<int>(10, 3));
			item_merchant->set_sprite(new gfx::Sprite("sailor"));
			item_merchant->set_direction(wedge::DIR_W, true, false);
			area->add_entity(item_merchant);
			
			weapons_merchant = new Sailor("weapons_merchant");
			weapons_merchant->start(area);
			weapons_merchant->set_position(util::Point<int>(10, 4));
			weapons_merchant->set_sprite(new gfx::Sprite("sailor"));
			weapons_merchant->set_direction(wedge::DIR_W, true, false);
			area->add_entity(weapons_merchant);
		}
		else {
			item_merchant = area->find_entity("item_merchant");
			weapons_merchant = area->find_entity("weapons_merchant");
		}

		return true;
	}

	void started()
	{
		if (static_cast<Monster_RPG_3_Area_Game *>(AREA)->get_num_areas_created() > 1) {
			autosave(true);
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> pos = activator->get_position();
		wedge::Direction dir = activator->get_direction();
		if (activator == AREA->get_player(ENY) && (
			((pos == util::Point<int>(1, 4) || pos == util::Point<int>(3, 4)) && dir == wedge::DIR_N) ||
			((pos == util::Point<int>(2, 2) || pos == util::Point<int>(2, 3) || pos == util::Point<int>(4, 2) || pos == util::Point<int>(4, 3)) && dir == wedge::DIR_W) ||
			((pos == util::Point<int>(2, 2) || pos == util::Point<int>(2, 3)) && dir == wedge::DIR_E)
			)
		) {
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new Inn_Step(wedge::DIALOGUE_MESSAGE, "", GLOBALS->game_t->translate(1262)/* Originally: Sleep here? */, "", "", "", 0, util::Point<int>(2, 2), new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}

		return false;
	}

	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activator == AREA->get_player(ENY) && activated == weapons_merchant) {
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(92)/* Originally: Sailor */ + TAG_END, GLOBALS->game_t->translate(1264)/* Originally: Looking to upgrade? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_TOMAHAWK, 150));
			items.push_back(OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_SPEAR, 800));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_WEAPON, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}
		else if (activator == AREA->get_player(ENY) && activated == item_merchant) {
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(92)/* Originally: Sailor */ + TAG_END, GLOBALS->game_t->translate(1266)/* Originally: Need supplies? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_BAIT, 5));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION, 20));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_CURE, 30));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION_PLUS, 50));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 100));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 100));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_ITEM, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}
		return false;
	}

private:
	wedge::Map_Entity *item_merchant;
	wedge::Map_Entity *weapons_merchant;
};

class Area_Hooks_Captains_Quarters : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Captains_Quarters(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		on_ship = true;

		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(2, 6), util::Size<int>(1, 1));
		fz1.area_name = "below_deck";
		fz1.player_positions.push_back(util::Point<int>(7, 1));
		fz1.player_positions.push_back(util::Point<int>(7, 1));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Captains_Quarters()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/boatin.mml");

		if (loaded == false) {
			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_APPLE, 3));
			chest->start(area);
			chest->set_position(util::Point<int>(3, 2));
			area->add_entity(chest);
		}

		return true;
	}
};

static void fishing_callback(void *data);

class Area_Hooks_At_Sea : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_At_Sea(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		on_ship = true;
		casting = false;

		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(12, 14), util::Size<int>(1, 1));
		fz1.area_name = "below_deck";
		fz1.player_positions.push_back(util::Point<int>(1, 5));
		fz1.player_positions.push_back(util::Point<int>(1, 5));
		fz1.directions.push_back(wedge::DIR_E);
		fz1.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_At_Sea()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/boatin.mml");

		if (loaded == false) {
			Sail_Ship *sailship_low = new Sail_Ship("sailship_low");
			sailship_low->start(area);
			sailship_low->set_position(util::Point<int>(3, 18));
			sailship_low->set_sprite(new gfx::Sprite("sailship_low"));
			sailship_low->set_layer(1);
			area->add_entity(sailship_low);
			
			Sail_Ship *sailship_high = new Sail_Ship("sailship_high");
			sailship_high->start(area);
			sailship_high->set_position(util::Point<int>(3, 18));
			sailship_high->set_sprite(new gfx::Sprite("sailship_high"));
			sailship_high->set_layer(3);
			area->add_entity(sailship_high);
			
			captain = new Sailor("captain");
			captain->start(area);
			captain->set_position(util::Point<int>(6, 13));
			captain->set_sprite(new gfx::Sprite("captain"));
			captain->set_direction(wedge::DIR_E, true, false);
			area->add_entity(captain);

			std::vector<wedge::Direction> dont_face1, dont_face2, dont_face3, dont_face4, dont_face5, dont_face6;
			dont_face3.push_back(wedge::DIR_E);
			dont_face4.push_back(wedge::DIR_E);
			dont_face5.push_back(wedge::DIR_E);
			dont_face5.push_back(wedge::DIR_N);
		
			Sailor *sailor1 = new Sailor("sailor1");
			sailor1->set_looks_around(dont_face1);
			sailor1->start(area);
			sailor1->set_position(util::Point<int>(25, 12));
			sailor1->set_sprite(new gfx::Sprite("sailor"));
			sailor1->set_direction(wedge::DIR_E, true, false);
			area->add_entity(sailor1);
			
			Sailor *sailor2 = new Sailor("sailor2");
			sailor2->set_looks_around(dont_face2);
			sailor2->start(area);
			sailor2->set_position(util::Point<int>(25, 15));
			sailor2->set_sprite(new gfx::Sprite("sailor"));
			sailor2->set_direction(wedge::DIR_E, true, false);
			sailor1->set_looks_around(dont_face1);
			area->add_entity(sailor2);

			Sailor_NPC *sailor3 = new Sailor_NPC("sailor3", "Sailor", "sailor", "sailor3");
			sailor3->set_looks_around(dont_face3);
			sailor3->start(area);
			sailor3->set_position(util::Point<int>(18, 13));
			sailor3->set_direction(wedge::DIR_W, true, false);
			area->add_entity(sailor3);
			
			Sailor_NPC *sailor4 = new Sailor_NPC("sailor4", "Sailor", "sailor", "sailor4");
			sailor4->set_looks_around(dont_face4);
			sailor4->start(area);
			sailor4->set_position(util::Point<int>(18, 14));
			sailor4->set_direction(wedge::DIR_W, true, false);
			area->add_entity(sailor4);

			Sailor *sailor5 = new Sailor("sailor5");
			sailor5->set_looks_around(dont_face5);
			sailor5->start(area);
			sailor5->set_position(util::Point<int>(7, 12));
			sailor5->set_sprite(new gfx::Sprite("sailor"));
			sailor5->set_direction(wedge::DIR_S, true, false);
			area->add_entity(sailor5);
			
			Sailor_NPC *sailor6 = new Sailor_NPC("sailor6", "Sailor", "sailor", "sailor6");
			sailor6->set_looks_around(dont_face6);
			sailor6->start(area);
			sailor6->set_position(util::Point<int>(6, 15));
			sailor6->set_direction(wedge::DIR_N, true, false);
			area->add_entity(sailor6);
		}
		else {
			captain = area->find_entity("captain");
		}
		
		captain->set_direction(wedge::DIR_E, true, false);

		return true;
	}

	void started()
	{
		if (static_cast<Monster_RPG_3_Area_Game *>(AREA)->get_num_areas_created() > 1) {
			autosave(true);
		}

		wedge::Object rune = OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_RUNE, 1);
		if (INSTANCE->is_milestone_complete(MS_JUST_BEFORE_MONSTER) == true && INSTANCE->inventory.find(rune) < 0) {
			INSTANCE->set_milestone_complete(MS_JUST_BEFORE_MONSTER, false);
			captain->set_direction(wedge::DIR_S, true, false);
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1268)/* Originally: Rumours are there's a fierce beast in these parts. Be ready! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data water_anim;
		water_anim.topleft = util::Point<int>(3, 27);
		water_anim.size = util::Size<int>(4, 1);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(3, 28));
		water_anim.frames.push_back(util::Point<int>(3, 29));
		tilemap->add_animation_data(water_anim);
		water_anim.topleft = util::Point<int>(10, 27);
		water_anim.size = util::Size<int>(3, 1);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(10, 28));
		water_anim.frames.push_back(util::Point<int>(10, 29));
		tilemap->add_animation_data(water_anim);
	}
	
	void run()
	{
		wedge::Map_Entity *sailship = area->find_entity("sailship_low");
		if (sailship == NULL) {
			return;
		}
		gfx::Sprite *sprite = sailship->get_sprite();
		gfx::Image *img = sprite->get_current_image();
		util::Point<float> sailship_centre = sailship->get_position() * shim::tile_size - util::Point<int>(0, (img->size.h - shim::tile_size)) + util::Point<float>(img->size.w/2.0f, img->size.h*4.0f/5.0f);

		wedge::Map_Entity_List &entities = area->get_entities();

		for (wedge::Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
			wedge::Map_Entity *e = *it;
			Sailor *s = dynamic_cast<Sailor *>(e);
			if (s) {
				s->set_use_pivot(true);
				s->set_pivot(sailship_centre);
			}
			else {
				Sailor_NPC *s = dynamic_cast<Sailor_NPC *>(e);
				if (s) {
					s->set_use_pivot(true);
					s->set_pivot(sailship_centre);
				}
			}
		}
	}

	void end()
	{
		std::vector<wedge::Map_Entity *> players = AREA->get_players();
		for (size_t i = 0; i < players.size(); i++) {
			Sailor *s = dynamic_cast<Sailor *>(players[i]);
			if (s) {
				s->set_use_pivot(false);
			}
		}
	}

	void talk_to_captain()
	{
		NEW_SYSTEM_AND_TASK(AREA)
		Captain_Step *cs = new Captain_Step(new_task);
		ADD_STEP(cs)
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)

		std::vector<std::string> choices;
		choices.push_back(GLOBALS->game_t->translate(1258)/* Originally: Head East! */);
		choices.push_back(GLOBALS->game_t->translate(1270)/* Originally: Turn back! */);
		choices.push_back(GLOBALS->game_t->translate(1271)/* Originally: Nevermind... */);
		do_question(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1261)/* Originally: At your command, miss! */, wedge::DIALOGUE_SPEECH, choices, cs);
	}
	
	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activated == captain) {
			captain->set_direction(wedge::DIR_S, true, false);
			talk_to_captain();
			return true;
		}
		return false;
	}
	
	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		if (entity != AREA->get_player(ENY)) {
			return false;
		}

		util::Point<int> pos = entity->get_position();

		if (pos == util::Point<int>(13, 11)) {
			maybe_cast();
			return true;
		}
		else {
			return false;
		}
	}

	bool try_tile(wedge::Map_Entity *entity, util::Point<int> tile_pos)
	{
		if (Area_Hooks::try_tile(entity, tile_pos)) {
			return true;
		}

		if (entity != AREA->get_player(ENY)) {
			return false;
		}

		if (tile_pos == util::Point<int>(13, 10)) {
			maybe_cast();
			return true;
		}
		else {
			return false;
		}
	}

	void maybe_cast()
	{
		if (casting) {
			return;
		}

		casting = true;

		NEW_SYSTEM_AND_TASK(AREA)
		Fishing_Step *fs = new Fishing_Step(new_task);
		ADD_STEP(fs)
		ADD_TASK(new_task)
		ANOTHER_TASK
		wedge::Generic_Callback_Step *gcs = new wedge::Generic_Callback_Step(fishing_callback, this, new_task);
		fs->add_monitor(gcs);
		ADD_STEP(gcs);
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)

		std::vector<std::string> choices;
		choices.push_back(GLOBALS->game_t->translate(1729)/* Originally: Yes */);
		choices.push_back(GLOBALS->game_t->translate(1730)/* Originally: No */);
		do_question("", GLOBALS->game_t->translate(1276)/* Originally: Cast a line? */, wedge::DIALOGUE_SPEECH, choices, fs);
	}

	void set_casting(bool casting)
	{
		this->casting = casting;
	}

	bool is_corner_portal(util::Point<int> pos)
	{
		return pos == util::Point<int>(12, 14);
	}

private:
	wedge::Map_Entity *captain;
	bool casting;
};

void fishing_callback(void *data)
{
	Area_Hooks_At_Sea *ah = static_cast<Area_Hooks_At_Sea *>(data);
	ah->set_casting(false);
}

static void boatin_callback(void *data);
static void no_anchor_callback(void *data);
static void splash_callback1(void *data);
static void splash_callback2(void *data);

#define DEFAULT_SPEED 0.05f
#define BOATIN_SPEED_E 0.1f
#define BOATIN_SPEED_W 0.2f

class Area_Hooks_Boatin : public Area_Hooks_Monster_RPG_3
{
public:
	static const int battle_every = 30;

	Area_Hooks_Boatin(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		set_path_if_stopped(true),
		retrying_boss(false),
		doing_splash(false)
	{
	}
	
	virtual ~Area_Hooks_Boatin()
	{
		delete sailship;
		delete wake;
	}

	std::string string_from_dir(wedge::Direction direction)
	{
		switch (direction) {
			case wedge::DIR_N:
				return "stand_n";
			case wedge::DIR_E:
				return "stand_e";
			case wedge::DIR_W:
				return "stand_w";
			case wedge::DIR_S:
				return "stand_s";
			case wedge::DIR_NONE:
				return "";
		}

		return "";
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		if (GLOBALS->retried_boss) {
			shim::music = new audio::MML("music/boatin.mml");
		}
		else {
			audio::play_music("music/boatin.mml");
		}

		sailship = new gfx::Sprite("sailship");
		wake = new gfx::Sprite("wake");

		std::string s = string_from_dir(M3_INSTANCE->boatin_direction);
		sailship->set_animation(s);
		wake->set_animation(s);

		if (GLOBALS->retried_boss) {
			M3_INSTANCE->boatin_done = true;
			wedge::Battle_Game *battle = new Battle_Monster();
			battle->start_transition_in();
			retrying_boss = true;
		}

		if (loaded == false) {
			wedge::Map_Entity *splash = new wedge::Map_Entity("splash");
			splash->start(area);
			splash->set_position(util::Point<int>(361, 6));
			splash->set_sprite(new gfx::Sprite("splash"));
			splash->set_visible(false);
			area->add_entity(splash);
		}

		return true;
	}

	void started()
	{
		std::vector<wedge::Map_Entity *> players = AREA->get_players();

		for (size_t i = 0; i < players.size(); i++) {
			wedge::Map_Entity *p = players[i];
			p->set_visible(false);
		}

		wedge::pause_presses(true);

		if (GLOBALS->retried_boss) {
			players[0]->set_position(M3_GLOBALS->pre_boss_boatin_pos);
		}
		else {
			players[0]->set_position(M3_INSTANCE->boatin_pos);
			if (static_cast<Monster_RPG_3_Area_Game *>(AREA)->get_num_areas_created() <= 1) {
				set_path_if_stopped = false;
				orders();
			}
			else {
				set_path();
			}
		}

		Area_Hooks_Monster_RPG_3::started();
	}
	
	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data water_anim;
		water_anim.topleft = util::Point<int>(24, 21);
		water_anim.size = util::Size<int>(2, 2);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(26, 21));
		tilemap->add_animation_data(water_anim);
	}

	void end()
	{
		std::vector<wedge::Map_Entity *> players = AREA->get_players();

		for (size_t i = 0; i < players.size(); i++) {
			wedge::Map_Entity *p = players[i];
			p->set_visible(true);
		}

		wedge::pause_presses(false);
	}

	void set_path()
	{
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		util::Point<int> pos = eny->get_position();
		bool w = M3_INSTANCE->boat_w;

		util::Point<int> dest;
		
		set_speed(false);

		if (w) {
			if (pos == util::Point<int>(385, 6)) {
				// port
				M3_INSTANCE->boatin_pos = eny->get_position();
				M3_INSTANCE->boatin_direction = eny->get_direction();

				set_speed(true);

				std::vector< util::Point<int> > positions;
				positions.push_back(util::Point<int>(6, 24));
				positions.push_back(util::Point<int>(7, 24));
				std::vector<wedge::Direction> directions;
				directions.push_back(wedge::DIR_S);
				directions.push_back(wedge::DIR_S);
				area->set_next_area("beach_e", positions, directions);
				M3_INSTANCE->boatin = false;
				M3_INSTANCE->boat_w = false;

				return;
			}
			else {
				// HERE!
				dest = util::Point<int>(385, 6);
			}
		}
		else {
			if (pos == util::Point<int>(8, 6)) {
				// port
				M3_INSTANCE->boatin_pos = eny->get_position();
				M3_INSTANCE->boatin_direction = eny->get_direction();
				
				set_speed(true);

				std::vector< util::Point<int> > positions;
				positions.push_back(util::Point<int>(6, 44));
				positions.push_back(util::Point<int>(7, 44));
				std::vector<wedge::Direction> directions;
				directions.push_back(wedge::DIR_S);
				directions.push_back(wedge::DIR_S);
				area->set_next_area("beach_w", positions, directions);
				M3_INSTANCE->boatin = false;
				M3_INSTANCE->boat_w = true;
				return;
			}
			else {
				if (pos == util::Point<int>(385, 6)) {
					dest = util::Point<int>(386, 6);
				}
				else if (pos == util::Point<int>(386, 6)) {
					dest = util::Point<int>(386, 7);
				}
				else if (pos == util::Point<int>(386, 7)) {
					dest = util::Point<int>(7, 7);
				}
				else if (pos == util::Point<int>(7, 7)) {
					dest = util::Point<int>(7, 6);
				}
				else if (pos == util::Point<int>(7, 6)) {
					dest = util::Point<int>(8, 6);
				}
				else {
					dest = util::Point<int>(7, 7);
				}
			}
		}

		wedge::Map_Entity_Input_Step *meis = eny->get_input_step();
		meis->set_path(dest, false, false);
	}
	
	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 0) {
			wedge::Map_Entity *eny = AREA->get_player(ENY);
			wedge::Map_Entity_Input_Step *meis = eny->get_input_step();
			
			gfx::Image *img;

			if (meis->is_following_path()) {
				img = wake->get_current_image();
				img->draw(util::Point<int>(shim::screen_size.w/2-img->size.w/2, shim::screen_size.h/2-img->size.h/2));
			}

			img = sailship->get_current_image();
			img->draw(util::Point<int>(shim::screen_size.w/2-img->size.w/2, shim::screen_size.h/2-img->size.h/2));
		}
		else if (GLOBALS->retried_boss && layer == 3) {
			gfx::clear(GLOBALS->gameover_fade_colour);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(0);
		if (GLOBALS->retried_boss) {
			v.push_back(3);
		}
		return v;
	}

	void run()
	{
		wedge::Map_Entity *eny = AREA->get_player(ENY);

		std::string s = string_from_dir(eny->get_direction());
		sailship->set_animation(s);
		if (wake->get_animation() != s) {
			wake->set_animation(s);
		}

		if (set_path_if_stopped && M3_INSTANCE->boatin && retrying_boss == false && doing_splash == false) {
			wedge::Map_Entity_Input_Step *meis = eny->get_input_step();
			if (meis->is_following_path() == false) {
				set_path();
			}
		}
	}

	void orders()
	{
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		M3_INSTANCE->boatin_pos = eny->get_position();
		M3_INSTANCE->boatin_direction = eny->get_direction();

		std::vector<std::string> choices;
		choices.push_back(GLOBALS->game_t->translate(1277)/* Originally: Keep going! */);
		choices.push_back(GLOBALS->game_t->translate(1278)/* Originally: Drop anchor. */);
		choices.push_back(GLOBALS->game_t->translate(1279)/* Originally: Turn back... */);
		Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(true, GLOBALS->game_t->translate(1280)/* Originally: Captain's orders? */, choices, -1, 0, 0, 0, 0, 0, 0, 0.03f, 0.03f, boatin_callback, this, 3, 150, true, 0.75f);
		gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
		shim::guis.push_back(gui);
	}
	
	void keep_going()
	{
		set_path_if_stopped = true;
		set_path();
	}

	void battle_ended(wedge::Battle_Game *battle)
	{
		if (dynamic_cast<Battle_Monster *>(battle) != NULL) {
			retrying_boss = false;
			area->remove_entity(area->find_entity("splash"), true);
			keep_going();
		}
		else {
			wedge::Map_Entity *eny = AREA->get_player(ENY);
			util::Point<int> pos = eny->get_position();
			if (pos.x == 300) { // last normal battle at 300, always stop here for a speech from Captain
				M3_INSTANCE->boatin_pos = eny->get_position();
				M3_INSTANCE->boatin_direction = eny->get_direction();
				INSTANCE->set_milestone_complete(MS_JUST_BEFORE_MONSTER, true);
				std::vector< util::Point<int> > positions;
				positions.push_back(util::Point<int>(6, 14));
				positions.push_back(util::Point<int>(7, 14));
				std::vector<wedge::Direction> directions;
				directions.push_back(wedge::DIR_N);
				directions.push_back(wedge::DIR_N);
				area->set_next_area("at_sea", positions, directions);
			}
			else {
				if (pos.x == 120 || pos.x == 150 || pos.x == 180 || pos.x == 270) {
					M3_INSTANCE->boatin_pos = eny->get_position();
					M3_INSTANCE->boatin_direction = eny->get_direction();
					autosave(true);
				}
				orders();
			}
		}
	}

	bool has_battles()
	{
		return true;
	}

	void choice(int choice)
	{
		if (choice == 1) {
			wedge::Map_Entity *eny = AREA->get_player(ENY);
			int x = eny->get_position().x;
			if (x != 120 && x != 150 && x != 180 && x != 270) {
				NEW_SYSTEM_AND_TASK(AREA)
				wedge::Generic_Callback_Step *cs = new wedge::Generic_Callback_Step(no_anchor_callback, this, new_task);
				ADD_STEP(cs)
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)

				GLOBALS->do_dialogue(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1282)/* Originally: Sea's too deep to anchor here! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, cs);
			}
			else {
				std::vector< util::Point<int> > positions;
				positions.push_back(util::Point<int>(6, 14));
				positions.push_back(util::Point<int>(7, 14));
				std::vector<wedge::Direction> directions;
				directions.push_back(wedge::DIR_S);
				directions.push_back(wedge::DIR_S);
				area->set_next_area("at_sea", positions, directions);
			}
		}
		else if (choice == 2) {
			set_path_if_stopped = true;
			M3_INSTANCE->boat_w = false;
			wedge::Map_Entity *eny = AREA->get_player(ENY);
			util::Point<int> pos = eny->get_position();
			pos.y++;
			set_speed(false);
			wedge::Map_Entity_Input_Step *meis = eny->get_input_step();
			meis->set_path(pos, false, false);
		}
		else {
			keep_going();
		}
	}

	void set_speed(bool _default)
	{
		wedge::Map_Entity *eny = AREA->get_player(ENY);

		if (_default) {
			eny->set_speed(DEFAULT_SPEED);
		}
		else {
			if (M3_INSTANCE->boat_w) {
				if (INSTANCE->is_milestone_complete(MS_ARRIVED_EAST)) {
					eny->set_speed(BOATIN_SPEED_W);
				}
				else {
					eny->set_speed(BOATIN_SPEED_E);
				}
			}
			else {
				eny->set_speed(BOATIN_SPEED_W);
			}
		}
	}
	
	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		if (entity != AREA->get_player(ENY)) {
			return false;
		}
		
		if (M3_INSTANCE->boat_w == false) {
			return false;
		}

		util::Point<int> pos = entity->get_position();

		bool go, boss;

		if (M3_INSTANCE->boatin_done) {
			go = boss = false;
		}
		else if (pos.x > 300) { // last normal battle at x=300
			if (pos.x == 360) { // boss at x=360
				go = true;
				boss = true;
			}
			else {
				go = boss = false;
			}
		}
		else {
			go = pos.x % battle_every == 0;
			boss = false;
		}

		if (go) {
			if (rand_battle_table.size() == 0) {
				gen_rand_battle_table(4);
			}

			int type = rand_battle_table[rand_battle_table.size()-1];
			rand_battle_table.pop_back();

			set_path_if_stopped = false;

			set_speed(true);

			wedge::Battle_Game *battle = NULL;

			if (boss) {
				wedge::Map_Entity *splash = area->find_entity("splash");
				splash->set_visible(true);
				M3_GLOBALS->splash->play(false);
				gfx::Sprite *sprite = splash->get_sprite();
				sprite->set_animation("only", splash_callback1, this);
				sprite->reset();
				sprite->start();
				doing_splash = true;
			}
			else if (type == 3) {
				battle = new Battle_3Tentacle();
			}
			else if (type == 2) {
				battle = new Battle_Wave();
			}
			else if (type == 1) {
				battle = new Battle_3Shocker();
			}
			else {
				battle = new Battle_1Shocker2Tentacle();
			}

			if (battle) {
				battle->start_transition_in();
			}

			return true;
		}
		else {
			return false;
		}
	}

	void do_boss()
	{
		GLOBALS->boss_press = wedge::DIR_NONE;
		GLOBALS->boss_save = wedge::save();
		M3_GLOBALS->pre_boss_boatin_pos = AREA->get_player(ENY)->get_position();
		M3_INSTANCE->boatin_done = true;
		wedge::Battle_Game *battle = new Battle_Monster();
		battle->start_transition_in();
	}

	void splash1()
	{
		wedge::Map_Entity *splash = area->find_entity("splash");
		M3_GLOBALS->splash->play(false);
		gfx::Sprite *sprite = splash->get_sprite();
		sprite->set_animation("only", splash_callback2, this);
		sprite->reset();
		sprite->start();
	}

	void splash2()
	{
		doing_splash = false;
		area->find_entity("splash")->set_visible(false);
		do_boss();
	}

private:
	gfx::Sprite *sailship;
	gfx::Sprite *wake;
	bool set_path_if_stopped;
	bool retrying_boss;
	bool doing_splash;
};

static void boatin_callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Area_Hooks_Boatin *hooks = static_cast<Area_Hooks_Boatin *>(d->userdata);
	hooks->choice(d->choice);
}

static void no_anchor_callback(void *data)
{
	Area_Hooks_Boatin *hooks = static_cast<Area_Hooks_Boatin *>(data);
	hooks->orders();
}

void splash_callback1(void *data)
{
	Area_Hooks_Boatin *hooks = static_cast<Area_Hooks_Boatin *>(data);
	hooks->splash1();
}

void splash_callback2(void *data)
{
	Area_Hooks_Boatin *hooks = static_cast<Area_Hooks_Boatin *>(data);
	hooks->splash2();
}

class Area_Hooks_Beach_E : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Beach_E(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, false);
		
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(42, 1));
		fz1.area_name = "desert1";
		fz1.player_positions.push_back(util::Point<int>(9, 19));
		fz1.player_positions.push_back(util::Point<int>(10, 19));
		fz1.directions.push_back(wedge::DIR_N);
		fz1.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(12, 24), util::Size<int>(1, 1));
		fz2.area_name = "below_deck";
		fz2.player_positions.push_back(util::Point<int>(1, 5));
		fz2.player_positions.push_back(util::Point<int>(1, 5));
		fz2.directions.push_back(wedge::DIR_E);
		fz2.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_Beach_E()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/boatin.mml");

		if (loaded == false) {
			scroll_dealer = new wedge::Map_Entity("Scroll Dealer");
			scroll_dealer->start(area);
			scroll_dealer->set_position(util::Point<int>(17, 10));
			scroll_dealer->set_sprite(new gfx::Sprite("scroll_dealer"));
			area->add_entity(scroll_dealer);
			
			weapons_dealer = new wedge::Map_Entity("weapons_dealer");
			weapons_dealer->start(area);
			weapons_dealer->set_position(util::Point<int>(15, 10));
			gfx::Sprite *weapons_dealer_sprite = new gfx::Sprite("druid");
			weapons_dealer->set_sprite(weapons_dealer_sprite);
			area->add_entity(weapons_dealer);

			armour_dealer = new wedge::Map_Entity("armour_dealer");
			armour_dealer->start(area);
			armour_dealer->set_position(util::Point<int>(16, 10));
			gfx::Sprite *armour_dealer_sprite = new gfx::Sprite("druid");
			armour_dealer->set_sprite(armour_dealer_sprite);
			area->add_entity(armour_dealer);

			wedge::NPC *druid = new wedge::NPC("Druid", "Druid", "druid", "druid");
			druid->start(area);
			druid->set_position(util::Point<int>(13, 6));
			druid->set_direction(wedge::DIR_S, true, false);
			area->add_entity(druid);

			Sail_Ship *sailship_low = new Sail_Ship("sailship_low");
			sailship_low->start(area);
			sailship_low->set_position(util::Point<int>(3, 28));
			sailship_low->set_sprite(new gfx::Sprite("sailship_low"));
			sailship_low->set_layer(1);
			area->add_entity(sailship_low);
			
			Sail_Ship *sailship_high = new Sail_Ship("sailship_high");
			sailship_high->start(area);
			sailship_high->set_position(util::Point<int>(3, 28));
			sailship_high->set_sprite(new gfx::Sprite("sailship_high"));
			sailship_high->set_layer(3);
			area->add_entity(sailship_high);
		
			captain = new Sailor("captain");
			captain->start(area);
			captain->set_position(util::Point<int>(6, 23));
			captain->set_sprite(new gfx::Sprite("captain"));
			captain->set_direction(wedge::DIR_E, true, false);
			area->add_entity(captain);

			std::vector<wedge::Direction> dont_face1, dont_face2, dont_face3, dont_face4, dont_face5, dont_face6;
			dont_face3.push_back(wedge::DIR_E);
			dont_face4.push_back(wedge::DIR_E);
			dont_face5.push_back(wedge::DIR_E);
			dont_face5.push_back(wedge::DIR_N);
		
			Sailor *sailor1 = new Sailor("sailor1");
			sailor1->set_looks_around(dont_face1);
			sailor1->start(area);
			sailor1->set_position(util::Point<int>(25, 22));
			sailor1->set_sprite(new gfx::Sprite("sailor"));
			sailor1->set_direction(wedge::DIR_E, true, false);
			area->add_entity(sailor1);
			
			Sailor *sailor2 = new Sailor("sailor2");
			sailor2->set_looks_around(dont_face2);
			sailor2->start(area);
			sailor2->set_position(util::Point<int>(25, 25));
			sailor2->set_sprite(new gfx::Sprite("sailor"));
			sailor2->set_direction(wedge::DIR_E, true, false);
			sailor1->set_looks_around(dont_face1);
			area->add_entity(sailor2);

			Sailor_NPC *sailor3 = new Sailor_NPC("sailor3", "Sailor", "sailor", "sailor3");
			sailor3->set_looks_around(dont_face3);
			sailor3->start(area);
			sailor3->set_position(util::Point<int>(18, 23));
			sailor3->set_direction(wedge::DIR_W, true, false);
			area->add_entity(sailor3);
			
			Sailor_NPC *sailor4 = new Sailor_NPC("sailor4", "Sailor", "sailor", "sailor4");
			sailor4->set_looks_around(dont_face4);
			sailor4->start(area);
			sailor4->set_position(util::Point<int>(18, 24));
			sailor4->set_direction(wedge::DIR_W, true, false);
			area->add_entity(sailor4);

			Sailor *sailor5 = new Sailor("sailor5");
			sailor5->set_looks_around(dont_face5);
			sailor5->start(area);
			sailor5->set_position(util::Point<int>(7, 22));
			sailor5->set_sprite(new gfx::Sprite("sailor"));
			sailor5->set_direction(wedge::DIR_S, true, false);
			area->add_entity(sailor5);
			
			Sailor_NPC *sailor6 = new Sailor_NPC("sailor6", "Sailor", "sailor", "sailor6");
			sailor6->set_looks_around(dont_face6);
			sailor6->start(area);
			sailor6->set_position(util::Point<int>(6, 25));
			sailor6->set_direction(wedge::DIR_N, true, false);
			area->add_entity(sailor6);
		}
		else {
			captain = area->find_entity("captain");
			scroll_dealer = area->find_entity("Scroll Dealer");
			weapons_dealer = area->find_entity("weapons_dealer");
			armour_dealer = area->find_entity("armour_dealer");
		}

		captain->set_direction(wedge::DIR_E, true, false);

		return true;
	}

	void started()
	{
		if (INSTANCE->is_milestone_complete(MS_ARRIVED_EAST) == false) {
			if (static_cast<Monster_RPG_3_Area_Game *>(AREA)->get_num_areas_created() > 1) {
				autosave(true);
			}
			INSTANCE->set_milestone_complete(MS_ARRIVED_EAST, true);
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1284)/* Originally: Here we are, miss! */ + NEW_PARAGRAPH + GLOBALS->game_t->translate(1285)/* Originally: The Kingdom is North of here, through the ummm, errr, desert! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, NULL);
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data water_anim;
		water_anim.topleft = util::Point<int>(3, 27);
		water_anim.size = util::Size<int>(4, 1);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(3, 28));
		water_anim.frames.push_back(util::Point<int>(3, 29));
		tilemap->add_animation_data(water_anim);
		water_anim.topleft = util::Point<int>(10, 27);
		water_anim.size = util::Size<int>(3, 1);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(10, 28));
		water_anim.frames.push_back(util::Point<int>(10, 29));
		tilemap->add_animation_data(water_anim);
	}
	
	void run()
	{
		wedge::Map_Entity *sailship = area->find_entity("sailship_low");
		if (sailship == NULL) {
			return;
		}
		gfx::Sprite *sprite = sailship->get_sprite();
		gfx::Image *img = sprite->get_current_image();
		util::Point<float> sailship_centre = sailship->get_position() * shim::tile_size - util::Point<int>(0, (img->size.h - shim::tile_size)) + util::Point<float>(img->size.w/2.0f, img->size.h*4.0f/5.0f);

		wedge::Map_Entity_List &entities = area->get_entities();

		for (wedge::Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
			wedge::Map_Entity *e = *it;
			Sailor *s = dynamic_cast<Sailor *>(e);
			if (s) {
				util::Point<int> position = s->get_position();
				if (position.y >= 21) {
					s->set_use_pivot(true);
					s->set_pivot(sailship_centre);
				}
				else {
					s->set_use_pivot(false);
				}
			}
			else {
				Sailor_NPC *s = dynamic_cast<Sailor_NPC *>(e);
				if (s) {
					util::Point<int> position = s->get_position();
					if (position.y >= 21) {
						s->set_use_pivot(true);
						s->set_pivot(sailship_centre);
					}
					else {
						s->set_use_pivot(false);
					}
				}
			}
		}
	}

	void end()
	{
		std::vector<wedge::Map_Entity *> players = AREA->get_players();
		for (size_t i = 0; i < players.size(); i++) {
			Sailor *s = dynamic_cast<Sailor *>(players[i]);
			if (s) {
				s->set_use_pivot(false);
			}
		}
	}
	
	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activator == AREA->get_player(ENY) && activated == captain) {
			captain->set_direction(wedge::DIR_S, true, false);

			NEW_SYSTEM_AND_TASK(AREA)
			Captain_Step *cs = new Captain_Step(new_task);
			ADD_STEP(cs)
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			std::vector<std::string> choices;
			choices.push_back(GLOBALS->game_t->translate(1286)/* Originally: Head West! */);
			choices.push_back(GLOBALS->game_t->translate(1259)/* Originally: Maybe later... */);
			do_question(GLOBALS->game_t->translate(87)/* Originally: Captain */ + TAG_END, GLOBALS->game_t->translate(1261)/* Originally: At your command, miss! */, wedge::DIALOGUE_SPEECH, choices, cs);

			return true;
		}
		else if (activated == scroll_dealer) {
			bool has_ice = false;
			wedge::Object o = OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ICE_SCROLL, 1);
			int index = INSTANCE->inventory.find(o);
			if (index < 0) {
				std::vector<std::string> spells1 = INSTANCE->stats[ENY].base.get_spells();
				std::vector<std::string> spells2 = INSTANCE->stats[TIGGY].base.get_spells();
				has_ice = std::find(spells1.begin(), spells1.end(), "Ice") != spells1.end() ||
					std::find(spells2.begin(), spells2.end(), "Ice") != spells2.end();
			}
			else {
				has_ice = true;
			}
			if (has_ice) {
				GLOBALS->do_dialogue(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1554)/* Originally: There's nothing more I can teach you... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
			}
			else {
				NEW_SYSTEM_AND_TASK(AREA)
				Buy_Scroll_Step *bss = new Buy_Scroll_Step(ITEM_ICE_SCROLL, 1000, new_task);
				ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
				ADD_STEP(bss)
				ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)

				std::vector<std::string> choices;
				choices.push_back(GLOBALS->game_t->translate(1729)/* Originally: Yes */);
				choices.push_back(GLOBALS->game_t->translate(1730)/* Originally: No */);
				do_question(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1294)/* Originally: Would you like to buy an Ice Scroll? Only 1000 gold! */, wedge::DIALOGUE_SPEECH, choices, bss);
			}
			return true;
		}
		if (activated == weapons_dealer) {
			weapons_dealer->face(AREA->get_player(ENY), false);
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(93)/* Originally: Druid */ + TAG_END, GLOBALS->game_t->translate(1296)/* Originally: You'll be needing protection if you're heading North... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_LONG_SWORD, 1000));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_WEAPON, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_STEP(new wedge::Set_Direction_Step(weapons_dealer, wedge::DIR_S, true, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}
		else if (activated == armour_dealer) {
			armour_dealer->face(AREA->get_player(ENY), false);
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(93)/* Originally: Druid */ + TAG_END, GLOBALS->game_t->translate(1298)/* Originally: Western armour is no good here! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_ARMOUR, ARMOUR_PLATE_MAIL, 500));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_ARMOUR, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_STEP(new wedge::Set_Direction_Step(armour_dealer, wedge::DIR_S, true, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}
		return false;
	}

	bool is_corner_portal(util::Point<int> pos)
	{
		return pos == util::Point<int>(12, 24);
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			std::string next_area = area->get_next_area_name();

			if (next_area == "desert1") {
				AREA->set_sfx_volume_to_zero_next_area_fade(true);
				AREA->set_fade_sfx_volume(true);
			}

			return true;
		}

		return false;
	}

private:
	wedge::Map_Entity *captain;
	wedge::Map_Entity *scroll_dealer;
	wedge::Map_Entity *weapons_dealer;
	wedge::Map_Entity *armour_dealer;
};

class Area_Hooks_Desert1 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Desert1(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		samples_done(false)
	{
		Scroll_Zone z;
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(19, 0);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(19, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(20, 1));
		z.area_name = "desert2";
		z.topleft_dest = util::Point<int>(0, 19);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);
		
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(0, 19), util::Size<int>(20, 1));
		fz1.area_name = "beach_e";
		fz1.player_positions.push_back(util::Point<int>(12, 0));
		fz1.player_positions.push_back(util::Point<int>(13, 0));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Desert1()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/desert.mml");

		if (loaded == false) {
		}

		return true;
	}
	
	void started()
	{
		if (INSTANCE->is_milestone_complete(MS_HOW_DID_WE_GET_HERE) == true) {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1300)/* Originally: Huh? How did we wind up back here??? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
		}

		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, true);
		
		if (M3_GLOBALS->wind1->is_playing() == false) {
			M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
			// Must start one of these here so check in run() works
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			GLOBALS->max_battle_steps = Monster_RPG_3_Globals::MAX_BATTLE_STEPS * 0.667f;
			GLOBALS->min_battle_steps = Monster_RPG_3_Globals::MIN_BATTLE_STEPS * 0.667f;
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_1Sandworm2Flare();
		}
		else if (type == 2) {
			return new Battle_1Bones2Flare();
		}
		else if (type == 1) {
			return new Battle_2Bones();
		}
		else {
			return new Battle_3Cyclone();
		}
	}
	
	void run()
	{
		if (M3_GLOBALS->wind2->is_done() == true || M3_GLOBALS->wind3->is_done() == true) {
			// Stop them to set done = false do above condition will be true
			M3_GLOBALS->wind2->stop();
			M3_GLOBALS->wind3->stop();
			samples_done = true;
			next_play = GET_TICKS() + util::rand(0, 10000);
		}

		if (samples_done && GET_TICKS() >= next_play) {
			samples_done = false;
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			std::string next_area = area->get_next_area_name();

			if (next_area == "beach_e") {
				AREA->set_fade_sfx_volume(true);
			}

			return true;
		}

		return false;
	}

private:
	bool samples_done;
	Uint32 next_play;
};

class Area_Hooks_Desert2 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Desert2(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		samples_done(false)
	{
		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, true);
		
		Scroll_Zone z;
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(19, 0);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(19, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(20, 1));
		z.area_name = "desert3";
		z.topleft_dest = util::Point<int>(0, 19);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 19), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Desert2()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/desert.mml");
		
		if (loaded == false) {
			wedge::Chest *chest1 = new wedge::Chest("chest1", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION_PLUS, 3));
			chest1->start(area);
			chest1->set_position(util::Point<int>(11, 11));
			area->add_entity(chest1);
		}

		return true;
	}
	
	void started()
	{
		if (M3_GLOBALS->wind1->is_playing() == false) {
			M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
			// Must start one of these here so check in run() works
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_1Sandworm2Flare();
		}
		else if (type == 2) {
			return new Battle_1Bones2Flare();
		}
		else if (type == 1) {
			return new Battle_2Bones();
		}
		else {
			return new Battle_3Cyclone();
		}
	}
	
	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			if (entity == AREA->get_player(ENY) && entity->get_position().y == area->get_tilemap()->get_size().h-1) {
				INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, false);
			}
			return true;
		}

		return false;
	}
	
	void run()
	{
		if (M3_GLOBALS->wind2->is_done() == true || M3_GLOBALS->wind3->is_done() == true) {
			// Stop them to set done = false do above condition will be true
			M3_GLOBALS->wind2->stop();
			M3_GLOBALS->wind3->stop();
			samples_done = true;
			next_play = GET_TICKS() + util::rand(0, 10000);
		}

		if (samples_done && GET_TICKS() >= next_play) {
			samples_done = false;
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}
	}

private:
	bool samples_done;
	Uint32 next_play;
};

class Area_Hooks_Desert3 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Desert3(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		samples_done(false)
	{
		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, true);
		
		Scroll_Zone z;
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(1, 20));
		z.area_name = "desert4";
		z.topleft_dest = util::Point<int>(19, 0);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(19, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 19);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 19), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Desert3()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/desert.mml");
		
		if (loaded == false) {
		}

		return true;
	}
	
	void started()
	{
		if (M3_GLOBALS->wind1->is_playing() == false) {
			M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
			// Must start one of these here so check in run() works
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_1Sandworm2Flare();
		}
		else if (type == 2) {
			return new Battle_1Bones2Flare();
		}
		else if (type == 1) {
			return new Battle_2Bones();
		}
		else {
			return new Battle_3Cyclone();
		}
	}
	
	void run()
	{
		if (M3_GLOBALS->wind2->is_done() == true || M3_GLOBALS->wind3->is_done() == true) {
			// Stop them to set done = false do above condition will be true
			M3_GLOBALS->wind2->stop();
			M3_GLOBALS->wind3->stop();
			samples_done = true;
			next_play = GET_TICKS() + util::rand(0, 10000);
		}

		if (samples_done && GET_TICKS() >= next_play) {
			samples_done = false;
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}
	}

private:
	bool samples_done;
	Uint32 next_play;
};

class Area_Hooks_Desert4 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Desert4(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		samples_done(false)
	{
		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, true);
		
		Scroll_Zone z;
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(19, 0);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(19, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(20, 1));
		z.area_name = "desert5";
		z.topleft_dest = util::Point<int>(0, 19);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 19), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Desert4()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/desert.mml");
		
		if (loaded == false) {
			wedge::Chest *chest1 = new wedge::Chest("chest1", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_ICE_SICKLE, 1));
			chest1->start(area);
			chest1->set_position(util::Point<int>(2, 13));
			area->add_entity(chest1);
		}

		return true;
	}
	
	void started()
	{
		if (M3_GLOBALS->wind1->is_playing() == false) {
			M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
			// Must start one of these here so check in run() works
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_1Sandworm2Flare();
		}
		else if (type == 2) {
			return new Battle_1Bones2Flare();
		}
		else if (type == 1) {
			return new Battle_2Bones();
		}
		else {
			return new Battle_3Cyclone();
		}
	}
	
	void run()
	{
		if (M3_GLOBALS->wind2->is_done() == true || M3_GLOBALS->wind3->is_done() == true) {
			// Stop them to set done = false do above condition will be true
			M3_GLOBALS->wind2->stop();
			M3_GLOBALS->wind3->stop();
			samples_done = true;
			next_play = GET_TICKS() + util::rand(0, 10000);
		}

		if (samples_done && GET_TICKS() >= next_play) {
			samples_done = false;
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}
	}

private:
	bool samples_done;
	Uint32 next_play;
};

class Area_Hooks_Desert5 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Desert5(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		samples_done(false)
	{
		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, true);
		
		Scroll_Zone z;
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(19, 0);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(19, 0), util::Size<int>(1, 20));
		z.area_name = "desert6";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 19);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 19), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Desert5()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/desert.mml");
		
		if (loaded == false) {
		}

		return true;
	}
	
	void started()
	{
		if (M3_GLOBALS->wind1->is_playing() == false) {
			M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
			// Must start one of these here so check in run() works
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_1Sandworm2Flare();
		}
		else if (type == 2) {
			return new Battle_1Bones2Flare();
		}
		else if (type == 1) {
			return new Battle_2Bones();
		}
		else {
			return new Battle_3Cyclone();
		}
	}
	
	void run()
	{
		if (M3_GLOBALS->wind2->is_done() == true || M3_GLOBALS->wind3->is_done() == true) {
			// Stop them to set done = false do above condition will be true
			M3_GLOBALS->wind2->stop();
			M3_GLOBALS->wind3->stop();
			samples_done = true;
			next_play = GET_TICKS() + util::rand(0, 10000);
		}

		if (samples_done && GET_TICKS() >= next_play) {
			samples_done = false;
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}
	}

#if 0 // don't want this... makes me think I've went the wrong way and the "How did we get here?" message is popping up
	void started()
	{
		if (INSTANCE->is_milestone_complete(MS_WERE_ON_THE_RIGHT_TRACK) == false) {
			INSTANCE->set_milestone_complete(MS_WERE_ON_THE_RIGHT_TRACK, true);
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(1683)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1684)/* Originally: We're on the right track! I know it! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
		}

		Area_Hooks_Monster_RPG_3::started();
	}
#endif

private:
	bool samples_done;
	Uint32 next_play;
};

class Area_Hooks_Desert6 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Desert6(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		samples_done(false)
	{
		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, true);
		
		Scroll_Zone z;
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(19, 0);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(19, 0), util::Size<int>(1, 20));
		z.area_name = "desert7";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 19);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 19), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Desert6()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/desert.mml");
		
		if (loaded == false) {
			wedge::Chest *chest1 = new wedge::Chest("chest1", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 2));
			chest1->start(area);
			chest1->set_position(util::Point<int>(8, 5));
			area->add_entity(chest1);
		}

		return true;
	}
	
	void started()
	{
		if (M3_GLOBALS->wind1->is_playing() == false) {
			M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
			// Must start one of these here so check in run() works
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_1Sandworm2Flare();
		}
		else if (type == 2) {
			return new Battle_1Bones2Flare();
		}
		else if (type == 1) {
			return new Battle_2Bones();
		}
		else {
			return new Battle_3Cyclone();
		}
	}
	
	void run()
	{
		if (M3_GLOBALS->wind2->is_done() == true || M3_GLOBALS->wind3->is_done() == true) {
			// Stop them to set done = false do above condition will be true
			M3_GLOBALS->wind2->stop();
			M3_GLOBALS->wind3->stop();
			samples_done = true;
			next_play = GET_TICKS() + util::rand(0, 10000);
		}

		if (samples_done && GET_TICKS() >= next_play) {
			samples_done = false;
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}
	}

private:
	bool samples_done;
	Uint32 next_play;
};

class Area_Hooks_Desert7 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Desert7(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		samples_done(false)
	{
		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, true);
		
		Scroll_Zone z;
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(19, 0);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(19, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(20, 1));
		z.area_name = "desert8";
		z.topleft_dest = util::Point<int>(0, 19);
		z.direction = wedge::DIR_N;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 19), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Desert7()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/desert.mml");
		
		if (loaded == false) {
		}

		return true;
	}
	
	void started()
	{
		if (M3_GLOBALS->wind1->is_playing() == false) {
			M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
			// Must start one of these here so check in run() works
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}

		Area_Hooks_Monster_RPG_3::started();
	}

	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_1Sandworm2Flare();
		}
		else if (type == 2) {
			return new Battle_1Bones2Flare();
		}
		else if (type == 1) {
			return new Battle_2Bones();
		}
		else {
			return new Battle_3Cyclone();
		}
	}
	
	void run()
	{
		if (M3_GLOBALS->wind2->is_done() == true || M3_GLOBALS->wind3->is_done() == true) {
			// Stop them to set done = false do above condition will be true
			M3_GLOBALS->wind2->stop();
			M3_GLOBALS->wind3->stop();
			samples_done = true;
			next_play = GET_TICKS() + util::rand(0, 10000);
		}

		if (samples_done && GET_TICKS() >= next_play) {
			samples_done = false;
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}
	}

private:
	bool samples_done;
	Uint32 next_play;
};

static void give_vice(void *data)
{
	M3_INSTANCE->add_vampire("vIce");

	NEW_SYSTEM_AND_TASK(AREA)
	ADD_STEP(new wedge::Play_Sound_Step(GLOBALS->levelup, false, false, new_task))
	ADD_STEP(new Dialogue_Step("", GLOBALS->game_t->translate(1301)/* Originally: Learned vIce! */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_AUTO, new_task))
	ADD_TASK(new_task)
	FINISH_SYSTEM(AREA)
}

class Area_Hooks_Desert8 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Desert8(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		samples_done(false)
	{
		INSTANCE->set_milestone_complete(MS_HOW_DID_WE_GET_HERE, true);
		
		Scroll_Zone z;
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(19, 0);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(19, 0), util::Size<int>(1, 20));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	
		z.zone = util::Rectangle<int>(util::Point<int>(0, 19), util::Size<int>(20, 1));
		z.area_name = "desert1";
		z.topleft_dest = util::Point<int>(0, 0);
		z.direction = wedge::DIR_S;
		scroll_zones.push_back(z);
		
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(0, 0), util::Size<int>(20, 1));
		fz1.area_name = "castle_front";
		fz1.player_positions.push_back(util::Point<int>(14, 37));
		fz1.player_positions.push_back(util::Point<int>(15, 37));
		fz1.directions.push_back(wedge::DIR_N);
		fz1.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Desert8()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/desert.mml");
		
		if (loaded == false) {
			util::Point<int> druid_pos(10, 2);
			druid = new wedge::Map_Entity("druid");
			druid->start(area);
			druid->set_position(druid_pos);
			gfx::Sprite *druid_sprite = new gfx::Sprite("druid");
			druid->set_sprite(druid_sprite);
			druid->set_direction(wedge::DIR_S, true, false);
			area->add_entity(druid);
		}
		else {
			druid = area->find_entity("druid");
		}

		return true;
	}
		
	void started()
	{
		if (M3_GLOBALS->wind1->is_playing() == false) {
			M3_GLOBALS->wind1->play(1.0f, true, audio::SAMPLE_TYPE_USER+0);
			// Must start one of these here so check in run() works
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			GLOBALS->max_battle_steps = Monster_RPG_3_Globals::MAX_BATTLE_STEPS * 0.667f;
			GLOBALS->min_battle_steps = Monster_RPG_3_Globals::MIN_BATTLE_STEPS * 0.667f;
		}

		Area_Hooks_Monster_RPG_3::started();
	}
	
	bool has_battles()
	{
		return true;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_1Sandworm2Flare();
		}
		else if (type == 2) {
			return new Battle_1Bones2Flare();
		}
		else if (type == 1) {
			return new Battle_2Bones();
		}
		else {
			return new Battle_3Cyclone();
		}
	}
	
	void run()
	{
		if (M3_GLOBALS->wind2->is_done() == true || M3_GLOBALS->wind3->is_done() == true) {
			// Stop them to set done = false do above condition will be true
			M3_GLOBALS->wind2->stop();
			M3_GLOBALS->wind3->stop();
			samples_done = true;
			next_play = GET_TICKS() + util::rand(0, 10000);
		}

		if (samples_done && GET_TICKS() >= next_play) {
			samples_done = false;
			if (util::rand(0, 1) == 0) {
				M3_GLOBALS->wind2->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
			else {
				M3_GLOBALS->wind3->play(1.0f, false, audio::SAMPLE_TYPE_USER+0);
			}
		}
	}

	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activated == druid) {
			std::vector<std::string> vampires = M3_INSTANCE->get_vampires();
			bool has_vice = false;
			for (size_t i = 0; i < vampires.size(); i++) {
				if (vampires[i] == "vIce") {
					has_vice = true;
					break;
				}
			}

			activated->face(activator, false);

			if (has_vice) {
				NEW_SYSTEM_AND_TASK(AREA)
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(93)/* Originally: Druid */ + TAG_END, GLOBALS->game_t->translate(1303)/* Originally: If you want to get back to the sea, head South! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
				ADD_STEP(new wedge::Set_Direction_Step(activated, wedge::DIR_S, true, false, new_task))
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)
			}
			else {
				NEW_SYSTEM_AND_TASK(AREA)
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(93)/* Originally: Druid */ + TAG_END, GLOBALS->game_t->translate(1305)/* Originally: Nice work, navigating the desert!^You should have this! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
				ADD_STEP(new wedge::Generic_Immediate_Callback_Step(give_vice, NULL, new_task))
				ADD_STEP(new wedge::Set_Direction_Step(activated, wedge::DIR_S, true, false, new_task))
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)
			}

			return true;
		}

		return false;
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			std::string next_area = area->get_next_area_name();

			if (next_area == "castle_front") {
				AREA->set_fade_sfx_volume(true);
			}

			return true;
		}

		return false;
	}

private:
	bool samples_done;
	Uint32 next_play;
	wedge::Map_Entity *druid;
};

class Area_Hooks_Castle_Front : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_Front(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(0, 37), util::Size<int>(29, 1));
		fz1.area_name = "desert8";
		fz1.player_positions.push_back(util::Point<int>(9, 0));
		fz1.player_positions.push_back(util::Point<int>(10, 0));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(14, 12), util::Size<int>(1, 1));
		fz2.area_name = "castle_entrance";
		fz2.player_positions.push_back(util::Point<int>(12, 16));
		fz2.player_positions.push_back(util::Point<int>(12, 16));
		fz2.directions.push_back(wedge::DIR_N);
		fz2.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz2);

		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(14, 7), util::Size<int>(1, 1));
		fz3.area_name = "castle_to_ramparts";
		fz3.player_positions.push_back(util::Point<int>(5, 8));
		fz3.player_positions.push_back(util::Point<int>(5, 8));
		fz3.directions.push_back(wedge::DIR_N);
		fz3.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz3);
	}
	
	virtual ~Area_Hooks_Castle_Front()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {
		}

		if (area->find_entity("chest") == NULL) {
			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(26, 12));
			area->add_entity(chest);
		}

		if (area->find_entity("mm1") == NULL) {
			util::Point<int> mm1_pos(14, 18);
			wedge::NPC *mm1 = new wedge::NPC("mm1", "Mighty Man", "mighty_man", "mm1");
			mm1->set_wanders(true, true, true, true, mm1_pos, 4);
			mm1->start(area);
			mm1->set_position(mm1_pos);
			area->add_entity(mm1);
		}

		if (area->find_entity("mm2") == NULL) {
			util::Point<int> mm2_pos(12, 30);
			wedge::NPC *mm2 = new wedge::NPC("mm2", "Mighty Man", "mighty_man_short", "mm2");
			mm2->start(area);
			mm2->set_position(mm2_pos);
			mm2->set_direction(wedge::DIR_S, true, false);
			area->add_entity(mm2);
		}

		if (area->find_entity("mm3") == NULL) {
			util::Point<int> mm3_pos(16, 30);
			wedge::NPC *mm3 = new wedge::NPC("mm3", "Mighty Man", "mighty_man", "mm3");
			mm3->start(area);
			mm3->set_position(mm3_pos);
			mm3->set_direction(wedge::DIR_S, true, false);
			area->add_entity(mm3);
		}

		if (area->find_entity("mm4") == NULL) {
			util::Point<int> mm4_pos(12, 25);
			wedge::NPC *mm4 = new wedge::NPC("mm4", "Mighty Man", "mighty_man_short", "mm4");
			mm4->start(area);
			mm4->set_layer(2);
			mm4->set_position(mm4_pos);
			mm4->set_direction(wedge::DIR_S, true, false);
			area->add_entity(mm4);
		}

		if (area->find_entity("mm5") == NULL) {
			util::Point<int> mm5_pos(16, 25);
			wedge::NPC *mm5 = new wedge::NPC("mm5", "Mighty Man", "mighty_man_beard", "mm5");
			mm5->start(area);
			mm5->set_layer(2);
			mm5->set_position(mm5_pos);
			mm5->set_direction(wedge::DIR_S, true, false);
			area->add_entity(mm5);
		}

		return true;
	}

	void started()
	{
		wedge::Map_Entity *eny = AREA->get_player(ENY);

		if (eny->get_position() == util::Point<int>(14, 7)) {
			gfx::Tilemap *tilemap = area->get_tilemap();

			tilemap->set_solid(-1, util::Point<int>(8, 25), util::Size<int>(4, 1), false);
			tilemap->set_solid(-1, util::Point<int>(17, 25), util::Size<int>(4, 1), false);
			tilemap->set_solid(-1, util::Point<int>(8, 24), util::Size<int>(4, 1), true);
			tilemap->set_solid(-1, util::Point<int>(17, 24), util::Size<int>(4, 1), true);
		}

		Area_Hooks_Monster_RPG_3::started();
	}
	
	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			std::string next_area = area->get_next_area_name();

			if (next_area == "desert8") {
				AREA->set_sfx_volume_to_zero_next_area_fade(true);
				AREA->set_fade_sfx_volume(true);
			}

			return true;
		}

		return false;
	}
};

class Area_Hooks_Castle_Entrance : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_Entrance(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(12, 16), util::Size<int>(1, 1));
		fz1.area_name = "castle_front";
		fz1.player_positions.push_back(util::Point<int>(14, 12));
		fz1.player_positions.push_back(util::Point<int>(14, 12));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(8, 9), util::Size<int>(1, 1));
		fz2.area_name = "castle_to_ramparts";
		fz2.player_positions.push_back(util::Point<int>(1, 3));
		fz2.player_positions.push_back(util::Point<int>(1, 3));
		fz2.directions.push_back(wedge::DIR_E);
		fz2.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz2);

		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(16, 9), util::Size<int>(1, 1));
		fz3.area_name = "castle_to_ramparts";
		fz3.player_positions.push_back(util::Point<int>(9, 3));
		fz3.player_positions.push_back(util::Point<int>(9, 3));
		fz3.directions.push_back(wedge::DIR_W);
		fz3.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz3);

		Fade_Zone fz4;
		fz4.zone = util::Rectangle<int>(util::Point<int>(2, 9), util::Size<int>(1, 1));
		fz4.area_name = "castle_back";
		fz4.player_positions.push_back(util::Point<int>(13, 34));
		fz4.player_positions.push_back(util::Point<int>(13, 34));
		fz4.directions.push_back(wedge::DIR_S);
		fz4.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz4);

		Fade_Zone fz5;
		fz5.zone = util::Rectangle<int>(util::Point<int>(22, 9), util::Size<int>(1, 1));
		fz5.area_name = "castle_back";
		fz5.player_positions.push_back(util::Point<int>(35, 34));
		fz5.player_positions.push_back(util::Point<int>(35, 34));
		fz5.directions.push_back(wedge::DIR_S);
		fz5.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz5);

		Fade_Zone fz6;
		fz6.zone = util::Rectangle<int>(util::Point<int>(8, 2), util::Size<int>(1, 1));
		fz6.area_name = "castle_throne";
		fz6.player_positions.push_back(util::Point<int>(1, 10));
		fz6.player_positions.push_back(util::Point<int>(1, 10));
		fz6.directions.push_back(wedge::DIR_E);
		fz6.directions.push_back(wedge::DIR_E);
		fade_zones.push_back(fz6);

		Fade_Zone fz7;
		fz7.zone = util::Rectangle<int>(util::Point<int>(16, 2), util::Size<int>(1, 1));
		fz7.area_name = "castle_throne";
		fz7.player_positions.push_back(util::Point<int>(9, 10));
		fz7.player_positions.push_back(util::Point<int>(9, 10));
		fz7.directions.push_back(wedge::DIR_W);
		fz7.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz7);
	}
	
	virtual ~Area_Hooks_Castle_Entrance()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {
			util::Point<int> mm1_pos(9, 5);
			wedge::NPC *mm1 = new wedge::NPC("mm1", "Mighty Man", "mighty_man_blond", "mm6");
			mm1->start(area);
			mm1->set_position(mm1_pos);
			mm1->set_direction(wedge::DIR_S, true, false);
			area->add_entity(mm1);

			util::Point<int> mm2_pos(15, 5);
			wedge::NPC *mm2 = new wedge::NPC("mm2", "Mighty Man", "mighty_man", "mm7");
			mm2->start(area);
			mm2->set_position(mm2_pos);
			mm2->set_direction(wedge::DIR_S, true, false);
			area->add_entity(mm2);
		}

		return true;
	}
};

class Area_Hooks_Castle_To_Ramparts : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_To_Ramparts(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(1, 3), util::Size<int>(1, 1));
		fz1.area_name = "castle_entrance";
		fz1.player_positions.push_back(util::Point<int>(8, 9));
		fz1.player_positions.push_back(util::Point<int>(8, 9));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(9, 3), util::Size<int>(1, 1));
		fz2.area_name = "castle_entrance";
		fz2.player_positions.push_back(util::Point<int>(16, 9));
		fz2.player_positions.push_back(util::Point<int>(16, 9));
		fz2.directions.push_back(wedge::DIR_S);
		fz2.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz2);

		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(5, 8), util::Size<int>(1, 1));
		fz3.area_name = "castle_front";
		fz3.player_positions.push_back(util::Point<int>(14, 7));
		fz3.player_positions.push_back(util::Point<int>(14, 7));
		fz3.directions.push_back(wedge::DIR_S);
		fz3.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz3);
	}
	
	virtual ~Area_Hooks_Castle_To_Ramparts()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {
		}

		return true;
	}
};

class Area_Hooks_Castle_Back : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_Back(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(13, 34), util::Size<int>(1, 1));
		fz1.area_name = "castle_entrance";
		fz1.player_positions.push_back(util::Point<int>(2, 9));
		fz1.player_positions.push_back(util::Point<int>(2, 9));
		fz1.directions.push_back(wedge::DIR_N);
		fz1.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(35, 34), util::Size<int>(1, 1));
		fz2.area_name = "castle_entrance";
		fz2.player_positions.push_back(util::Point<int>(22, 9));
		fz2.player_positions.push_back(util::Point<int>(22, 9));
		fz2.directions.push_back(wedge::DIR_N);
		fz2.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz2);
		
		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(9, 14), util::Size<int>(1, 1));
		fz3.area_name = "castle_shop";
		fz3.player_positions.push_back(util::Point<int>(4, 11));
		fz3.player_positions.push_back(util::Point<int>(4, 11));
		fz3.directions.push_back(wedge::DIR_N);
		fz3.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz3);
		
		Fade_Zone fz4;
		fz4.zone = util::Rectangle<int>(util::Point<int>(39, 14), util::Size<int>(1, 1));
		fz4.area_name = "castle_quarters";
		fz4.player_positions.push_back(util::Point<int>(4, 11));
		fz4.player_positions.push_back(util::Point<int>(4, 11));
		fz4.directions.push_back(wedge::DIR_N);
		fz4.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz4);
		
		Fade_Zone fz5;
		fz5.zone = util::Rectangle<int>(util::Point<int>(15, 0), util::Size<int>(19, 1));
		fz5.area_name = "mountains1";
		fz5.player_positions.push_back(util::Point<int>(24, 53));
		fz5.player_positions.push_back(util::Point<int>(25, 53));
		fz5.directions.push_back(wedge::DIR_N);
		fz5.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz5);
		
		Scroll_Zone z;
		z.zone = util::Rectangle<int>(util::Point<int>(48, 36), util::Size<int>(1, 2));
		z.area_name = "castle_scroll_dealer";
		z.topleft_dest = util::Point<int>(0, 15);
		z.direction = wedge::DIR_E;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Castle_Back()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {
			sign = new wedge::Map_Entity("sign");
			sign->start(area);
			sign->set_position(util::Point<int>(22, 2));
			area->add_entity(sign);
			
			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_SECOND_CHANCE, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(28, 21));
			area->add_entity(chest);
		
			util::Point<int> mm1_pos(23, 9);
			mm1 = new wedge::NPC("mm1", "Mighty Man", "mighty_man_blond", "mm9");
			mm1->set_wanders(false, false, true, true, mm1_pos, 2);
			mm1->start(area);
			mm1->set_position(mm1_pos);
			mm1->set_direction(wedge::DIR_N, true, false);
			area->add_entity(mm1);
		
			util::Point<int> mm2_pos(25, 7);
			mm2 = new wedge::NPC("mm2", "Mighty Man", "mighty_man_short", "mm10");
			mm2->start(area);
			mm2->set_position(mm2_pos);
			mm2->set_direction(wedge::DIR_N, true, false);
			area->add_entity(mm2);
		}
		else {
			sign = area->find_entity("sign");
			mm1 = static_cast<wedge::NPC *>(area->find_entity("mm1"));
			mm2 = static_cast<wedge::NPC *>(area->find_entity("mm2"));
		}

		return true;
	}

	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activated == sign) {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1556)/* Originally: "DANGER AHEAD!"... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
			return true;
		}
		return false;
	}

	bool try_tile(wedge::Map_Entity *entity, util::Point<int> tile_pos)
	{
		if (entity == AREA->get_player(ENY) && INSTANCE->is_milestone_complete(MS_TALKED_TO_QUEEN) == false) {
			if (tile_pos.y <= 8) {
				mm1->face(entity, mm1->is_moving());
				mm2->face(entity, mm2->is_moving());
				entity->get_input_step()->end_movement();
				GLOBALS->do_dialogue(GLOBALS->game_t->translate(279)/* Originally: Mighty Man */ + TAG_END, GLOBALS->game_t->translate(1558)/* Originally: HALT! Not without the Queen's permission! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
				return true;
			}
		}

		return false;
	}

private:
	wedge::Map_Entity *sign;
	wedge::NPC *mm1;
	wedge::NPC *mm2;
};

class Area_Hooks_Castle_Scroll_Dealer : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_Scroll_Dealer(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Scroll_Zone z;
		z.zone = util::Rectangle<int>(util::Point<int>(0, 15), util::Size<int>(1, 2));
		z.area_name = "castle_back";
		z.topleft_dest = util::Point<int>(48, 36);
		z.direction = wedge::DIR_W;
		scroll_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Castle_Scroll_Dealer()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {	
			scroll_dealer = new wedge::Map_Entity("Scroll Dealer");
			scroll_dealer->start(area);
			scroll_dealer->set_position(util::Point<int>(9, 13));
			scroll_dealer->set_sprite(new gfx::Sprite("scroll_dealer"));
			area->add_entity(scroll_dealer);
		}
		else {
			scroll_dealer = area->find_entity("Scroll Dealer");
		}

		return true;
	}

	
	bool activate(wedge::Map_Entity *activator, wedge::Map_Entity *activated)
	{
		if (activated == scroll_dealer) {
			bool have = INSTANCE->is_milestone_complete(MS_GOT_HEAL_OMEGA);
			INSTANCE->set_milestone_complete(MS_GOT_HEAL_OMEGA, true);
			
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))

			if (have) {
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1560)/* Originally: Haven't I given you enough?! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
			}
			else {
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */ + TAG_END, GLOBALS->game_t->translate(1562)/* Originally: You know, I could make a killing off this scroll...^But the world needs you! Here, it's on me! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
				ADD_STEP(new wedge::Give_Object_Step(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HEAL_OMEGA_SCROLL, 1), wedge::DIALOGUE_AUTO, new_task))
				ADD_STEP(new Scroll_Help_Step(GLOBALS->game_t->translate(1111)/* Originally: Scroll Dealer */, new_task));
			}
			
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			return true;
		}

		return false;
	}

private:
	wedge::Map_Entity *scroll_dealer;
};

class Area_Hooks_Castle_Shop : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_Shop(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(4, 11), util::Size<int>(1, 1));
		fz1.area_name = "castle_back";
		fz1.player_positions.push_back(util::Point<int>(9, 14));
		fz1.player_positions.push_back(util::Point<int>(9, 14));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Castle_Shop()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {
			armour_dealer = new wedge::Map_Entity("armour_dealer");
			armour_dealer->start(area);
			armour_dealer->set_position(util::Point<int>(2, 4));
			gfx::Sprite *armour_dealer_sprite = new gfx::Sprite("mighty_man");
			armour_dealer->set_sprite(armour_dealer_sprite);
			armour_dealer->set_direction(wedge::DIR_S, true, false);
			area->add_entity(armour_dealer);
			
			weapons_dealer = new wedge::Map_Entity("weapons_dealer");
			weapons_dealer->start(area);
			weapons_dealer->set_position(util::Point<int>(4, 4));
			gfx::Sprite *weapons_dealer_sprite = new gfx::Sprite("mighty_man_eyepatch");
			weapons_dealer->set_sprite(weapons_dealer_sprite);
			weapons_dealer->set_direction(wedge::DIR_S, true, false);
			area->add_entity(weapons_dealer);

			items_dealer = new wedge::Map_Entity("items_dealer");
			items_dealer->start(area);
			items_dealer->set_position(util::Point<int>(6, 4));
			gfx::Sprite *items_dealer_sprite = new gfx::Sprite("mighty_man");
			items_dealer->set_sprite(items_dealer_sprite);
			items_dealer->set_direction(wedge::DIR_S, true, false);
			area->add_entity(items_dealer);
		}
		else {
			armour_dealer = area->find_entity("armour_dealer");
			weapons_dealer = area->find_entity("weapons_dealer");
			items_dealer = area->find_entity("items_dealer");
		}

		return true;
	}
	
	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> activator_pos = activator->get_position();
		util::Point<int> armour_dealer_pos = armour_dealer->get_position();
		util::Point<int> weapons_dealer_pos = weapons_dealer->get_position();
		util::Point<int> items_dealer_pos = items_dealer->get_position();
		if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && (activator_pos-armour_dealer_pos) == util::Point<int>(0, 2)) {
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(279)/* Originally: Mighty Man */ + TAG_END, GLOBALS->game_t->translate(1565)/* Originally: You'll need better than that rusty armour you came here with! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_ARMOUR, ARMOUR_ONYX_ARMOUR, 1000));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_ARMOUR, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}
		else if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && (activator_pos-weapons_dealer_pos) == util::Point<int>(0, 2)) {
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(279)/* Originally: Mighty Man */ + TAG_END, GLOBALS->game_t->translate(1567)/* Originally: We make the finest weapons found anywhere. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_ONYX_BLADE, 2000));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_WEAPON, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}
		else if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && (activator_pos-items_dealer_pos) == util::Point<int>(0, 2)) {
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(279)/* Originally: Mighty Man */ + TAG_END, GLOBALS->game_t->translate(1569)/* Originally: We sell only top quality goods here. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			std::vector<wedge::Object> items;
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_CURE, 75));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION_PLUS, 75));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION_OMEGA, 100));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_HOLY_WATER, 200));
			items.push_back(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 250));
			ADD_STEP(new wedge::Shop_Step(wedge::OBJECT_ITEM, items, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}

		return false;
	}

private:
	wedge::Map_Entity *armour_dealer;
	wedge::Map_Entity *weapons_dealer;
	wedge::Map_Entity *items_dealer;
};

class Area_Hooks_Castle_Quarters : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_Quarters(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(4, 11), util::Size<int>(1, 1));
		fz1.area_name = "castle_back";
		fz1.player_positions.push_back(util::Point<int>(39, 14));
		fz1.player_positions.push_back(util::Point<int>(39, 14));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(6, 3), util::Size<int>(1, 1));
		fz2.area_name = "castle_quarters_below";
		fz2.player_positions.push_back(util::Point<int>(6, 1));
		fz2.player_positions.push_back(util::Point<int>(6, 1));
		fz2.directions.push_back(wedge::DIR_S);
		fz2.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_Castle_Quarters()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {
			wedge::Chest *chest = new wedge::Chest("chest", "", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION_OMEGA, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(7, 3));
			area->add_entity(chest);
		}

		return true;
	}

	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> pos = activator->get_position();
		wedge::Direction dir = activator->get_direction();
		if (activator == AREA->get_player(ENY) && (
			((pos == util::Point<int>(2, 5) || pos == util::Point<int>(4, 5)) && dir == wedge::DIR_N) ||
			((pos == util::Point<int>(3, 3) || pos == util::Point<int>(3, 4) || pos == util::Point<int>(5, 3) || pos == util::Point<int>(5, 4)) && dir == wedge::DIR_W) ||
			((pos == util::Point<int>(3, 3) || pos == util::Point<int>(3, 4) || pos == util::Point<int>(1, 4)) && dir == wedge::DIR_E)
			)
		) {
			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new Inn_Step(wedge::DIALOGUE_MESSAGE, "", GLOBALS->game_t->translate(1262)/* Originally: Sleep here? */, "", "", "", 0, util::Point<int>(3, 3), new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			return true;
		}

		return false;
	}
};

class Area_Hooks_Castle_Quarters_Below : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_Quarters_Below(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(6, 1), util::Size<int>(1, 1));
		fz1.area_name = "castle_quarters";
		fz1.player_positions.push_back(util::Point<int>(6, 3));
		fz1.player_positions.push_back(util::Point<int>(6, 3));
		fz1.directions.push_back(wedge::DIR_W);
		fz1.directions.push_back(wedge::DIR_W);
		fade_zones.push_back(fz1);
	}
	
	virtual ~Area_Hooks_Castle_Quarters_Below()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {
			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_JADE_SWORD, 2));
			chest->start(area);
			chest->set_position(util::Point<int>(4, 24));
			area->add_entity(chest);
			
			util::Point<int> mm1_pos(2, 3);
			wedge::NPC *mm1 = new wedge::NPC("mm1", "Mighty Man", "mighty_man", "mm8");
			mm1->start(area);
			mm1->set_extra_offset(util::Point<int>(0, 6));
			mm1->set_position(mm1_pos);
			mm1->get_sprite()->set_animation("sleep");
			area->add_entity(mm1);
		}

		return true;
	}
};

class Area_Hooks_Castle_Throne : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Castle_Throne(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(1, 10), util::Size<int>(1, 1));
		fz1.area_name = "castle_entrance";
		fz1.player_positions.push_back(util::Point<int>(8, 2));
		fz1.player_positions.push_back(util::Point<int>(8, 2));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);
		
		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(9, 10), util::Size<int>(1, 1));
		fz2.area_name = "castle_entrance";
		fz2.player_positions.push_back(util::Point<int>(16, 2));
		fz2.player_positions.push_back(util::Point<int>(16, 2));
		fz2.directions.push_back(wedge::DIR_S);
		fz2.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_Castle_Throne()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/castle.mml");
		
		if (loaded == false) {
			util::Point<int> queen_pos(5, 4);
			wedge::Map_Entity *queen = new wedge::Map_Entity("queen");
			queen->start(area);
			queen->set_position(queen_pos);
			queen->set_sprite(new gfx::Sprite("queen"));
			queen->set_layer(3);
			queen->set_solid(false);
			queen->set_direction(wedge::DIR_S, true, false);
			area->add_entity(queen);
			
			util::Point<int> mm11_pos(7, 6);
			wedge::NPC *mm11 = new wedge::NPC("mm11", "Mighty Man", "mighty_man_short", "mm11");
			mm11->start(area);
			mm11->set_position(mm11_pos);
			mm11->set_direction(wedge::DIR_S, true, false);
			area->add_entity(mm11);
		}

		return true;
	}
	
	void started()
	{
		if (INSTANCE->is_milestone_complete(MS_TALKED_TO_QUEEN) == false) {
			wedge::Map_Entity *eny = AREA->get_player(ENY);
			wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);
			INSTANCE->set_milestone_complete(MS_TALKED_TO_QUEEN, true);
			INSTANCE->party_following_player = false;
			wedge::pause_presses(true);
			gfx::Sprite *sprite = area->find_entity("queen")->get_sprite();

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(eny, util::Point<int>(4, 6), new_task))
			ADD_STEP(new wedge::Set_Direction_Step(eny, wedge::DIR_N, true, false, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(eny);
			entities.push_back(tiggy);
			std::vector< util::Point<int> > positions;
			positions.push_back(util::Point<int>(4, 6));
			positions.push_back(util::Point<int>(6, 6));
			ADD_STEP(new wedge::Check_Positions_Step(entities, positions, true, new_task))
			ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->jump, false, false, new_task))
			ADD_STEP(new wedge::Play_Animation_Step(sprite, "jump", new_task));
			ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->jump, false, false, new_task))
			ADD_STEP(new wedge::Play_Animation_Step(sprite, "jump", new_task));
			ADD_STEP(new wedge::Play_Animation_Step(sprite, "stand_s", new_task));
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1570)/* Originally: Queen */ + TAG_END, GLOBALS->game_t->translate(1571)/* Originally: Visitors! We rarely have visitors!^What brings you through the perils of the desert to my castle?^Also... why are you barefoot? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1573)/* Originally: We were picking fiddleheads.^And we're here to get rid of the monsters. Do you know why they're spawning? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1570)/* Originally: Queen */ + TAG_END, GLOBALS->game_t->translate(1575)/* Originally: Ah... I wish I did. My Mighty Men have been fighting them for days now. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1577)/* Originally: Our friend said a great sadness was spawning them... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1570)/* Originally: Queen */ + TAG_END, GLOBALS->game_t->translate(1579)/* Originally: There was a boy who used to hang around the castle.^Saddest fellow anyone's ever seen. He used to steal bread, but he was so sad we couldn't bear punishing him.^Last anyone saw of him, he went up into mountains to the North. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1581)/* Originally: What do you know about this boy? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1570)/* Originally: Queen */ + TAG_END, GLOBALS->game_t->translate(1583)/* Originally: Not much. He came around a few months ago, sad as can be... nobody had seen him before. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1585)/* Originally: Thanks for your help. We'll head North to try and track him down. */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1570)/* Originally: Queen */ + TAG_END, GLOBALS->game_t->translate(1587)/* Originally: Good luck, and feel free to get some rest in the castle before you leave.^The castle merchants may have some better equipment for you also. You're going to need it! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1589)/* Originally: We've made it this far! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::A_Star_Step(tiggy, util::Point<int>(5, 6), new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(set_following, (void *)1, new_task))
			ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
			ADD_TASK(new_task)

			ANOTHER_TASK
			ADD_STEP(new wedge::Delay_Step(500, new_task))
			ADD_STEP(new wedge::A_Star_Step(tiggy, util::Point<int>(6, 6), new_task))
			ADD_STEP(new wedge::Set_Direction_Step(tiggy, wedge::DIR_N, true, false, new_task))
			ADD_TASK(new_task)

			FINISH_SYSTEM(AREA)
		}

		Area_Hooks_Monster_RPG_3::started();
	}
	
	bool activate_with(wedge::Map_Entity *activator)
	{
		util::Point<int> activator_pos = activator->get_position();
		util::Point<int> queen_pos = area->find_entity("queen")->get_position();
		if (activator == AREA->get_player(ENY) && activator->get_direction() == wedge::DIR_N && (activator_pos-queen_pos) == util::Point<int>(0, 2)) {
			GLOBALS->do_dialogue(GLOBALS->game_t->translate(1570)/* Originally: Queen */ + TAG_END, GLOBALS->game_t->translate(1591)/* Originally: Good luck in the mountains! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, NULL);
			return true;
		}

		return false;
	}
};

class Area_Hooks_Mountains1 : public Area_Hooks_Monster_RPG_3
{
	struct Cloud {
		util::Point<float> pos;
		util::Point<float> radius;
		int h;
		float speed;
		int image;
	};

	static const int NUM_CLOUDS = 125; // first 1/5th are white clouds (with shadows) in back, next 4/5ths  are shadows on tiles
	static const int NUM_CLOUD_IMAGES = 10; // there are really double, second half are shadows (1.5 times larger)

	static const int FADE_TIME = 5000;

public:
	Area_Hooks_Mountains1(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area),
		fading_to_credits(false),
		credits_rolling(false),
		credits_y(0.0f),
		fading_out(false)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(16, 53), util::Size<int>(16, 1));
		fz1.area_name = "castle_back";
		fz1.player_positions.push_back(util::Point<int>(24, 0));
		fz1.player_positions.push_back(util::Point<int>(25, 0));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(28, 12), util::Size<int>(1, 1));
		fz2.area_name = "mountains2";
		fz2.player_positions.push_back(util::Point<int>(39, 32));
		fz2.player_positions.push_back(util::Point<int>(39, 32));
		fz2.directions.push_back(wedge::DIR_N);
		fz2.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz2);

		Fade_Zone fz3;
		fz3.zone = util::Rectangle<int>(util::Point<int>(17, 16), util::Size<int>(1, 1));
		fz3.area_name = "mountains2";
		fz3.player_positions.push_back(util::Point<int>(18, 49));
		fz3.player_positions.push_back(util::Point<int>(18, 49));
		fz3.directions.push_back(wedge::DIR_N);
		fz3.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(fz3);

		bg = new gfx::Image("misc/mountains_bg.tga");
		nooskewl_logo = new gfx::Image("misc/nooskewl_logo.tga");
		credits_gradient = new gfx::Image("misc/credits_gradient.tga");
		credits_gradient_top_bottom = new gfx::Image("misc/credits_gradient_top_bottom.tga");
		
		for (int i = 0; i < NUM_CLOUD_IMAGES; i++) {
			int y = util::rand(8, 16);
			int x = util::rand(y*1.5f, y*2.0f);
			cloud_image_sizes.push_back(util::Size<int>(x, y));
		}

		create_cloud_images();

		for (int i = 0; i < NUM_CLOUDS; i++) {
			Cloud c;
			gen_cloud(i, c);
			clouds.push_back(c);
		}
	}
	
	virtual ~Area_Hooks_Mountains1()
	{
		delete bg;
		delete nooskewl_logo;
		delete credits_gradient;
		delete credits_gradient_top_bottom;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/mountains.mml");
		
		if (GLOBALS->retried_boss) {
			retrying_boss = true;
		}
		else {
			retrying_boss = false;
		}
		
		if (loaded == false) {
			wedge::Chest *chest1 = new wedge::Chest("chest1", "chest", OBJECT->make_object(wedge::OBJECT_ARMOUR, ARMOUR_JADE_ARMOUR, 1));
			chest1->start(area);
			chest1->set_position(util::Point<int>(37, 27));
			area->add_entity(chest1);
			
			wedge::Chest *chest2 = new wedge::Chest("chest2", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_POTION_OMEGA, 5));
			chest2->start(area);
			chest2->set_position(util::Point<int>(9, 22));
			area->add_entity(chest2);
			
			wedge::Chest *chest3 = new wedge::Chest("chest3", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_ELIXIR, 3));
			chest3->start(area);
			chest3->set_position(util::Point<int>(8, 14));
			area->add_entity(chest3);
		
			add_gayan();
		}
		else {
			gayan = area->find_entity("gayan");
			if (gayan == NULL) {
				add_gayan();
			}
		}

		int line_h = (shim::font->get_height() + 2);

		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1685)/* Originally: ^GFX, Sounds */);
		credits.push_back(GLOBALS->game_t->translate(1686)/* Originally: ^Story + Code */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1687)/* Originally: Trent Gamblin */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1688)/* Originally: ^Quality Assurance */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1689)/* Originally: Eric Johnson */);
		credits.push_back(GLOBALS->game_t->translate(1690)/* Originally: Kristian Johnson */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1691)/* Originally: ^Original Palette */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1692)/* Originally: ENDESGA */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1693)/* Originally: ^Fonts */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1694)/* Originally: Craig Kroeger */);
		credits.push_back(GLOBALS->game_t->translate(1755)/* Originally: Jonas Hecksher */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1695)/* Originally: ^Battle Music */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1696)/* Originally: Jacob Dawid */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1697)/* Originally: ^Extra Music */);
		credits.push_back(GLOBALS->game_t->translate(1738)/* Originally: ^Arrangement */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1698)/* Originally: Rupert Cole */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1756)/* Originally: ^Tradução para Português */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1757)/* Originally: Adam Wallisson */);
		credits.push_back(GLOBALS->game_t->translate(1758)/* Originally: Stéfany Wine */);
		credits.push_back(GLOBALS->game_t->translate(1759)/* Originally: Stella Mares */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1724)/* Originally: ^French Translation */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1725)/* Originally: Words of Magic */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1745)/* Originally: ^Spanish Translation */);
		credits.push_back("")END;
		credits.push_back(GLOBALS->game_t->translate(1748)/* Originally: Ramón Méndez */);
		credits.push_back(GLOBALS->game_t->translate(1753)/* Originally: Alba Calvo */);
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		nooskewl_logo_pos = (int)credits.size() * line_h;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("")END;
		credits.push_back("^" + GLOBALS->game_t->translate(1642));

		credits_max_y = (int)credits.size() * line_h;

		return true;
	}

	void started()
	{
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		if (eny->get_position() == util::Point<int>(17, 16)) {
			gfx::Tilemap *tilemap = area->get_tilemap();
			tilemap->set_solid(-1, util::Point<int>(26, 7), util::Size<int>(1, 9), true);
			tilemap->set_solid(-1, util::Point<int>(25, 7), util::Size<int>(1, 9), false);
			
			tilemap->set_solid(-1, util::Point<int>(27, 16), util::Size<int>(1, 1), true);
			tilemap->set_solid(-1, util::Point<int>(28, 17), util::Size<int>(1, 1), true);
			tilemap->set_solid(-1, util::Point<int>(29, 18), util::Size<int>(1, 1), true);

			tilemap->set_solid(-1, util::Point<int>(26, 16), util::Size<int>(1, 1), false);
			tilemap->set_solid(-1, util::Point<int>(27, 17), util::Size<int>(1, 1), false);
			tilemap->set_solid(-1, util::Point<int>(28, 18), util::Size<int>(1, 1), false);

			tilemap->set_solid(-1, util::Point<int>(30, 19), util::Size<int>(1, 4), true);
			tilemap->set_solid(-1, util::Point<int>(29, 19), util::Size<int>(1, 4), false);

			eny->set_layer(2);
			AREA->get_player(TIGGY)->set_layer(2);

			battles = false;
		}
		else {
			battles = true;
		}

		// FIXME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//start_credits();

		Area_Hooks_Monster_RPG_3::started();
	}

	void pre_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 0 && credits_rolling == false) {
			float px, py;
			get_parallax(px, py);

			float x = -(bg->size.w - shim::screen_size.w) * px;
			float y = -(bg->size.h - shim::screen_size.h/2.0f) * py;
			
			bg->draw(util::Point<float>(x, y));

			SDL_Colour shadow = shim::black;
			shadow.r *= 0.25f;
			shadow.g *= 0.25f;
			shadow.b *= 0.25f;
			shadow.a *= 0.25f;

			// draw shadows first
			for (size_t i = 0; i < clouds.size()/5; i++) {
				Cloud &c = clouds[i];
				float xx = ((c.pos.x / (bg->size.w + c.radius.x*3)) * (bg->size.w + c.radius.x*3)) + x;
				float yy = ((c.pos.y / (bg->size.h + c.radius.y*3)) * (bg->size.h + c.radius.y*3)) + y + c.h;
				if (xx < -c.radius.x*1.5f || xx > shim::screen_size.w+c.radius.x*1.5f || yy < -c.radius.y*1.5f || yy > shim::screen_size.h+c.radius.y*1.5f) {
					continue;
				}
				gfx::Image *image = cloud_images[c.image+NUM_CLOUD_IMAGES];
				image->draw_tinted(shadow, util::Point<float>(xx-c.radius.x*1.5f, yy-c.radius.y*1.5f));
			}
			
			for (size_t i = 0; i < clouds.size()/5; i++) {
				Cloud &c = clouds[i];
				float xx = ((c.pos.x / (bg->size.w + c.radius.x*3)) * (bg->size.w + c.radius.x*3)) + x;
				float yy = ((c.pos.y / (bg->size.h + c.radius.y*3)) * (bg->size.h + c.radius.y*3)) + y;
				if (xx < -c.radius.x || xx > shim::screen_size.w+c.radius.x || yy < -c.radius.y || yy > shim::screen_size.h+c.radius.y) {
					continue;
				}
				gfx::Image *image = cloud_images[c.image];
				image->draw(util::Point<float>(xx-c.radius.x, yy-c.radius.y));
			}
			
			bak_shader = shim::current_shader;
			shim::current_shader = M3_GLOBALS->alpha_test_shader;
			shim::current_shader->use();
			gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
			gfx::update_projection();

			// Get depth buffer ready for second cloud pass (only draw over tiles)
			gfx::enable_depth_write(true);
			gfx::enable_depth_test(true);
			gfx::clear_depth_buffer(1.0f);
			gfx::set_depth_mode(gfx::COMPARE_ALWAYS);
		}
	}

	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			if (credits_rolling) {
				gfx::clear(shim::black);

				credits_gradient->start_batch();
				int grad_y = 0;
				do {
					credits_gradient->draw(util::Point<int>(0, grad_y));
					credits_gradient->draw(util::Point<int>(shim::screen_size.w-credits_gradient->size.w, grad_y), gfx::Image_Base::FLIP_H);
					grad_y += credits_gradient->size.h;
				} while (grad_y < shim::screen_size.h);
				credits_gradient->end_batch();

				credits_gradient_top_bottom->start_batch();
				int grad_x = 0;
				do {
					credits_gradient_top_bottom->draw(util::Point<int>(grad_x, 0));
					credits_gradient_top_bottom->draw(util::Point<int>(grad_x, shim::screen_size.h-credits_gradient_top_bottom->size.h), gfx::Image_Base::FLIP_V);
					grad_x += credits_gradient_top_bottom->size.w;
				} while (grad_x < shim::screen_size.w);
				credits_gradient_top_bottom->end_batch();

				for (size_t i = 0; i < credits.size(); i++) {
					std::string s = credits[i];
					gfx::Font *font;
					SDL_Colour colour;
					if (s[0] == '^') {
						font = GLOBALS->bold_font;
						s = s.substr(1);
						if (font->get_text_width(s) >= shim::screen_size.w) {
							font = shim::font;
						}
						colour = shim::white;
					}
					else {
						font = shim::font;
						colour = shim::palette[21];
					}

					int line_h = font->get_height() + 2;

					float y = shim::screen_size.h + credits_y + i * line_h;

					if (y < -font->get_height() - 5 || y >= shim::screen_size.h) {
						continue;
					}

					float alpha;

					if (y+font->get_height() <= credits_gradient->size.w) {
						alpha = (y+font->get_height()) / (float)credits_gradient->size.w;
					}
					else if (y >= shim::screen_size.h-credits_gradient->size.w) {
						alpha = (credits_gradient->size.w-(y-(shim::screen_size.h-credits_gradient->size.w)))/(float)credits_gradient->size.w;
					}
					else {
						alpha = 1.0f;
					}

					colour.r *= alpha;
					colour.g *= alpha;
					colour.b *= alpha;
					colour.a *= alpha;
					
					font->draw(colour, s, util::Point<float>(shim::screen_size.w/2.0f, y), false, true);
				}

				nooskewl_logo->draw(util::Point<float>(shim::screen_size.w/2.0f-nooskewl_logo->size.w/2.0f, shim::screen_size.h+credits_y+nooskewl_logo_pos));

				gfx::Font::end_batches();

				if (fading_out) {
					Uint32 now = GET_TICKS();
					Uint32 elapsed = now - fade_out_start;
					if (elapsed < 5000) {
						elapsed = 0;
					}
					else {
						elapsed -= 5000;
					}
					float alpha = elapsed / (float)FADE_TIME;
					if (alpha >= 1.0f) {
						audio::stop_music();
						util::achieve((void *)ACHIEVE_CREDITS);
						alpha = 1.0f;
						OMNIPRESENT->set_hide_red_triangle(false);
						OMNIPRESENT->set_quit_game(true);
					}
					SDL_Colour colour = shim::black;
					colour.r *= alpha;
					colour.g *= alpha;
					colour.b *= alpha;
					colour.a *= alpha;
					shim::music->set_master_volume(orig_music_volume * (1.0f - alpha));
					gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
				}
			}
			else {
				gfx::enable_depth_write(false);
				gfx::set_depth_mode(gfx::COMPARE_EQUAL);

				SDL_Colour shadow = shim::black;
				shadow.r *= 0.25f;
				shadow.g *= 0.25f;
				shadow.b *= 0.25f;
				shadow.a *= 0.25f;

				// Draw shadows over tiles

				shim::current_shader = bak_shader;
				shim::current_shader->use();
				gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
				gfx::update_projection();

				for (size_t i = clouds.size()/5; i < clouds.size(); i++) {
					Cloud &c = clouds[i];
					float xx = c.pos.x+map_offset.x;
					float yy = c.pos.y+map_offset.y;
					if (xx < -c.radius.x*1.5f || xx > shim::screen_size.w+c.radius.x*1.5f || yy < -c.radius.y*1.5f || yy > shim::screen_size.h+c.radius.y*1.5f) {
						continue;
					}
					gfx::Image *image = cloud_images[c.image+NUM_CLOUD_IMAGES];
					image->draw_tinted(shadow, util::Point<float>(xx-c.radius.x*1.5f, yy-c.radius.y*1.5f));
				}
				
				gfx::enable_depth_test(false);
				gfx::set_depth_mode(gfx::COMPARE_LESSEQUAL);

				if (fading_to_credits) {
					Uint32 now = GET_TICKS();
					Uint32 elapsed = now - fade_start;
					float alpha = elapsed / (float)FADE_TIME;
					if (alpha >= 1.0f) {
						credits_rolling = true;
						audio::play_music("music/credits.mml");
						orig_music_volume = shim::music->get_master_volume();
						alpha = 1.0f;
					}
					SDL_Colour colour = shim::black;
					colour.r *= alpha;
					colour.g *= alpha;
					colour.b *= alpha;
					colour.a *= alpha;
					gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
				}
			}
		}
	}

	std::vector<int> get_pre_draw_layers()
	{
		std::vector<int> v = wedge::Area_Hooks::get_pre_draw_layers();
		v.push_back(0);
		return v;
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = wedge::Area_Hooks::get_pre_draw_layers();
		v.push_back(3);
		return v;
	}

	void run()
	{
		for (size_t i = 0; i < clouds.size(); i++) {
			Cloud &c = clouds[i];
			c.pos.x -= c.speed;
			if (c.pos.x < -c.radius.x*1.5f) {
				if (i < NUM_CLOUDS/5) {
					c.pos.x = bg->size.w + c.radius.x*1.5f;
				}
				else {
					c.pos.x = area->get_tilemap()->get_size().w*shim::tile_size + c.radius.x*1.5f;
				}
			}
		}

		if (credits_rolling) {
			if (fading_out == false) {
				credits_y -= 0.2f;
			}
			if (fading_out == false && -credits_y >= credits_max_y+((shim::screen_size.h-(shim::font->get_height() + 2)*5)/2.0f+4)) {
				fading_out = true;
				fade_out_start = GET_TICKS();
			}
		}
	}

	void gen_cloud(int i, Cloud &c)
	{
		c.image = util::rand(0, NUM_CLOUD_IMAGES-1);

		c.radius.x = cloud_images[c.image]->size.w / 2.0f;
		c.radius.y = cloud_images[c.image]->size.h / 2.0f;

		if (i < NUM_CLOUDS/5) {
			c.pos.x = (int)util::rand(0, bg->size.w+(int)(c.radius.x*3)) - c.radius.x*1.5f; // enought space for shadow which is 1.5 * size
			c.pos.y = util::rand(0, bg->size.h/3);
			c.h = bg->size.h/2;
		}
		else {
			util::Size<int> area_size = area->get_tilemap()->get_size() * shim::tile_size;
			c.pos.x = (int)util::rand(0, area_size.w+c.radius.x*3) - c.radius.x*1.5f;
			c.pos.y = util::rand(0, area_size.h*2/3);
			c.h = 0;
		}

		c.speed = util::rand(0, 1000) / 1000.0f * 0.1f + 0.1f;
	}

	void get_parallax(float &px, float &py)
	{
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		util::Point<float> player_pos = eny->get_position();
		player_pos += eny->get_offset();
		player_pos *= shim::tile_size;
		util::Size<float> map_size = area->get_tilemap()->get_size() * shim::tile_size;

		// don't scroll if < half screen where the area doesn't scroll
		player_pos.x -= shim::screen_size.w / 2.0f;
		player_pos.y -= shim::screen_size.h / 2.0f;

		// the extra - shim::screen_size is so it doesn't scroll of > the last half screen where the area doesn't scroll
		px = MAX(0.0f, MIN(1.0f, player_pos.x / (map_size.w-shim::tile_size-shim::screen_size.w)));
		py = MAX(0.0f, MIN(1.0f, (float)player_pos.y / (23*shim::tile_size+shim::screen_size.h/2.0f))); // bg is 23 tiles high (trees the rest of the way
	}

	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		for (int i = 0; i < NUM_CLOUD_IMAGES*2; i++) {
			delete cloud_images[i];
			cloud_images[i] = NULL;
		}
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		create_cloud_images();
	}

	void create_cloud_images()
	{
		for (int i = 0; i < NUM_CLOUD_IMAGES; i++) {
			cloud_images[i] = new gfx::Image(cloud_image_sizes[i]*2.0f);
			draw_cloud(cloud_images[i]);
		}
		
		for (int i = 0; i < NUM_CLOUD_IMAGES; i++) {
			int j = i + NUM_CLOUD_IMAGES;
			cloud_images[j] = new gfx::Image(cloud_image_sizes[i/*yes, i*/]*3.0f);
			draw_cloud(cloud_images[j]);
		}
	}

	void draw_cloud(gfx::Image *image)
	{
		gfx::set_target_image(image);
		gfx::clear(shim::transparent);
		gfx::draw_filled_ellipse(shim::white, util::Point<float>(image->size.w/2.0f, image->size.h/2.0f), image->size.w/2.0f, image->size.h/2.0f);
		gfx::set_target_backbuffer();
	}

	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}
		
		gfx::Tilemap *tilemap = area->get_tilemap();
		util::Point<int> pos = entity->get_position();
			
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);

		util::Point<int> eny_dest = util::Point<int>(16, 10);
		util::Point<int> tiggy_dest = util::Point<int>(17, 10);
		
		if (entity == eny && (pos == util::Point<int>(20, 15) || pos == util::Point<int>(21, 15))) {
			tilemap->set_solid(-1, util::Point<int>(22, 8), util::Size<int>(1, 5), true);
			tilemap->set_solid(-1, util::Point<int>(17, 7), util::Size<int>(5, 1), true);
			tilemap->set_solid(-1, util::Point<int>(16, 6), util::Size<int>(1, 1), true);

			tilemap->set_solid(-1, util::Point<int>(23, 8), util::Size<int>(1, 5), false);
			tilemap->set_solid(-1, util::Point<int>(21, 6), util::Size<int>(1, 1), false);
			tilemap->set_solid(-1, util::Point<int>(22, 7), util::Size<int>(1, 1), false);
			tilemap->set_solid(-1, util::Point<int>(18, 5), util::Size<int>(3, 1), false);

			entity->set_layer(2);
			AREA->get_player(TIGGY)->set_layer(2);
		}
		else if (entity == eny && (pos == util::Point<int>(20, 14) || pos == util::Point<int>(21, 14))) {
			// This fixes tile solids/layers but you can never get past the final battle anyway
			/*
			tilemap->set_solid(-1, util::Point<int>(22, 8), util::Size<int>(1, 5), false);
			tilemap->set_solid(-1, util::Point<int>(17, 7), util::Size<int>(5, 1), false);
			tilemap->set_solid(-1, util::Point<int>(16, 6), util::Size<int>(1, 1), false);

			tilemap->set_solid(-1, util::Point<int>(23, 8), util::Size<int>(1, 5), true);
			tilemap->set_solid(-1, util::Point<int>(21, 6), util::Size<int>(1, 1), true);
			tilemap->set_solid(-1, util::Point<int>(22, 7), util::Size<int>(1, 1), true);
			tilemap->set_solid(-1, util::Point<int>(18, 5), util::Size<int>(3, 1), true);
			
			entity->set_layer(3);
			AREA->get_player(TIGGY)->set_layer(3);
		}
		else if (pos == util::Point<int>(20, 14) || pos == util::Point<int>(21, 14)) {
			*/
			INSTANCE->set_milestone_complete(MS_GAYAN_STEP, true);

			INSTANCE->party_following_player = false;

			wedge::pause_presses(true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(entity, eny_dest, new_task))
			std::vector<wedge::Map_Entity *> entities;
			entities.push_back(entity);
			entities.push_back(tiggy);
			std::vector< util::Point<int> > positions;
			positions.push_back(eny_dest);
			positions.push_back(tiggy_dest);
			ADD_STEP(new wedge::Check_Positions_Step(entities, positions, true, new_task))
			ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->cry, false, false, new_task, 2.0f))
			ADD_STEP(new wedge::Play_Animation_Step(gayan->get_sprite(), "cry", new_task))
			ADD_STEP(new wedge::Play_Animation_Step(gayan->get_sprite(), "sit", new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1657)/* Originally: You there! Are you the one spawning these monsters? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->cry, false, false, new_task, 2.0f))
			ADD_STEP(new wedge::Play_Animation_Step(gayan->get_sprite(), "cry", new_task))
			ADD_STEP(new wedge::Play_Animation_Step(gayan->get_sprite(), "sit", new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1658)/* Originally: Waaa, please leave me here alone in my sorrows!^I don't need anyone trying to cheer me up! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1659)/* Originally: Listen, we can tell you're really sad, and we think that's causing monsters to appear. You have to stop! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->cry, false, false, new_task, 2.0f))
			ADD_STEP(new wedge::Play_Animation_Step(gayan->get_sprite(), "cry", new_task))
			ADD_STEP(new wedge::Play_Animation_Step(gayan->get_sprite(), "sit", new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1660)/* Originally: Waaa, I'll always be sad. After what I've done, I deserve nothing else! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1661)/* Originally: I'm sure you're very sorry for what you've done and you've learned your lesson... You don't have to go on like this! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->cry, false, false, new_task, 2.0f))
			ADD_STEP(new wedge::Play_Animation_Step(gayan->get_sprite(), "cry", new_task))
			ADD_STEP(new wedge::Play_Animation_Step(gayan->get_sprite(), "sit", new_task))
			ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1662)/* Originally: Waaa, oh yeah, try and stop me! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
			ADD_STEP(new Start_Battle_Step(new Battle_Gayan(), new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			return true;
		}
		else if (entity == tiggy && INSTANCE->is_milestone_complete(MS_GAYAN_STEP) && INSTANCE->is_milestone_complete(MS_GAYAN_STEP2) == false) {
			INSTANCE->set_milestone_complete(MS_GAYAN_STEP2, true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::Delay_Step(500, new_task))
			ADD_STEP(new wedge::A_Star_Step(tiggy, tiggy_dest, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)

			return true;
		}

		return false;
	}

	bool try_tile(wedge::Map_Entity *entity, util::Point<int> tile_pos)
	{
		if (GLOBALS->retried_boss) {
			return false;
		}

		if (INSTANCE->is_milestone_complete(MS_GAYAN_STEP)) {
			return false;
		}

		wedge::Map_Entity *eny = AREA->get_player(ENY);

		if (entity != eny) {
			return false;
		}

		if (tile_pos == util::Point<int>(20, 14) || tile_pos == util::Point<int>(21, 14)) {
			GLOBALS->boss_save = wedge::save();
			GLOBALS->boss_press = wedge::DIR_N;
		}

		return false;
	}

	void start_credits()
	{
		fading_to_credits = true;
		fade_start = GET_TICKS();
		OMNIPRESENT->set_hide_red_triangle(true);
		if (M3_GLOBALS->loaded == false) {
			util::achieve((void *)ACHIEVE_HARDCORE);
		}
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (battles == false || retrying_boss) {
			return NULL;
		}

		// Limit battles to mountains tiles (mountains and steps)

		util::Rectangle<int> r1(util::Point<int>(13, 32), util::Size<int>(6, 6));
		util::Rectangle<int> r2(util::Point<int>(19, 35), util::Size<int>(4, 1));

		util::Point<int> pos = AREA->get_player(ENY)->get_position();
		util::Point<int> tile_xy;
		bool solid;
		bool found = false;
		gfx::Tilemap *tilemap = AREA->get_current_area()->get_tilemap();
		int num_layers = tilemap->get_num_layers();

		for (int i = 0; i < num_layers; i++) {
			tilemap->get_tile(i, pos, tile_xy, solid);
			if (r1.contains(tile_xy) || r2.contains(tile_xy)) {
				found = true;
				break;
			}
		}

		if (found == false) {
			return NULL;
		}

		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(4);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 3) {
			return new Battle_3Rocky();
		}
		else if (type == 2) {
			return new Battle_2Wraith();
		}
		else if (type == 1) {
			return new Battle_1Reaper2Rocky();
		}
		else {
			return new Battle_1Reaper1Wraith();
		}
	}

	void add_gayan()
	{
		gayan = new wedge::Map_Entity("gayan");
		gayan->start(area);
		gayan->set_position(util::Point<int>(16, 8));
		gayan->set_sprite(new gfx::Sprite("gayan"));
		gayan->get_sprite()->set_animation("sit");
		area->add_entity(gayan);
	}

private:
	gfx::Image *bg;
	gfx::Image *cloud_images[NUM_CLOUD_IMAGES*2];
	std::vector<Cloud> clouds;
	std::vector< util::Size<int> > cloud_image_sizes;
	gfx::Shader *bak_shader;
	bool fading_to_credits;
	bool credits_rolling;
	float credits_y;
	Uint32 fade_start;
	std::vector<std::string> credits;
	int credits_max_y;
	gfx::Image *nooskewl_logo;
	gfx::Image *credits_gradient;
	gfx::Image *credits_gradient_top_bottom;
	int nooskewl_logo_pos;
	bool fading_out;
	Uint32 fade_out_start;
	bool battles;
	wedge::Map_Entity *gayan;
	float orig_music_volume;
	bool retrying_boss;
};

static void add_pendants(void *data)
{
	wedge::Area *area = AREA->get_current_area();

	wedge::Map_Entity *pendant1 = new wedge::Map_Entity("pendant1");
	pendant1->start(area);
	pendant1->set_position(util::Point<int>(16, 41));
	pendant1->set_sprite(new gfx::Sprite("pendant"));
	pendant1->set_layer(3);
	area->add_entity(pendant1);
	
	wedge::Map_Entity *pendant2 = new wedge::Map_Entity("pendant2");
	pendant2->start(area);
	pendant2->set_position(util::Point<int>(20, 41));
	pendant2->set_sprite(new gfx::Sprite("pendant"));
	pendant2->set_layer(3);
	area->add_entity(pendant2);
}

class Area_Hooks_Mountains2 : public Area_Hooks_Monster_RPG_3
{
public:
	Area_Hooks_Mountains2(wedge::Area *area) :
		Area_Hooks_Monster_RPG_3(area)
	{
		Fade_Zone fz1;
		fz1.zone = util::Rectangle<int>(util::Point<int>(39, 32), util::Size<int>(1, 1));
		fz1.area_name = "mountains1";
		fz1.player_positions.push_back(util::Point<int>(28, 12));
		fz1.player_positions.push_back(util::Point<int>(28, 12));
		fz1.directions.push_back(wedge::DIR_S);
		fz1.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz1);

		Fade_Zone fz2;
		fz2.zone = util::Rectangle<int>(util::Point<int>(18, 49), util::Size<int>(1, 1));
		fz2.area_name = "mountains1";
		fz2.player_positions.push_back(util::Point<int>(17, 16));
		fz2.player_positions.push_back(util::Point<int>(17, 16));
		fz2.directions.push_back(wedge::DIR_S);
		fz2.directions.push_back(wedge::DIR_S);
		fade_zones.push_back(fz2);
	}
	
	virtual ~Area_Hooks_Mountains2()
	{
		delete darkness_image1;
		delete darkness_image2;
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		audio::play_music("music/mountains.mml");
		
		darkness_image1 = gen_hh_darkness(1, 0.5f, 0.25f, shim::palette[24]);
		darkness_image2 = gen_hh_darkness(2, 0.5f, 0.0f, shim::palette[24]);
		darkness_offset1 = util::Point<float>(0, 0);
		darkness_offset2 = util::Point<float>(128, 64);

		if (loaded == false) {
			Revive_Entity *revive1 = new Revive_Entity();
			revive1->start(area);
			revive1->set_position(util::Point<int>(22, 41));
			area->add_entity(revive1);
			
			Revive_Entity *revive2 = new Revive_Entity();
			revive2->start(area);
			revive2->set_position(util::Point<int>(23, 41));
			area->add_entity(revive2);
		
			Revive_Entity *revive3 = new Revive_Entity();
			revive3->start(area);
			revive3->set_position(util::Point<int>(24, 41));
			area->add_entity(revive3);
			
			Revive_Entity *revive4 = new Revive_Entity();
			revive4->start(area);
			revive4->set_position(util::Point<int>(11, 41));
			area->add_entity(revive4);
			
			Revive_Entity *revive5 = new Revive_Entity();
			revive5->start(area);
			revive5->set_position(util::Point<int>(12, 41));
			area->add_entity(revive5);
			
			Revive_Entity *revive6 = new Revive_Entity();
			revive6->start(area);
			revive6->set_position(util::Point<int>(13, 41));
			area->add_entity(revive6);
			
			wedge::Chest *chest1 = new wedge::Chest("chest1", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_FLAME_SWORD, 1));
			chest1->start(area);
			chest1->set_position(util::Point<int>(16, 22));
			area->add_entity(chest1);
			
			wedge::Chest *chest2 = new wedge::Chest("chest2", "chest", OBJECT->make_object(wedge::OBJECT_ARMOUR, ARMOUR_JADE_ARMOUR, 1));
			chest2->start(area);
			chest2->set_position(util::Point<int>(52, 15));
			area->add_entity(chest2);
		}

		return true;
	}

	bool can_save()
	{
		util::Rectangle<int> r(util::Point<int>(10, 41), util::Size<int>(16, 9));
		if (r.contains(AREA->get_player(ENY)->get_position())) {
			return true;
		}
		return false;
	}
	
	void run()
	{
		darkness_offset1.x += 0.04f;
		darkness_offset1.y += 0.02f;
		darkness_offset2.x -= 0.04f;
		darkness_offset2.y -= 0.02f;
	}
	
	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			draw_darkness(map_offset);
		}
	}

	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v = Area_Hooks_Monster_RPG_3::get_post_draw_layers();
		v.push_back(3);
		return v;
	}
	
	void lost_device()
	{
		Area_Hooks_Monster_RPG_3::lost_device();
		delete darkness_image1;
		darkness_image1 = NULL;
		delete darkness_image2;
		darkness_image2 = NULL;
	}

	void found_device()
	{
		Area_Hooks_Monster_RPG_3::found_device();
		darkness_image1 = gen_hh_darkness(1, 0.5f, 0.25f, shim::palette[24]);
		darkness_image2 = gen_hh_darkness(2, 0.5f, 0.0f, shim::palette[24]);
	}
	
	void set_animated_tiles()
	{
		gfx::Tilemap *tilemap = area->get_tilemap();
		gfx::Tilemap::Animation_Data water_anim;
		water_anim.topleft = util::Point<int>(19, 36);
		water_anim.size = util::Size<int>(4, 2);
		water_anim.delay = 500;
		water_anim.frames.clear();
		water_anim.frames.push_back(util::Point<int>(19, 38));
		water_anim.frames.push_back(util::Point<int>(19, 40));
		water_anim.frames.push_back(util::Point<int>(19, 42));
		tilemap->add_animation_data(water_anim);
		gfx::Tilemap::Animation_Data savezone_anim;
		savezone_anim.topleft = util::Point<int>(9, 0);
		savezone_anim.size = util::Size<int>(2, 2);
		savezone_anim.delay = 128;
		savezone_anim.frames.push_back(util::Point<int>(9, 2));
		savezone_anim.frames.push_back(util::Point<int>(9, 4));
		savezone_anim.frames.push_back(util::Point<int>(9, 6));
		savezone_anim.frames.push_back(util::Point<int>(9, 8));
		savezone_anim.frames.push_back(util::Point<int>(9, 10));
		savezone_anim.frames.push_back(util::Point<int>(9, 12));
		savezone_anim.frames.push_back(util::Point<int>(9, 14));
		savezone_anim.frames.push_back(util::Point<int>(9, 16));
		savezone_anim.frames.push_back(util::Point<int>(9, 18));
		savezone_anim.frames.push_back(util::Point<int>(9, 20));
		savezone_anim.frames.push_back(util::Point<int>(9, 22));
		tilemap->add_animation_data(savezone_anim);
		gfx::Tilemap::Animation_Data water_anim2;
		water_anim2.topleft = util::Point<int>(25, 36);
		water_anim2.size = util::Size<int>(4, 2);
		water_anim2.delay = 500;
		water_anim2.frames.clear();
		water_anim2.frames.push_back(util::Point<int>(25, 38));
		water_anim2.frames.push_back(util::Point<int>(25, 40));
		water_anim2.frames.push_back(util::Point<int>(25, 42));
		tilemap->add_animation_data(water_anim2);
		gfx::Tilemap::Animation_Data savezone_anim2;
		savezone_anim2.topleft = util::Point<int>(29, 35);
		savezone_anim2.size = util::Size<int>(3, 2);
		savezone_anim2.delay = 128;
		savezone_anim2.frames.push_back(util::Point<int>(29, 37));
		savezone_anim2.frames.push_back(util::Point<int>(29, 39));
		savezone_anim2.frames.push_back(util::Point<int>(29, 41));
		savezone_anim2.frames.push_back(util::Point<int>(29, 43));
		savezone_anim2.frames.push_back(util::Point<int>(29, 45));
		savezone_anim2.frames.push_back(util::Point<int>(29, 47));
		savezone_anim2.frames.push_back(util::Point<int>(29, 49));
		savezone_anim2.frames.push_back(util::Point<int>(29, 51));
		savezone_anim2.frames.push_back(util::Point<int>(29, 53));
		savezone_anim2.frames.push_back(util::Point<int>(29, 55));
		savezone_anim2.frames.push_back(util::Point<int>(29, 57));
		tilemap->add_animation_data(savezone_anim2);
	}
	
	bool on_tile(wedge::Map_Entity *entity)
	{
		if (Area_Hooks_Monster_RPG_3::on_tile(entity)) {
			return true;
		}

		if (entity == AREA->get_player(ENY)) {
			util::Point<int> eny_pos = entity->get_position();
			if (eny_pos == util::Point<int>(18, 42) && INSTANCE->is_milestone_complete(MS_PENDANTS_SCENE) == false) {
				INSTANCE->set_milestone_complete(MS_PENDANTS_SCENE, true);

				INSTANCE->inventory.remove(INSTANCE->inventory.find(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_VAMPIRE1, 1)), 1);
				INSTANCE->inventory.remove(INSTANCE->inventory.find(OBJECT->make_object(wedge::OBJECT_SPECIAL, SPECIAL_VAMPIRE2, 1)), 1);

				wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);

				entity->set_solid(false);
				tiggy->set_solid(false);

				set_following((void *)0);

				util::Point<int> eny_dest(16, 43);
				util::Point<int> tiggy_dest(20, 43);
			
				M3_INSTANCE->add_vampire("vFire");

				NEW_SYSTEM_AND_TASK(AREA)
				ADD_STEP(new wedge::Pause_Presses_Step(true, false, new_task))
				ADD_STEP(new wedge::A_Star_Step(entity, eny_dest, new_task))
				ADD_STEP(new wedge::Set_Direction_Step(entity, wedge::DIR_N, true, false, new_task))
				std::vector<wedge::Map_Entity *> entities;
				entities.push_back(entity);
				entities.push_back(tiggy);
				std::vector< util::Point<int> > positions;
				positions.push_back(eny_dest);
				positions.push_back(tiggy_dest);
				ADD_STEP(new wedge::Check_Positions_Step(entities, positions, true, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1702)/* Originally: Do you see what I see? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1703)/* Originally: The holes in the wall... they match our Vampire Pendants! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1704)/* Originally: Maybe this is where the Mayor and Palla found them! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new wedge::Set_Direction_Step(entity, wedge::DIR_E, true, false, new_task))
				ADD_STEP(new wedge::Set_Direction_Step(tiggy, wedge::DIR_W, true, false, new_task))
				wedge::Delay_Step *ds = new wedge::Delay_Step(1000, new_task);
				ADD_STEP(ds)
				ADD_STEP(new wedge::A_Star_Step(entity, eny_dest+util::Point<int>(0, -1), new_task))
				ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->pendants, false, false, new_task))
				ADD_STEP(new wedge::Generic_Immediate_Callback_Step(add_pendants, NULL, new_task))
				ADD_STEP(new wedge::Delay_Step(2500, new_task))
				ADD_STEP(new wedge::Play_Sound_Step(GLOBALS->levelup, false, false, new_task))
				ADD_STEP(new Dialogue_Step("", GLOBALS->game_t->translate(1705)/* Originally: Learned vFire! */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step("", GLOBALS->game_t->translate(1706)/* Originally: vFire, also known as Double Vampire, is the most powerful Vampire skill of all! No one who has used it ever lived to tell the tale, because it drains 100% of the health and magic of both of those who hold the Pendants! */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1707)/* Originally: Nice! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
				ADD_STEP(new wedge::A_Star_Step(tiggy, eny_dest+util::Point<int>(1, -1), new_task))
				ADD_STEP(new wedge::Generic_Immediate_Callback_Step(set_following, (void *)1, new_task))
				ADD_STEP(new wedge::Set_Solid_Step(entity, true, new_task))
				ADD_STEP(new wedge::Set_Solid_Step(tiggy, true, new_task))
				ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
				ADD_TASK(new_task)

				ANOTHER_TASK
				ADD_STEP(new wedge::A_Star_Step(tiggy, tiggy_dest, new_task))
				ADD_STEP(new wedge::Set_Direction_Step(tiggy, wedge::DIR_N, true, false, new_task))
				wedge::Wait_Step *ws = new wedge::Wait_Step(new_task);
				ADD_STEP(ws)
				ds->add_monitor(ws);
				ADD_STEP(new wedge::A_Star_Step(tiggy, tiggy_dest+util::Point<int>(0, -1), new_task))
				ADD_TASK(new_task)

				FINISH_SYSTEM(AREA)

				return true;
			}
		}

		return false;
	}
	
	wedge::Battle_Game *get_random_battle()
	{
		// No battles in save zone
		if (can_save()) {
			return NULL;
		}

		if (rand_battle_table.size() == 0) {
			gen_rand_battle_table(3);
		}

		int type = rand_battle_table[rand_battle_table.size()-1];
		rand_battle_table.pop_back();

		if (type == 2) {
			return new Battle_3Shadow();
		}
		else if (type == 1) {
			return new Battle_2Wraith_Caves();
		}
		else {
			return new Battle_1Reaper1Wraith_Caves();
		}
	}
};

const float Monster_RPG_3_Area_Game::MIN_DUST_SPEED = 1.5f;
const float Monster_RPG_3_Area_Game::MAX_DUST_SPEED = 2.0f;

Monster_RPG_3_Area_Game::Monster_RPG_3_Area_Game() :
	use_camera(false),
	area_create_count(0)
{
	fadeout_colour = shim::black;

	for (size_t i = 0; i < NUM_DUST; i++) {
		Dust d;
		d.position = util::Point<float>(util::rand(0, shim::tile_size*20-1), util::rand(0, shim::tile_size*20-1)); // desert levels are 20x20
		d.cycles = util::rand(2, 3);
		d.phase = util::rand(0, 1000)/1000.0f * M_PI * 2.0f;
		d.speed = MIN_DUST_SPEED + (util::rand(0, 1000)/1000.0f * (MAX_DUST_SPEED-MIN_DUST_SPEED));
		dust.push_back(d);
	}

	dust_image = new gfx::Image("misc/dust.tga");

	prev_chests_opened = INSTANCE->chests_opened;
}

Monster_RPG_3_Area_Game::~Monster_RPG_3_Area_Game()
{
	delete dust_image;
}

wedge::Area_Hooks *Monster_RPG_3_Area_Game::get_area_hooks(std::string area_name, wedge::Area *area)
{
	wedge::Area_Hooks *hooks = NULL;

	if (area_name == "start") {
		hooks = new Area_Hooks_Start(area);
	}
	else if (area_name == "fiddler") {
		hooks = new Area_Hooks_Fiddler(area);
	}
	else if (area_name == "forest_save") {
		hooks = new Area_Hooks_Forest_Save(area);
	}
	else if (area_name == "forest1") {
		hooks = new Area_Hooks_Forest1(area);
	}
	else if (area_name == "forest2") {
		hooks = new Area_Hooks_Forest2(area);
	}
	else if (area_name == "riverside") {
		hooks = new Area_Hooks_Riverside(area);
	}
	else if (area_name == "riverside_inn") {
		hooks = new Area_Hooks_Riverside_Inn(area);
	}
	else if (area_name == "riverside_inn_upper") {
		hooks = new Area_Hooks_Riverside_Inn_Upper(area);
	}
	else if (area_name == "riverside_item_shop") {
		hooks = new Area_Hooks_Riverside_Item_Shop(area);
	}
	else if (area_name == "riverside_equipment_shop") {
		hooks = new Area_Hooks_Riverside_Equipment_Shop(area);
	}
	else if (area_name == "lumberjacks_dads") {
		hooks = new Area_Hooks_Lumberjacks_Dads(area);
	}
	else if (area_name == "coros") {
		hooks = new Area_Hooks_Coros(area);
	}
	else if (area_name == "moryts") {
		hooks = new Area_Hooks_Moryts(area);
	}
	else if (area_name == "captains1") {
		hooks = new Area_Hooks_Captains1(area);
	}
	else if (area_name == "captains2") {
		hooks = new Area_Hooks_Captains2(area);
	}
	else if (area_name == "womans") {
		hooks = new Area_Hooks_Womans(area);
	}
	else if (area_name == "palace1") {
		hooks = new Area_Hooks_Palace1(area);
	}
	else if (area_name == "palaceb1") {
		hooks = new Area_Hooks_PalaceB1(area);
	}
	else if (area_name == "palaceb2") {
		hooks = new Area_Hooks_PalaceB2(area);
	}
	else if (area_name == "palace2") {
		hooks = new Area_Hooks_Palace2(area);
	}
	else if (area_name == "hh") {
		hooks = new Area_Hooks_HH(area);
	}
	else if (area_name == "hh1") {
		hooks = new Area_Hooks_HH1(area);
	}
	else if (area_name == "hh2") {
		hooks = new Area_Hooks_HH2(area);
	}
	else if (area_name == "hh3") {
		hooks = new Area_Hooks_HH3(area);
	}
	else if (area_name == "hh_stairs") {
		hooks = new Area_Hooks_HH_Stairs(area);
	}
	else if (area_name == "hh_basement") {
		hooks = new Area_Hooks_HH_Basement(area);
	}
	else if (area_name == "hh_secret") {
		hooks = new Area_Hooks_HH_Secret(area);
	}
	else if (area_name == "hh_save") {
		hooks = new Area_Hooks_HH_Save(area);
	}
	else if (area_name == "beach_w") {
		hooks = new Area_Hooks_Beach_W(area);
	}
	else if (area_name == "below_deck") {
		hooks = new Area_Hooks_Below_Deck(area);
	}
	else if (area_name == "captains_quarters") {
		hooks = new Area_Hooks_Captains_Quarters(area);
	}
	else if (area_name == "at_sea") {
		hooks = new Area_Hooks_At_Sea(area);
	}
	else if (area_name == "boatin") {
		hooks = new Area_Hooks_Boatin(area);
	}
	else if (area_name == "beach_e") {
		hooks = new Area_Hooks_Beach_E(area);
	}
	else if (area_name == "desert1") {
		hooks = new Area_Hooks_Desert1(area);
	}
	else if (area_name == "desert2") {
		hooks = new Area_Hooks_Desert2(area);
	}
	else if (area_name == "desert3") {
		hooks = new Area_Hooks_Desert3(area);
	}
	else if (area_name == "desert4") {
		hooks = new Area_Hooks_Desert4(area);
	}
	else if (area_name == "desert5") {
		hooks = new Area_Hooks_Desert5(area);
	}
	else if (area_name == "desert6") {
		hooks = new Area_Hooks_Desert6(area);
	}
	else if (area_name == "desert7") {
		hooks = new Area_Hooks_Desert7(area);
	}
	else if (area_name == "desert8") {
		hooks = new Area_Hooks_Desert8(area);
	}
	else if (area_name == "castle_front") {
		hooks = new Area_Hooks_Castle_Front(area);
	}
	else if (area_name == "castle_entrance") {
		hooks = new Area_Hooks_Castle_Entrance(area);
	}
	else if (area_name == "castle_to_ramparts") {
		hooks = new Area_Hooks_Castle_To_Ramparts(area);
	}
	else if (area_name == "castle_back") {
		hooks = new Area_Hooks_Castle_Back(area);
	}
	else if (area_name == "castle_scroll_dealer") {
		hooks = new Area_Hooks_Castle_Scroll_Dealer(area);
	}
	else if (area_name == "castle_shop") {
		hooks = new Area_Hooks_Castle_Shop(area);
	}
	else if (area_name == "castle_quarters") {
		hooks = new Area_Hooks_Castle_Quarters(area);
	}
	else if (area_name == "castle_quarters_below") {
		hooks = new Area_Hooks_Castle_Quarters_Below(area);
	}
	else if (area_name == "castle_throne") {
		hooks = new Area_Hooks_Castle_Throne(area);
	}
	else if (area_name == "mountains1") {
		hooks = new Area_Hooks_Mountains1(area);
	}
	else if (area_name == "mountains2") {
		hooks = new Area_Hooks_Mountains2(area);
	}
	else {
		util::debugmsg("No area hooks for '%s'\n", area_name.c_str());
	}

	return hooks;
}

wedge::Game *Monster_RPG_3_Area_Game::create_menu()
{
	return new Menu_Game();
}

wedge::Game *Monster_RPG_3_Area_Game::create_shop(wedge::Object_Type type, std::vector<wedge::Object> items)
{
	return new Shop_Game(type, items);
}

wedge::Map_Entity *Monster_RPG_3_Area_Game::create_entity(std::string type, util::JSON::Node *json)
{
	util::JSON::Node *n = json->find("\"type\"");
	if (n) {
		if (n->value == "\"revive\"") {
			return new Revive_Entity(json);
		}
		else if (n->value == "\"sailor\"") {
			return new Sailor(json);
		}
		else if (n->value == "\"sailor_npc\"") {
			return new Sailor_NPC(json);
		}
		else if (n->value == "\"sailship\"") {
			return new Sail_Ship(json);
		}
	}
	return NULL; // not a custom entity
}

wedge::Map_Entity *Monster_RPG_3_Area_Game::create_player(std::string entity_name)
{
	return new Sailor(entity_name);
}

void Monster_RPG_3_Area_Game::draw()
{
	gfx::clear(shim::black);

	if (scrolling_in) {
		for (int i = 0; i < 2; i++) {
			wedge::Area::Layer_Spec spec;

			if (i == 0) {
				spec = wedge::Area::BELOW;
			}
			else {
				spec = wedge::Area::ABOVE;
			}

			util::Size<int> tilemap_size = current_area->get_tilemap()->get_size() * shim::tile_size;
			util::Point<float> maximum(shim::screen_size.w, shim::screen_size.h);
			maximum.x = MIN(maximum.x, tilemap_size.w);
			maximum.y = MIN(maximum.y, tilemap_size.h);
			util::Point<float> scrolled = scroll_offset * maximum;
			util::Point<int> player_pos = players[0]->get_position();
			util::Point<float> player_offset = util::Point<float>(0.0f, 0.0f);
			util::Size<int> player_size = players[0]->get_size();
			util::Point<float> sz(player_size.w / 2.0f, 1.0f - player_size.h / 2.0f);
			wedge::add_tiles(player_pos, player_offset, sz);
			//util::Point<float> curr_offset = current_area->get_centred_offset(players[0]->get_position(), util::Point<float>(0.0f, 0.0f), true);
			util::Point<float> curr_offset = current_area->get_centred_offset(player_pos, player_offset, true);
			curr_offset -= scrolled;
			current_area->draw(curr_offset, spec);
			util::Point<int> next_pos = next_area_positions[0];
			util::Point<float> next_o(0.0f, 0.0f);
			wedge::add_tiles(next_pos, next_o, sz);
			//util::Point<float> next_offset = next_area->get_centred_offset(next_area_positions[0], util::Point<float>(0.0f, 0.0f), true);
			util::Point<float> next_offset = next_area->get_centred_offset(next_pos, next_o, true);
			util::Point<float> no;
			if (scroll_increment.x < 0 || scroll_increment.y < 0) {
				util::Point<float> o = curr_offset;
				if (tilemap_size.w < shim::screen_size.w) {
					o.x -= (shim::screen_size.w-tilemap_size.w)/2.0f;
				}
				if (tilemap_size.h < shim::screen_size.h) {
					o.y -= (shim::screen_size.h-tilemap_size.h)/2.0f;
				}
				if (scroll_increment.x == 0.0f) {
					maximum.x = o.x;
				}
				else {
					maximum.y = o.y;
				}
				no = next_offset - (maximum - o);
				next_area->draw(next_offset - (maximum - o), spec);
			}
			else {
				if (scroll_increment.x == 0.0f) {
					maximum.x = scrolled.x;
				}
				else {
					maximum.y = scrolled.y;
				}
				no = next_offset + (maximum - scrolled);
				next_area->draw(next_offset + (maximum - scrolled), spec);
			}
			if (i == 0) {
				std::vector<wedge::Map_Entity *> entities = players;
				std::sort(entities.begin(), entities.end(), wedge::entity_y_compare);
				for (size_t i = 0; i < entities.size(); i++) {
					wedge::Map_Entity *p = entities[i];
					p->draw(curr_offset + (scroll_offset * shim::tile_size));
				}
			}
			if (i == 1) {
				dust_image->start_batch();
				draw_dust(curr_offset);
				draw_dust(no);
				dust_image->end_batch();
			}
		}
	}
	else {
		if (paused == false) {
			if (use_camera) {
				current_area->draw(camera);
			}
			else {
				current_area->draw();
			}

			wedge::Map_Entity *player = AREA->get_player(0);
			util::Point<int> pos = player->get_position();
			util::Point<float> offset = player->get_offset();
			util::Size<int> player_size = player->get_size();
			util::Point<float> sz(player_size.w / 2.0f, 1.0f - player_size.h / 2.0f);
			wedge::add_tiles(pos, offset, sz);
			util::Point<float> map_offset = current_area->get_centred_offset(pos, offset, true);
			dust_image->start_batch();
			draw_dust(map_offset);
			dust_image->end_batch();

			wedge::Game::draw();

			Uint32 now = GET_TICKS();

			if (pausing) {
				Uint32 diff = now - pause_start_time;
				if ((int)diff >= pause_fade_time) {
					diff = pause_fade_time;
				}
				float p = diff / (float)pause_fade_time;
				p = p * p;
				SDL_Colour *colours = start_bander(num_bands(shim::screen_size.h), shim::palette[17], shim::palette[19], p);
				gfx::draw_filled_rectangle(colours, util::Point<int>(0, 0), shim::screen_size);
				end_bander();
			}
			else if (int(now - pause_end_time) <= pause_fade_time) {
				Uint32 diff = now - pause_end_time;
				float p = 1.0f - (diff / (float)pause_fade_time);
				p = p * p;
				SDL_Colour *colours = start_bander(num_bands(shim::screen_size.h), shim::palette[17], shim::palette[19], p);
				gfx::draw_filled_rectangle(colours, util::Point<int>(0, 0), shim::screen_size);
				end_bander();
			}
	
			if (gameover) {
				Uint32 now = GET_TICKS();
				Uint32 end = gameover_time + GLOBALS->gameover_timeout;
				Uint32 diff;
				if (now > end) {
					diff = 0;
				}
				else {
					diff = end-now;
				}
				if ((int)diff <= GLOBALS->gameover_fade_time) {
					float p = 1.0f - ((float)diff / GLOBALS->gameover_fade_time);
					p = p * p;
					SDL_Colour colour;
					colour.r = GLOBALS->gameover_fade_colour.r * p;
					colour.g = GLOBALS->gameover_fade_colour.g * p;
					colour.b = GLOBALS->gameover_fade_colour.b * p;
					colour.a = GLOBALS->gameover_fade_colour.a * p;
					gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
				}
			}
		}
	}

	if (fading_in) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - fade_start_time;
		if ((int)elapsed > change_area_fade_duration) {
			elapsed = change_area_fade_duration;
		}

		float p;
		if ((int)elapsed >= change_area_fade_duration/2) {
			elapsed -= change_area_fade_duration/2;
			p = 1.0f - (elapsed / (change_area_fade_duration/2.0f));
		}
		else {
			p = elapsed / (change_area_fade_duration/2.0f);
		}
		p = MAX(0.0f, MIN(1.0f, p));
		SDL_Colour black;
		black.r = fadeout_colour.r * p;
		black.g = fadeout_colour.g * p;
		black.b = fadeout_colour.b * p;
		black.a = p * 255;
		gfx::draw_filled_rectangle(black, util::Point<int>(0, 0), shim::screen_size);
	}
}

void Monster_RPG_3_Area_Game::draw_fore()
{
	if (pausing == false && paused == false) {
		wedge::Area_Game::draw_fore();
	}
}

void Monster_RPG_3_Area_Game::draw_dust(util::Point<float> dust_offset)
{
	if (paused == false && current_area && current_area->get_name().substr(0, 6) == "desert") {
		for (size_t i = 0; i < dust.size(); i++) {
			Dust &d = dust[i];
			int level_w = shim::tile_size*20;
			int x = d.position.x;
			int section = level_w / d.cycles;
			int m = x % section;
			float p = (float)m / section * M_PI * 2.0f;
			float y_off = sin(p) * DUST_AMPLITUDE;
			util::Point<float> pos = dust_offset + util::Point<float>(d.position.x, d.position.y+y_off);
			if (pos.x < -dust_image->size.w/2.0f || pos.y < -dust_image->size.h/2.0f || pos.x > shim::screen_size.w+dust_image->size.w/2.0f || pos.y > shim::screen_size.h+dust_image->size.h/2.0f) {
				continue;
			}
			dust_image->draw_rotated(util::Point<float>(dust_image->size.w/2.0f, dust_image->size.h/2.0f), pos, p, 0);
		}
	}
}

wedge::Area *Monster_RPG_3_Area_Game::create_area(std::string name)
{
	area_create_count++;
	return new wedge::Area(name);
}

wedge::Area *Monster_RPG_3_Area_Game::create_area(util::JSON::Node *json)
{
	area_create_count++;
	return new wedge::Area(json);
}

static void start_credits(void *data)
{
	wedge::Area_Hooks *hooks = AREA->get_current_area()->get_hooks();
	static_cast<Area_Hooks_Mountains1 *>(hooks)->start_credits();
}

void Monster_RPG_3_Area_Game::battle_ended(wedge::Battle_Game *battle)
{
	wedge::Area_Game::battle_ended(battle);

	if (dynamic_cast<Battle_Fiddler *>(battle) != NULL) {
		return;
	}

	if (INSTANCE->is_milestone_complete(MS_FIRST_RANDOM_BATTLE) == false) {
		wedge::Map_Entity *eny = get_player(ENY);
		wedge::Map_Entity *tiggy = get_player(TIGGY);
		eny->face(tiggy, false);
		tiggy->face(eny, false);
		INSTANCE->set_milestone_complete(MS_FIRST_RANDOM_BATTLE, true);
		if (can_autosave()) {
			autosave(true);
		}
		wedge::pause_presses(true);
		NEW_SYSTEM_AND_TASK(AREA)
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1307)/* Originally: What are these things??? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1309)/* Originally: More monsters! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1311)/* Originally: Let's get out of here! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)
	}
	else if (INSTANCE->is_milestone_complete(MS_SECOND_RANDOM_BATTLE) == false) {
		wedge::Map_Entity *eny = get_player(ENY);
		wedge::Map_Entity *tiggy = get_player(TIGGY);
		eny->face(tiggy, false);
		tiggy->face(eny, false);
		INSTANCE->set_milestone_complete(MS_SECOND_RANDOM_BATTLE, true);
		if (can_autosave()) {
			autosave(true);
		}
		wedge::pause_presses(true);
		NEW_SYSTEM_AND_TASK(AREA)
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1313)/* Originally: I'm worried about the folks in Riverside! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1315)/* Originally: Let's go! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)
	}
	else if (current_area->get_name() == "hh1" && INSTANCE->is_milestone_complete(MS_FIRST_HH_RANDOM_BATTLE) == false) {
		wedge::Map_Entity *eny = get_player(ENY);
		wedge::Map_Entity *tiggy = get_player(TIGGY);
		eny->face(tiggy, false);
		tiggy->face(eny, false);
		INSTANCE->set_milestone_complete(MS_FIRST_HH_RANDOM_BATTLE, true);
		if (can_autosave()) {
			autosave(true);
		}
		wedge::pause_presses(true);
		NEW_SYSTEM_AND_TASK(AREA)
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1317)/* Originally: There are definitely monsters here! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1319)/* Originally: This could be the source of the outbreak! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1321)/* Originally: Let's keep looking... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)
	}
	else if (current_area->get_name() == "hh2" && INSTANCE->is_milestone_complete(MS_FIRST_HH2_RANDOM_BATTLE) == false) {
		wedge::Map_Entity *eny = get_player(ENY);
		wedge::Map_Entity *tiggy = get_player(TIGGY);
		eny->face(tiggy, false);
		tiggy->face(eny, false);
		INSTANCE->set_milestone_complete(MS_FIRST_HH2_RANDOM_BATTLE, true);
		if (can_autosave()) {
			autosave(true);
		}
		wedge::pause_presses(true);
		NEW_SYSTEM_AND_TASK(AREA)
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1323)/* Originally: I think we're on the right track! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1325)/* Originally: Keep looking! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)
	}
	else if (current_area->get_name().substr(0, 8)  == "mountain" && INSTANCE->is_milestone_complete(MS_GETTING_CLOSE) == false) {
		wedge::Map_Entity *eny = get_player(ENY);
		wedge::Map_Entity *tiggy = get_player(TIGGY);
		eny->face(tiggy, false);
		tiggy->face(eny, false);
		INSTANCE->set_milestone_complete(MS_GETTING_CLOSE, true);
		if (can_autosave()) {
			autosave(true);
		}
		wedge::pause_presses(true);
		NEW_SYSTEM_AND_TASK(AREA)
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1656)/* Originally: These guys are tough. We must be getting close! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_AUTO, new_task))
		ADD_STEP(new wedge::Pause_Presses_Step(false, false, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)
	}
	else if (dynamic_cast<Battle_Palla *>(battle) != NULL) {
		palla_battle_done = 1;
	}
	else if (dynamic_cast<Battle_Gayan *>(battle) != NULL) {
		battle->set_music_backup(new audio::MML("music/happy.mml"));

		wedge::Map_Entity *eny = get_player(ENY);
		wedge::Map_Entity *tiggy = get_player(TIGGY);
		gfx::Sprite *eny_sprite = eny->get_sprite();
		gfx::Sprite *tiggy_sprite = tiggy->get_sprite();
		eny_sprite->set_animation("dead");
		tiggy_sprite->set_animation("dead");

		wedge::Area *area = AREA->get_current_area();
		wedge::Map_Entity *gayan = area->find_entity("gayan");
		gfx::Sprite *gayan_sprite = gayan->get_sprite();
				
		NEW_SYSTEM_AND_TASK(AREA)
		ADD_STEP(new wedge::A_Star_Step(gayan, util::Point<int>(16, 9), new_task))
		ADD_STEP(new wedge::Play_Animation_Step(gayan_sprite, "stand_s", new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1666)/* Originally: They... died for me.^Knowing how awful I am, they died for me! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->cry, false, false, new_task, 2.0f))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1667)/* Originally: *crying* */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(eny_sprite, "sit_n", new_task))
		ADD_STEP(new wedge::Delay_Step(1000, new_task));
		ADD_STEP(new wedge::Play_Animation_Step(eny_sprite, "stand_n", new_task))
		ADD_STEP(new wedge::Delay_Step(1000, new_task));
		ADD_STEP(new wedge::Play_Animation_Step(tiggy_sprite, "sit_n", new_task))
		ADD_STEP(new wedge::Delay_Step(1000, new_task));
		ADD_STEP(new wedge::Play_Animation_Step(tiggy_sprite, "stand_n", new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1669)/* Originally: What happened? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->jump, false, false, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(gayan_sprite, "jump", new_task));
		ADD_STEP(new wedge::Play_Sound_Step(M3_GLOBALS->jump, false, false, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(gayan_sprite, "jump", new_task));
		ADD_STEP(new wedge::Play_Animation_Step(gayan_sprite, "stand_s", new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1670)/* Originally: You're alive! I can't believe it! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1671)/* Originally: Feeling better are we? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1672)/* Originally: I thought with the terrible deed I committed, I'd never be forgiven... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1673)/* Originally: What is it you did? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1674)/* Originally: It was my brother's birthday... I baked him a cake... with rare giant fiddlehead icing... and it poisoned him... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1675)/* Originally: I'm sorry. But now you must move forward and forgive yourself. Not doing so just causes more problems! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(gayan_sprite, "laugh", new_task));
		ADD_STEP(new wedge::Play_Animation_Step(gayan_sprite, "laugh", new_task));
		ADD_STEP(new wedge::Play_Animation_Step(gayan_sprite, "stand_s", new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1676)/* Originally: *laughs* I have been stirring up heaps of trouble.^Thank you both. I feel better! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1677)/* Originally: Come on, let's go do something fun to get your mind off your troubles! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(tiggy_sprite, "stand_w", new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1)/* Originally: Tiggy */ + TAG_END, GLOBALS->game_t->translate(1678)/* Originally: Can we find some shoes first? My feet hurt! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(tiggy_sprite, "stand_n", new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1645)/* Originally: Gayan */ + TAG_END, GLOBALS->game_t->translate(1679)/* Originally: Why are you barefoot? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1680)/* Originally: Eny, Tiggy */ + TAG_END, GLOBALS->game_t->translate(1668)/* ... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(eny_sprite, "laugh", new_task))
		ADD_STEP(new wedge::Play_Animation_Step(tiggy_sprite, "laugh", new_task))
		ADD_STEP(new wedge::Play_Animation_Step(gayan_sprite, "laugh", new_task))
		ADD_STEP(new Dialogue_Step(GLOBALS->game_t->translate(1681)/* Originally: Eny, Tiggy, Gayan */ + TAG_END, GLOBALS->game_t->translate(1682)/* ... */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM, new_task))
		ADD_STEP(new wedge::Delay_Step(1000, new_task));
		ADD_STEP(new wedge::Generic_Immediate_Callback_Step(start_credits, (void *)0, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)

		return;
	}
	
	if (can_autosave()) {
		autosave(true);
	}
}

void Monster_RPG_3_Area_Game::set_next_fadeout_colour(SDL_Colour colour)
{
	fadeout_colour = colour;
}

void Monster_RPG_3_Area_Game::set_use_camera(bool use_camera)
{
	this->use_camera = use_camera;
}

void Monster_RPG_3_Area_Game::set_camera(util::Point<float> camera)
{
	this->camera = camera;
}

void Monster_RPG_3_Area_Game::handle_event(TGUI_Event *event)
{
	if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_b4) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b4)
	) {
		show_settings();
	}
	else if (settings_active() == false && (shim::guis.size() == 0 || dynamic_cast<Save_Slot_GUI *>(shim::guis.back()) == NULL) && ((active_inns().size() == 0 && active_questions().size() == 0) || shim::guis.size() == 0)) {
		wedge::Area_Game::handle_event(event);
	}

#ifdef DEBUG
	if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_b) {
#else
	if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_b && util::bool_arg(false, shim::argc, shim::argv, "b-for-battle")) {
#endif
		wedge::Battle_Game *battle_game = new Battle_1Reaper1Wraith();
		battle_game->start_transition_in();
	}
}

bool Monster_RPG_3_Area_Game::run()
{
	if (current_area && current_area->get_name().substr(0, 6) == "desert") {
		for (size_t i = 0; i < dust.size(); i++) {
			Dust &d = dust[i];
			d.position.x += d.speed;
			if (d.position.x > shim::tile_size*20 + dust_image->size.w) {
				d.position.x = -dust_image->size.w;
				d.position.y = util::rand(0, shim::tile_size*20-1);
			}
		}
	}

	std::string prev_area_name = current_area->get_name();
	bool f = fading_in;

	bool ret = wedge::Area_Game::run();

	std::string curr_area_name = current_area->get_name();

	if (f && prev_area_name.substr(0, 6) == "desert" && curr_area_name.substr(0, 6) != "desert") {
		M3_GLOBALS->wind1->stop();
		M3_GLOBALS->wind2->stop();
		M3_GLOBALS->wind3->stop();
		GLOBALS->max_battle_steps = Monster_RPG_3_Globals::MAX_BATTLE_STEPS;
		GLOBALS->min_battle_steps = Monster_RPG_3_Globals::MIN_BATTLE_STEPS;
	}

	if (prev_chests_opened != INSTANCE->chests_opened) {
		prev_chests_opened = INSTANCE->chests_opened;
		if (prev_chests_opened >= CHESTS_FOR_ACHIEVEMENT) {
			util::achieve((void *)ACHIEVE_FOUND_ITEMS);
		}
	}

	return ret;
}

int Monster_RPG_3_Area_Game::get_num_areas_created()
{
	return area_create_count;
}
