#include "Nooskewl_Shim/shaders/glsl/default_fragment.h"

#if defined ANDROID || defined IOS || defined RASPBERRYPI
#include "Nooskewl_Shim/shaders/glsl/default_vertex.h"
#include "Nooskewl_Shim/shaders/glsl/default_textured_fragment.h"
#endif

#ifdef USE_D3DX
#include "Nooskewl_Shim/shaders/hlsl/default_vertex.h"
#include "Nooskewl_Shim/shaders/hlsl/default_fragment.h"
#endif

#ifdef TVOS
#include "Nooskewl_Shim/ios.h"
#endif

#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/map_entity.h>

#include "achievements.h"
#include "dialogue.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "inventory.h"
#include "milestones.h"
#include "monster_rpg_3.h"
#include "spells.h"
#include "stats.h"

static int icicle_compare_vertices(const void *a, const void *b)
{
	float *f1 = (float *)a;
	float *f2 = (float *)b;
	float *a1 = f1;
	float *a2 = f1 + 12;
	float *a3 = f1 + 24;
	float *b1 = f2;
	float *b2 = f2 + 12;
	float *b3 = f2 + 24;

	float x1, y1, z1;
	float x2, y2, z2;

	barycenter(a1, a2, a3, &x1, &y1, &z1);
	barycenter(b1, b2, b3, &x2, &y2, &z2);

	return z1 < z2 ? -1 : (z1 == z2 ? 0 : 1);
}

Monster_RPG_3_Globals::Monster_RPG_3_Globals() :
	wedge::Globals()
{
	terminate = false;

	player_start_positions.push_back(util::Point<int>(2, 11));
	player_start_directions.push_back(wedge::DIR_N);

	levels.push_back(Level());
	levels[0].experience = 250;
	levels[0].stat_increases.max_hp = 25;
	levels[0].stat_increases.max_mp = 10;
	levels[0].stat_increases.attack = 5;
	levels[0].stat_increases.defense = 0;
	levels[0].stat_increases.set_extra(LUCK, 5);

	levels.push_back(Level());
	levels[1].experience = 1000;
	levels[1].stat_increases.max_hp = 25;
	levels[1].stat_increases.max_mp = 15;
	levels[1].stat_increases.attack = 5;
	levels[1].stat_increases.defense = 5;
	levels[1].stat_increases.set_extra(LUCK, 5);

	levels.push_back(Level());
	levels[2].experience = 2500;
	levels[2].stat_increases.max_hp = 100;
	levels[2].stat_increases.max_mp = 50;
	levels[2].stat_increases.attack = 5;
	levels[2].stat_increases.defense = 5;
	levels[2].stat_increases.set_extra(LUCK, 10);

	levels.push_back(Level());
	levels[3].experience = 7500;
	levels[3].stat_increases.max_hp = 50;
	levels[3].stat_increases.max_mp = 25;
	levels[3].stat_increases.attack = 5;
	levels[3].stat_increases.defense = 5;
	levels[3].stat_increases.set_extra(LUCK, 0);

	levels.push_back(Level());
	levels[4].experience = 10000;
	levels[4].stat_increases.max_hp = 50;
	levels[4].stat_increases.max_mp = 25;
	levels[4].stat_increases.attack = 5;
	levels[4].stat_increases.defense = 5;
	levels[4].stat_increases.set_extra(LUCK, 5);

	levels.push_back(Level());
	levels[5].experience = 15000;
	levels[5].stat_increases.max_hp = 649;
	levels[5].stat_increases.max_mp = 100;
	levels[5].stat_increases.attack = 15;
	levels[5].stat_increases.defense = 15;
	levels[5].stat_increases.set_extra(LUCK, 5);

	for (int i = 0; i < ITEM_SIZE; i++) {
		item_sfx.push_back(NULL); // default no sound
	}
	item_sfx[ITEM_POTION] = new audio::MML("sfx/potion.mml");
	item_sfx[ITEM_CURE] = new audio::MML("sfx/cure.mml");
	item_sfx[ITEM_HOLY_WATER] = new audio::MML("sfx/holy_water.mml");
	item_sfx[ITEM_ELIXIR] = new audio::MML("sfx/elixir.mml");
	item_sfx[ITEM_BAIT] = new audio::MML("sfx/potion.mml");
	item_sfx[ITEM_POTION_PLUS] = new audio::MML("sfx/potion_plus.mml");
	item_sfx[ITEM_APPLE] = new audio::MML("sfx/potion_omega.mml");
	item_sfx[ITEM_FISH] = new audio::MML("sfx/potion_omega.mml");
	item_sfx[ITEM_POTION_OMEGA] = new audio::MML("sfx/potion_omega.mml");

	spell_sfx["Fire"] = new audio::MML("sfx/fire.mml");
	spell_sfx["Bolt"] = new audio::MML("sfx/bolt.mml");
	spell_sfx["Ice"] = new audio::MML("sfx/ice.mml");
	spell_sfx["Heal"] = new audio::MML("sfx/potion.mml");
	spell_sfx["Cure"] = new audio::MML("sfx/cure.mml");
	spell_sfx["Heal Plus"] = new audio::MML("sfx/potion_plus.mml");
	spell_sfx["Heal Omega"] = new audio::MML("sfx/potion_omega.mml");
	
	spell_sfx["vBolt"] = new audio::MML("sfx/vbolt.mml");

	spell_interface = new Monster_RPG_3_Spell_Interface();
	object_interface = new Monster_RPG_3_Object_Interface();

	melee = new audio::MML("sfx/melee.mml");
	run = new audio::MML("sfx/run.mml");
	sleep = new audio::MML("sfx/sleep.mml");
	buysell = new audio::MML("sfx/buysell.mml");
	fire = new audio::MML("sfx/fire.mml");
	jump = new audio::MML("sfx/jump.mml");
	second_chance = new audio::MML("sfx/second_chance.mml");
	cast_line = new audio::MML("sfx/cast_line.mml");
	splash = new audio::MML("sfx/splash.mml");
	blind = new audio::MML("sfx/blind.mml");
	pendants = new audio::MML("sfx/pendants.mml");
	grow = new audio::MML("sfx/grow.mml");
		
	vbolt_sample = new audio::Sample("vbolt.ogg");
	wind1 = new audio::Sample("wind1.ogg");
	wind2 = new audio::Sample("wind2.ogg");
	wind3 = new audio::Sample("wind3.ogg");
	vice_sample = new audio::Sample("vice.ogg");
	cry = new audio::Sample("cry.ogg");

	up_arrow = new gfx::Image("ui/up_arrow.tga");
	down_arrow = new gfx::Image("ui/down_arrow.tga");
	selection_arrow = new gfx::Image("ui/selection_arrow.tga");
	nomore = new gfx::Image("ui/nomore.tga");
	profile_images[0] = new gfx::Image("misc/eny_profile.tga");
	profile_images[1] = new gfx::Image("misc/tiggy_profile.tga");
	mini_profile_images[0] = new gfx::Image("misc/mini_eny.tga");
	mini_profile_images[1] = new gfx::Image("misc/mini_tiggy.tga");
	play_pause = new gfx::Image("ui/play_pause.tga");
	bottom_shadow = new gfx::Image("misc/bottom_shadow.tga");

	blind_sprite = new gfx::Sprite("blind");

	if (shim::opengl) {
		std::string alpha_fragment_source = util::load_text("gfx/shaders/glsl/alpha_fragment.txt");
		std::string bander_fragment_source = util::load_text("gfx/shaders/glsl/bander_fragment.txt");
		std::string blur_fragment_source = util::load_text("gfx/shaders/glsl/blur_fragment.txt");
		std::string darken_fragment_source = util::load_text("gfx/shaders/glsl/darken_fragment.txt");
		std::string darken_textured_fragment_source = util::load_text("gfx/shaders/glsl/darken_textured_fragment.txt");
		std::string darken_both_fragment_source = util::load_text("gfx/shaders/glsl/darken_both_fragment.txt");
		std::string darkness_fragment_source = util::load_text("gfx/shaders/glsl/darkness_fragment.txt");
		std::string solid_colour_fragment_source = util::load_text("gfx/shaders/glsl/solid_colour_fragment.txt");
		std::string heat_wave_fragment_source = util::load_text("gfx/shaders/glsl/heat_wave_fragment.txt");
		std::string alpha_test_fragment_source = util::load_text("gfx/shaders/glsl/alpha_test_fragment.txt");
		std::string lit_3d_vertex_source = util::load_text("gfx/shaders/glsl/lit_3d_vertex.txt");
		std::string default_fragment_source = DEFAULT_GLSL_FRAGMENT_SHADER;
		gfx::Shader::OpenGL_Shader *alpha_fragment = gfx::Shader::load_opengl_fragment_shader(alpha_fragment_source);
		gfx::Shader::OpenGL_Shader *bander_fragment = gfx::Shader::load_opengl_fragment_shader(bander_fragment_source);
		gfx::Shader::OpenGL_Shader *blur_fragment = gfx::Shader::load_opengl_fragment_shader(blur_fragment_source);
		gfx::Shader::OpenGL_Shader *darken_fragment = gfx::Shader::load_opengl_fragment_shader(darken_fragment_source);
		gfx::Shader::OpenGL_Shader *darken_textured_fragment = gfx::Shader::load_opengl_fragment_shader(darken_textured_fragment_source);
		gfx::Shader::OpenGL_Shader *darken_both_fragment = gfx::Shader::load_opengl_fragment_shader(darken_both_fragment_source);
		gfx::Shader::OpenGL_Shader *darkness_fragment = gfx::Shader::load_opengl_fragment_shader(darkness_fragment_source);
		gfx::Shader::OpenGL_Shader *solid_colour_fragment = gfx::Shader::load_opengl_fragment_shader(solid_colour_fragment_source);
		gfx::Shader::OpenGL_Shader *heat_wave_fragment;
		gfx::Shader::OpenGL_Shader *alpha_test_fragment;
#ifdef ANDROID
		if (have_highp_fragment_shaders)
#endif
		{
			heat_wave_fragment = gfx::Shader::load_opengl_fragment_shader(heat_wave_fragment_source, gfx::Shader::HIGH);
			alpha_test_fragment = gfx::Shader::load_opengl_fragment_shader(alpha_test_fragment_source, gfx::Shader::HIGH);
		}
#ifdef ANDROID
		else {
			alpha_test_fragment = gfx::Shader::load_opengl_fragment_shader(alpha_test_fragment_source, gfx::Shader::MEDIUM);
		}
#endif
		gfx::Shader::OpenGL_Shader *lit_3d_vertex = gfx::Shader::load_opengl_vertex_shader(lit_3d_vertex_source, gfx::Shader::HIGH);
		gfx::Shader::OpenGL_Shader *default_fragment = gfx::Shader::load_opengl_fragment_shader(default_fragment_source);
		alpha_shader = new gfx::Shader(default_opengl_vertex_shader, alpha_fragment, false, true);
		bander_shader = new gfx::Shader(default_opengl_vertex_shader, bander_fragment, false, true);
		blur_shader = new gfx::Shader(default_opengl_vertex_shader, blur_fragment, false, true);
		darken_shader = new gfx::Shader(default_opengl_vertex_shader, darken_fragment, false, true);
		darken_textured_shader = new gfx::Shader(default_opengl_vertex_shader, darken_textured_fragment, false, true);
		darken_both_shader = new gfx::Shader(default_opengl_vertex_shader, darken_both_fragment, false, true);
		darkness_shader = new gfx::Shader(default_opengl_vertex_shader, darkness_fragment, false, true);
		solid_colour_shader = new gfx::Shader(default_opengl_vertex_shader, solid_colour_fragment, false, true);
#ifdef ANDROID
		if (have_highp_fragment_shaders)
#endif
		{
			heat_wave_shader = new gfx::Shader(default_opengl_vertex_shader, heat_wave_fragment, false, true);
		}
#ifdef ANDROID
		else {
			heat_wave_shader = NULL;
		}
#endif
		alpha_test_shader = new gfx::Shader(default_opengl_vertex_shader, alpha_test_fragment, false, true);
		lit_3d_shader = new gfx::Shader(lit_3d_vertex, default_fragment, true, true);
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		default_vertex = util::load_text("gfx/shaders/hlsl/default_vertex.txt");
		default_fragment = DEFAULT_HLSL_FRAGMENT_SHADER;
		alpha_fragment = util::load_text("gfx/shaders/hlsl/alpha_fragment.txt");
		bander_fragment = util::load_text("gfx/shaders/hlsl/bander_fragment.txt");
		blur_fragment = util::load_text("gfx/shaders/hlsl/blur_fragment.txt");
		darken_fragment = util::load_text("gfx/shaders/hlsl/darken_fragment.txt");
		darken_textured_fragment = util::load_text("gfx/shaders/hlsl/darken_textured_fragment.txt");
		darken_both_fragment = util::load_text("gfx/shaders/hlsl/darken_both_fragment.txt");
		darkness_fragment = util::load_text("gfx/shaders/hlsl/darkness_fragment.txt");
		solid_colour_fragment = util::load_text("gfx/shaders/hlsl/solid_colour_fragment.txt");
		heat_wave_fragment = util::load_text("gfx/shaders/hlsl/heat_wave_fragment.txt");
		alpha_test_fragment = util::load_text("gfx/shaders/hlsl/alpha_test_fragment.txt");
		lit_3d_vertex = util::load_text("gfx/shaders/hlsl/lit_3d_vertex.txt");
		alpha_shader = new gfx::Shader(shim::opengl, default_vertex, alpha_fragment);
		bander_shader = new gfx::Shader(shim::opengl, default_vertex, bander_fragment, gfx::Shader::HIGH);
		blur_shader = new gfx::Shader(shim::opengl, default_vertex, blur_fragment);
		darken_shader = new gfx::Shader(shim::opengl, default_vertex, darken_fragment);
		darken_textured_shader = new gfx::Shader(shim::opengl, default_vertex, darken_textured_fragment);
		darken_both_shader = new gfx::Shader(shim::opengl, default_vertex, darken_both_fragment);
		darkness_shader = new gfx::Shader(shim::opengl, default_vertex, darkness_fragment);
		solid_colour_shader = new gfx::Shader(shim::opengl, default_vertex, solid_colour_fragment);
		heat_wave_shader = new gfx::Shader(shim::opengl, default_vertex, heat_wave_fragment);
		alpha_test_shader = new gfx::Shader(shim::opengl, default_vertex, alpha_test_fragment);
		lit_3d_shader = new gfx::Shader(shim::opengl, lit_3d_vertex, default_fragment);
#else
		gfx::Shader::D3D_Fragment_Shader *alpha_fragment = gfx::Shader::load_d3d_fragment_shader("alpha_fragment");
		gfx::Shader::D3D_Fragment_Shader *bander_fragment = gfx::Shader::load_d3d_fragment_shader("bander_fragment");
		gfx::Shader::D3D_Fragment_Shader *blur_fragment = gfx::Shader::load_d3d_fragment_shader("blur_fragment");
		gfx::Shader::D3D_Fragment_Shader *darken_fragment = gfx::Shader::load_d3d_fragment_shader("darken_fragment");
		gfx::Shader::D3D_Fragment_Shader *darken_textured_fragment = gfx::Shader::load_d3d_fragment_shader("darken_textured_fragment");
		gfx::Shader::D3D_Fragment_Shader *darken_both_fragment = gfx::Shader::load_d3d_fragment_shader("darken_both_fragment");
		gfx::Shader::D3D_Fragment_Shader *darkness_fragment = gfx::Shader::load_d3d_fragment_shader("darkness_fragment");
		gfx::Shader::D3D_Fragment_Shader *solid_colour_fragment = gfx::Shader::load_d3d_fragment_shader("solid_colour_fragment");
		gfx::Shader::D3D_Fragment_Shader *heat_wave_fragment = gfx::Shader::load_d3d_fragment_shader("heat_wave_fragment");
		gfx::Shader::D3D_Fragment_Shader *alpha_test_fragment = gfx::Shader::load_d3d_fragment_shader("alpha_test_fragment");
		gfx::Shader::D3D_Vertex_Shader *lit_3d_vertex = gfx::Shader::load_d3d_vertex_shader("lit_3d_vertex");
		gfx::Shader::D3D_Fragment_Shader *default_fragment = gfx::Shader::load_d3d_fragment_shader("noo_default_fragment");
		alpha_shader = new gfx::Shader(default_d3d_vertex_shader, alpha_fragment, false, true);
		bander_shader = new gfx::Shader(default_d3d_vertex_shader, bander_fragment, false, true);
		blur_shader = new gfx::Shader(default_d3d_vertex_shader, blur_fragment, false, true);
		darken_shader = new gfx::Shader(default_d3d_vertex_shader, darken_fragment, false, true);
		darken_textured_shader = new gfx::Shader(default_d3d_vertex_shader, darken_textured_fragment, false, true);
		darken_both_shader = new gfx::Shader(default_d3d_vertex_shader, darken_both_fragment, false, true);
		darkness_shader = new gfx::Shader(default_d3d_vertex_shader, darkness_fragment, false, true);
		solid_colour_shader = new gfx::Shader(default_d3d_vertex_shader, solid_colour_fragment, false, true);
		heat_wave_shader = new gfx::Shader(default_d3d_vertex_shader, heat_wave_fragment, false, true);
		alpha_test_shader = new gfx::Shader(default_d3d_vertex_shader, alpha_test_fragment, false, true);
		lit_3d_shader = new gfx::Shader(lit_3d_vertex, default_fragment, true, true);
#endif
	}
#endif

	zeus = new gfx::Model("zeus.nsm");
	icicle = new gfx::Model("icicle.nsm");
	dragon = new gfx::Model("dragon.nsm");

	// Sort icicle triangles by Y, needed for the effect of smashing it apart
	gfx::Model::Node *node = icicle->find("Model");
	qsort(node->vertices, node->num_vertices / 3, 36 * sizeof(float), icicle_compare_vertices);
	memcpy(node->animated_vertices, node->vertices, sizeof(float) * node->num_vertices * 12);

	red_triangle_colour = shim::palette[26];
	gameover_fade_colour = shim::palette[8];

	key_b4 = TGUIK_F1;

	tv_safe_mode = false;
	hide_onscreen_settings_button = false;

	darken_screen_on_next_dialogue = false;

	max_battle_steps = Monster_RPG_3_Globals::MAX_BATTLE_STEPS;
	min_battle_steps = Monster_RPG_3_Globals::MIN_BATTLE_STEPS;
}

Monster_RPG_3_Globals::~Monster_RPG_3_Globals()
{
	delete melee;
	delete run;
	delete sleep;
	delete buysell;
	delete fire;
	delete jump;
	delete second_chance;
	delete cast_line;
	delete splash;
	delete blind;
	delete pendants;
	delete grow;

	delete vbolt_sample;
	delete wind1;
	delete wind2;
	delete wind3;
	delete vice_sample;
	delete cry;
	
	delete up_arrow;
	delete down_arrow;
	delete selection_arrow;
	delete nomore;
	delete profile_images[0];
	delete profile_images[1];
	delete mini_profile_images[0];
	delete mini_profile_images[1];
	delete play_pause;
	delete bottom_shadow;

	delete blind_sprite;

	delete alpha_shader;
	delete bander_shader;
	delete darken_shader;
	delete darken_textured_shader;
	delete darken_both_shader;
	delete darkness_shader;
	delete solid_colour_shader;
	delete blur_shader;
	delete heat_wave_shader;
	delete lit_3d_shader;
	delete alpha_test_shader;

	delete zeus;
	delete icicle;
	delete dragon;
}

void Monster_RPG_3_Globals::do_dialogue(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Step *monitor)
{
	wedge::Game *g;
	if (MENU) {
		g = MENU;
	}
	else if (BATTLE) {
		g = BATTLE;
	}
	else {
		g = AREA;
	}

	NEW_SYSTEM_AND_TASK(g)
	Dialogue_Step *d = new Dialogue_Step(tag, text, type, position, new_task, darken_screen_on_next_dialogue);
	darken_screen_on_next_dialogue = false;
	if (monitor) {
		d->add_monitor(monitor);
	}
	for (size_t i = 0; i < next_dialogue_monitors.size(); i++) {
		d->add_monitor(next_dialogue_monitors[i]);
	}
	next_dialogue_monitors.clear();
	ADD_STEP(d)
	ADD_TASK(new_task)
	FINISH_SYSTEM(g)
}

bool Monster_RPG_3_Globals::dialogue_active(wedge::Game *game, bool only_if_initialised)
{
	std::vector<Dialogue_Step *> v = active_dialogues(game);

	if (only_if_initialised == false && v.size() > 0) {
		return true;
	}

	for (size_t i = 0; i < v.size(); i++) {
		if (v[i]->is_initialised()) {
			return true;
		}
	}

	return false;
}

void Monster_RPG_3_Globals::add_yes_no_gui(std::string text, bool escape_cancels, bool selected, util::Callback callback, void *callback_data)
{
	Yes_No_GUI *gui = new Yes_No_GUI(text, escape_cancels, callback, callback_data);
	gui->set_selected(selected);
	gui->hook_omnipresent(true, true);
	shim::guis.push_back(gui);
}

void Monster_RPG_3_Globals::set_darken_screen_on_next_dialogue(bool darken)
{
	darken_screen_on_next_dialogue = darken;
}

bool Monster_RPG_3_Globals::add_title_gui()
{
	wind1->stop();
	wind2->stop();
	wind3->stop();

	if (terminate) {
		return false;
	}

	Title_GUI *title = new Title_GUI();
	shim::guis.push_back(title);

	return true;
}

util::Point<float> Monster_RPG_3_Globals::get_onscreen_button_position(wedge::Onscreen_Button button)
{
	dpad->set_animation("dpad");
	util::Size<int> dpad_size = dpad->get_current_image()->size / 3;
	dpad->set_animation("button1");
	util::Size<int> button_size = dpad->get_current_image()->size;
	int offset = shim::tile_size/2;
	float b1_x = shim::screen_size.w*0.95f-GLOBALS->dpad->get_image(0)->size.w;

	switch (button) {
		case wedge::ONSCREEN_UP:
			return util::Point<float>(offset+dpad_size.w, shim::screen_size.h-offset-dpad_size.h*3);
		case wedge::ONSCREEN_RIGHT:
			return util::Point<float>(offset+dpad_size.w*2, shim::screen_size.h-offset-dpad_size.h*2);
		case wedge::ONSCREEN_DOWN:
			return util::Point<float>(offset+dpad_size.w, shim::screen_size.h-offset-dpad_size.h);
		case wedge::ONSCREEN_LEFT:
			return util::Point<float>(offset, shim::screen_size.h-offset-dpad_size.h*2);
		case wedge::ONSCREEN_B1:
			return util::Point<float>(b1_x, shim::screen_size.h-offset-dpad_size.h*1.5f-button_size.h/2);
		case wedge::ONSCREEN_B2:
			return util::Point<float>(b1_x - button_size.w*1.5f, shim::screen_size.h-offset-dpad_size.h*1.5f-button_size.h/2);
		case wedge::ONSCREEN_B3: {
			//return util::Point<float>(shim::screen_size.w-offset-button_size.w*2.5f, shim::screen_size.h-offset-dpad_size.h*1.5f-button_size.h/2);
			util::Point<float> b1_pos = get_onscreen_button_position(wedge::ONSCREEN_B1);
			util::Point<float> b4_pos = get_onscreen_button_position(wedge::ONSCREEN_B4);
			util::Point<float> pos(b1_pos.x, (b1_pos.y+b4_pos.y)/2);
			return pos;
		}
		case wedge::ONSCREEN_B4: {
			// NOTE: Only works because dpad sprite animation is "button1" (as set above)
			return util::Point<float>(b1_x, shim::screen_size.h*0.05f+GLOBALS->bold_font->get_height());
		}
		default:
			return util::Point<float>(0, 0);
	}
}

void Monster_RPG_3_Globals::draw_custom_status(wedge::Map_Entity *entity, int status, util::Point<float> draw_pos)
{
	if (status == STATUS_BLIND) {
		gfx::Image *img = M3_GLOBALS->blind_sprite->get_current_image();
		util::Point<int> topleft, bottomright;
		gfx::Sprite *sprite = entity->get_sprite();
		sprite->get_bounds(topleft, bottomright);
		util::Point<int> sz = bottomright - topleft + util::Size<int>(1, 1);
		img->draw(draw_pos+topleft+util::Point<float>(sz.x/2-img->size.w/2, 0));
	}
}

bool Monster_RPG_3_Globals::can_walk()
{
	return can_show_settings(false, true, true, true) && shim::guis.size() == 0 && BATTLE == NULL;
}

bool Monster_RPG_3_Globals::title_gui_is_top()
{
	if (shim::guis.size() == 0) {
		return false;
	}
	return dynamic_cast<Title_GUI *>(shim::guis.back()) != NULL;
}

//--

Monster_RPG_3_Globals::Instance::Instance(util::JSON::Node *root) :
	wedge::Globals::Instance(root)
{
	for (int i = (int)milestones.size(); i < MS_SIZE; i++) {
		milestones.push_back(false);
	}

	boatin = false;
	boatin_pos = util::Point<int>(8, 6);
	boatin_direction = wedge::DIR_E;
	boat_w = true;
	boatin_done = false;

	fish_caught = 0;
	lucky_hits = 0;

	if (root) {
		util::JSON::Node *v = root->find("\"vampires\"");
		if (v != NULL) {
			for (size_t i = 0; i < v->children.size(); i++) {
				util::JSON::Node *node = v->children[i];
				add_vampire(util::JSON::trim_quotes(node->value), true);
			}
		}
		util::JSON::Node *n;
		n = root->find("\"boatin\"");
		if (n != NULL) {
			boatin = wedge::json_to_bool(n);
		}
		n = root->find("\"boatin_pos\"");
		if (n != NULL) {
			boatin_pos = wedge::json_to_integer_point(n);
		}
		n = root->find("\"boatin_direction\"");
		if (n != NULL) {
			boatin_direction = (wedge::Direction)wedge::json_to_integer(n);
		}
		n = root->find("\"boat_w\"");
		if (n != NULL) {
			boat_w = wedge::json_to_bool(n);
		}
		n = root->find("\"boatin_done\"");
		if (n != NULL) {
			boatin_done = wedge::json_to_bool(n);
		}
		n = root->find("\"fish_caught\"");
		if (n != NULL) {
			fish_caught = wedge::json_to_integer(n);
		}
		n = root->find("\"lucky_hits\"");
		if (n != NULL) {
			lucky_hits = wedge::json_to_integer(n);
		}
	}
	else {
		stats.push_back(wedge::Player_Stats());
		stats[0].name = "Eny";
		stats[0].sprite = new gfx::Sprite("eny");

		stats[0].level = 1;
		stats[0].experience = 0;
		stats[0].base.fixed.max_hp = 100;
		stats[0].base.fixed.max_mp = 25;
		stats[0].base.fixed.attack = 25;
		stats[0].base.fixed.defense = 25;
		stats[0].base.fixed.set_extra(LUCK, 10);
		stats[0].base.hp = stats[0].base.fixed.max_hp;
		stats[0].base.mp = stats[0].base.fixed.max_mp;
	}
}

Monster_RPG_3_Globals::Instance::~Instance()
{
}

std::string Monster_RPG_3_Globals::Instance::save()
{
	std::string s = wedge::Globals::Instance::save();
	s += ",";
	s += "\"vampires\": [";
	for (size_t i = 0; i < vampires.size(); i++) {
		s += "\"" + vampires[i] + "\"";
		if (i < vampires.size()-1) {
			s += ",";
		}
	}
	s += "],";
	s += "\"boatin\": " + wedge::bool_to_string(boatin) + ",";
	s += "\"boatin_pos\": " + wedge::integer_point_to_string(boatin_pos) + ",";
	s += "\"boatin_direction\": " + util::itos((int)boatin_direction) + ",";
	s += "\"boat_w\": " + wedge::bool_to_string(boat_w) + ",";
	s += "\"boatin_done\": " + wedge::bool_to_string(boatin_done) + ",";
	s += "\"fish_caught\": " + util::itos(fish_caught) + ",";
	s += "\"lucky_hits\": " + util::itos(lucky_hits);
	return s;
}

void Monster_RPG_3_Globals::Instance::add_vampire(std::string name, bool push_back)
{
	if (push_back) {
		vampires.push_back(name);
	}
	else {
		vampires.insert(vampires.begin(), name);
	}
}

std::string Monster_RPG_3_Globals::Instance::vampire(int index)
{
	return vampires[index];
}

int Monster_RPG_3_Globals::Instance::num_vampires()
{
	return (int)vampires.size();
}

std::vector<std::string> Monster_RPG_3_Globals::Instance::get_vampires()
{
	return vampires;
}
