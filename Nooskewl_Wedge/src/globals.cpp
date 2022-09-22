#include "Nooskewl_Shim/shaders/glsl/default_vertex.h"
#include "Nooskewl_Shim/shaders/glsl/default_fragment.h"

#ifdef _WIN32
#include "Nooskewl_Shim/shaders/hlsl/default_vertex.h"
#include "Nooskewl_Shim/shaders/hlsl/default_fragment.h"
#endif

#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/battle_game.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/input.h"
#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/omnipresent.h"
#include "Nooskewl_Wedge/spells.h"

using namespace wedge;

namespace wedge {

static Uint32 mini_pause_start;
static Uint32 mini_pause_total;
static bool mini_paused;
static Uint32 next_mini_pause_can_start;

static void quit_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = (Yes_No_GUI::Callback_Data *)data;
	bool quit = d->cancelled == false && d->choice;
	GLOBALS->quit(quit);
}

Globals *globals;

Globals::Globals()
{
	game_t = NULL;
	english_game_t = NULL;

	spell_interface = NULL;
	object_interface = NULL;
	area_game = NULL;
	battle_game = NULL;
	menu_game = NULL;
	omnipresent_game = NULL;
	shop_game = NULL;
	instance = NULL;

	max_gold = 9999;
	gameover_timeout = 10000;
	gameover_fade_time = 2500;

	key_b1 = TGUIK_RETURN;
	key_b2 = TGUIK_ESCAPE;
	key_b3 = TGUIK_e;
	key_switch = TGUIK_COMMA;
	key_l = TGUIK_LEFT;
	key_r = TGUIK_RIGHT;
	key_u = TGUIK_UP;
	key_d = TGUIK_DOWN;
	joy_b1 = 0;
	joy_b2 = 1;
	joy_b3 = 2;
	joy_b4 = 3;
#ifdef ANDROID
	joy_switch = 10;
#else
	joy_switch = 5;
#endif

	onscreen_controls_alpha = 0.333f;

#if defined ANDROID || defined IOS
	rumble_enabled = false;
#else
	rumble_enabled = true;
#endif

	try {
		poison_sprite = new gfx::Sprite("poison");
	}
	catch (util::Error e) {
		poison_sprite = NULL;
	}

	try {
		dpad = new gfx::Sprite("dpad");
	}
	catch (util::Error e) {
		dpad = NULL;
	}

	if (shim::opengl) {
#ifdef ANDROID
		GLint range, precision;
		glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, &range, &precision);
		if (range == 0 && precision == 0) {
			have_highp_fragment_shaders = false;
		}
		else {
			have_highp_fragment_shaders = true;
		}
#endif

		std::string default_vertex_source = DEFAULT_GLSL_VERTEX_SHADER;
		std::string enemy_die_fragment_source = util::load_text("gfx/shaders/glsl/enemy_die_fragment.txt");
		std::string boss_die_fragment_source = util::load_text("gfx/shaders/glsl/boss_die_fragment.txt");
		default_opengl_vertex_shader = gfx::Shader::load_opengl_vertex_shader(default_vertex_source, gfx::Shader::HIGH);
		gfx::Shader::OpenGL_Shader *enemy_die_fragment = gfx::Shader::load_opengl_fragment_shader(enemy_die_fragment_source);
		enemy_die_shader = new gfx::Shader(default_opengl_vertex_shader, enemy_die_fragment, true, true);
#ifdef ANDROID
		if (have_highp_fragment_shaders)
#endif
		{
			gfx::Shader::OpenGL_Shader *boss_die_fragment = gfx::Shader::load_opengl_fragment_shader(boss_die_fragment_source, gfx::Shader::HIGH);
			boss_die_shader = new gfx::Shader(default_opengl_vertex_shader, boss_die_fragment, false, true);
		}
#ifdef ANDROID
		else {
			boss_die_shader = NULL;
		}
#endif
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		default_vertex = util::load_text("gfx/shaders/hlsl/default_vertex.txt");
		enemy_die_fragment = util::load_text("gfx/shaders/hlsl/enemy_die_fragment.txt");
		boss_die_fragment = util::load_text("gfx/shaders/hlsl/boss_die_fragment.txt");
		enemy_die_shader = new gfx::Shader(shim::opengl, default_vertex, enemy_die_fragment);
		boss_die_shader = new gfx::Shader(shim::opengl, default_vertex, boss_die_fragment);
#else
		default_d3d_vertex_shader = gfx::Shader::load_d3d_vertex_shader("default_vertex");
		gfx::Shader::D3D_Fragment_Shader *enemy_die_fragment = gfx::Shader::load_d3d_fragment_shader("enemy_die_fragment");
		gfx::Shader::D3D_Fragment_Shader *boss_die_fragment = gfx::Shader::load_d3d_fragment_shader("boss_die_fragment");
		enemy_die_shader = new gfx::Shader(default_d3d_vertex_shader, enemy_die_fragment, true, true);
		boss_die_shader = new gfx::Shader(default_d3d_vertex_shader, boss_die_fragment, false, true);
#endif
	}
#endif

	if (shim::use_hires_font) {
#ifdef USE_TTF
		bold_font = new gfx::TTF("hbold.ttf", 8*int(shim::scale));
#else
		bold_font = new gfx::Pixel_Font("hbold");
#endif
	}
	else {
		bold_font = new gfx::Pixel_Font("bold");
	}
	bold_font->set_vertex_cache_id(2);
	bold_font->start_batch();

	chest = new audio::MML("sfx/chest.mml");
	battle_start = new audio::MML("sfx/battle_start.mml");
	hit = new audio::MML("sfx/hit.mml");
	enemy_die = new audio::MML("sfx/enemy_die.mml");
	boss_die = new audio::MML("sfx/boss_die.mml");
	enemy_attack = new audio::MML("sfx/enemy_attack.mml");
	button = new audio::MML("sfx/button.mml");
	button->set_pause_with_sfx(false);

	poison = NULL;
	levelup = NULL;

	try {
		poison = new audio::MML("sfx/poison.mml");
		levelup = new audio::MML("sfx/levelup.mml");
	}
	catch (util::Error e) {
	}

	gameover = new audio::MML("music/gameover.mml");

	boss_press = DIR_NONE;
	retry_boss = false;
	retried_boss = false;

	create_work_image();
	noise_data = NULL;
	create_noise();

	language = "English";
	
	onscreen_controller_was_enabled = false;
	onscreen_controller_temporarily_disabled = false;
}

Globals::~Globals()
{
	delete poison_sprite;
	delete dpad;

	delete enemy_die_shader;
	delete boss_die_shader;

	delete bold_font;
	
	delete chest;
	delete battle_start;
	delete hit;
	delete enemy_die;
	delete boss_die;
	delete enemy_attack;
	delete button;
	delete poison;
	delete levelup;

	delete gameover;

	for (size_t i = 0; i < item_sfx.size(); i++) {
		delete item_sfx[i];
	}

	std::map<std::string, audio::Sound *>::iterator it;
	for (it = spell_sfx.begin(); it != spell_sfx.end(); it++) {
		std::pair<std::string, audio::Sound *> p = *it;
		delete p.second;
	}

	delete spell_interface;
	delete object_interface;

	delete instance;

	delete work_image;

	delete noise;
	delete[] noise_data;

	delete game_t;
	delete english_game_t;
}

void Globals::add_next_dialogue_monitor(wedge::Step *monitor)
{
	next_dialogue_monitors.push_back(monitor);
}

void Globals::create_work_image()
{
	bool old_create_depth_buffer = gfx::Image::create_depth_buffer;
	gfx::Image::create_depth_buffer = true;
	work_image = new gfx::Image(shim::real_screen_size);
	gfx::Image::create_depth_buffer = old_create_depth_buffer;
}

void Globals::create_noise()
{
	const int size = 256;

	if (noise_data == NULL) {
		noise_data = new Uint8[size*size*4];

		util::srand(0); // make it the same each time

		for (int i = 0; i < size*size*4; i += 4) {
			Uint8 *p = noise_data + i;
			p[0] = util::rand(0, 255);
			p[1] = util::rand(0, 255);
			p[2] = util::rand(0, 255);
			p[3] = util::rand(0, 255);
		}

		util::srand((uint32_t)time(NULL));
	}

	noise = new gfx::Image(noise_data, util::Size<int>(size, size));
}

util::Point<float> Globals::get_onscreen_button_position(Onscreen_Button button)
{
	return util::Point<float>(0.0f, 0.0f);
}

void Globals::mini_pause()
{
	if (instance == NULL || mini_paused || SDL_GetTicks() < next_mini_pause_can_start) {
		return;
	}

	button->play(false);

	mini_paused = true;
	mini_pause_start = SDL_GetTicks();

	audio::pause_sfx(true);

	add_yes_no_gui(GLOBALS->game_t->translate(1014)/* Originally: Quit to title? */, true, false, quit_callback);
}

bool Globals::is_mini_paused()
{
	if (instance == NULL) {
		return false;
	}
	return mini_paused;
}

void Globals::quit(bool yesno)
{
	if (yesno) {
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
	
	mini_paused = false;
	mini_pause_total += SDL_GetTicks() - mini_pause_start;
	next_mini_pause_can_start = SDL_GetTicks() + 100;
	
	audio::pause_sfx(false);
}

Uint32 Globals::get_ticks()
{
	if (instance == NULL) {
		return SDL_GetTicks();
	}
	else if (mini_paused) {
		return mini_pause_start - mini_pause_total;
	}
	else {
		return SDL_GetTicks() - mini_pause_total;
	}
}

void Globals::reload_fonts()
{
	gfx::destroy_fonts();
	delete bold_font;

	gfx::load_fonts();
	shim::font->set_vertex_cache_id(1);
	shim::font->start_batch();

	if (shim::use_hires_font) {
#ifdef USE_TTF
		bold_font = new gfx::TTF("hbold.ttf", 8*int(shim::scale));
#else
		bold_font = new gfx::Pixel_Font("hbold");
#endif
	}
	else {
		bold_font = new gfx::Pixel_Font("bold");
	}
	bold_font->set_vertex_cache_id(2);
	bold_font->start_batch();
}

//--

Globals::Instance::Instance(util::JSON::Node *root)
{
	play_time = 0;
	
	mini_paused = false;
	mini_pause_total = 0;
	next_mini_pause_can_start = 0;

	if (root) {
		util::JSON::Node *cfg = root->find("\"game\"");
		util::JSON::Node *n;
		n = cfg->find("\"gold\"");
		if (n != NULL) {
			gold = json_to_integer(n);
		}
		n = cfg->find("\"step_count\"");
		if (n != NULL) {
			step_count = json_to_integer(n);
		}
		n = cfg->find("\"play_time\"");
		if (n != NULL) {
			play_time = json_to_integer(n);
		}
		n = cfg->find("\"party_following_player\"");
		if (n != NULL) {
			party_following_player = json_to_bool(n);
		}
		n = cfg->find("\"chests_opened\"");
		if (n != NULL) {
			chests_opened = json_to_integer(n);
		}

		n = root->find("\"players\"");

		for (size_t i = 0; i < n->children.size(); i++) {
			stats.push_back(Player_Stats(n->children[i]));
		}

		n = root->find("\"inventory\"");
		if (n != NULL) {
			inventory = Inventory(n);
			inventory.sort();
		}
		
		util::JSON::Node *ms = root->find("\"milestones\"");
		if (ms != NULL) {
			for (size_t i = 0; i < ms->children.size(); i++) {
				util::JSON::Node *node = ms->children[i];
				milestones.push_back(node->value == "true");
			}
		}
	}
	else {
		gold = 0;
		step_count = 0;

		party_following_player = false;

		chests_opened = 0;
	}
	
	play_start = GLOBALS->get_ticks();
}

Globals::Instance::~Instance()
{
	for (size_t i = 0; i < stats.size(); i++) {
		delete stats[i].sprite;
	}
}	

bool Globals::Instance::is_milestone_complete(int milestone)
{
	return milestones[milestone];
}

void Globals::Instance::set_milestone_complete(int milestone, bool complete)
{
	milestones[milestone] = complete;
}

int Globals::Instance::num_milestones()
{
	return (int)milestones.size();
}

int Globals::Instance::get_gold()
{
	return gold;
}

void Globals::Instance::add_gold(int amount)
{
	gold += amount;
	if (gold < 0) {
		gold = 0;
	}
	if (gold > globals->max_gold) {
		gold = globals->max_gold;
	}
}

std::string Globals::Instance::save()
{
	std::string s;
	s += "\"game\": {";
	s += "\"gold\": " + util::itos(get_gold()) + ",";
	s += "\"step_count\": " + util::itos(step_count) + ",";
	s += "\"party_following_player\": " + bool_to_string(party_following_player) + ",";
	s += "\"chests_opened\": " + util::itos(chests_opened) + ",";
	s += "\"play_time\": " + util::itos(play_time);
	s += "},";
	s += "\"players\": [";
	for (size_t i = 0; i < stats.size(); i++) {
		s += stats[i].save();
		if (i < stats.size()-1) {
			s += ",";
		}
	}
	s += "],";
	s += "\"inventory\": [";
	s += inventory.save();
	s += "],";
	s += "\"milestones\": [";
	for (int i = 0; i < num_milestones(); i++) {
		if (is_milestone_complete(i)) {
			s += "true";
		}
		else {
			s += "false";
		}
		if (i < num_milestones()-1) {
			s += ",";
		}
	}
	s += "]";
	return s;
}

void Globals::draw_custom_status(Map_Entity *entity, int status, util::Point<float> draw_pos)
{
}

void Globals::load_translation()
{
	delete game_t;
	delete english_game_t;

	std::string game_t_text = util::load_text(std::string("text/") + language + std::string(".utf8"));
	std::string english_game_t_text = util::load_text("text/English.utf8");

	game_t = new util::Translation(game_t_text);
	english_game_t = new util::Translation(english_game_t_text);

	/*
	// Do this if using a TTF
	entire_translation = game_t->get_entire_translation();

	std::vector<std::string> filenames = shim::cpa->get_all_filenames();

	for (size_t i = 0; i < filenames.size(); i++) {
		util::Tokenizer t(filenames[i], '|');
		std::string s = t.next();
		s = t.next();
		entire_translation += s;
	}
	*/
}

std::string Globals::tag_end()
{
	if (language == "French") {
		return std::string(" : ");
	}
	else {
		return std::string(": ");
	}
}

void Globals::loop()
{
}

}
