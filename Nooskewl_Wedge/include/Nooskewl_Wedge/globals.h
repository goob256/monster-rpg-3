// Search this file for --, everything there needs done in a subclass of these classes. The game
// then allocates wedge::globals and wedge::globals->instance itself.

#ifndef NOOSKEWL_WEDGE_GLOBALS_H
#define NOOSKEWL_WEDGE_GLOBALS_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/inventory.h"
#include "Nooskewl_Wedge/onscreen_controller.h"
#include "Nooskewl_Wedge/stats.h"
#include "Nooskewl_Wedge/systems.h"

#define MAX_PARTY wedge::globals->instance->stats.size()

#define GLOBALS wedge::globals
#define INSTANCE wedge::globals->instance
#define SPELLS wedge::globals->spell_interface
#define OBJECT wedge::globals->object_interface

#define AREA wedge::globals->area_game
#define BATTLE wedge::globals->battle_game
#define MENU wedge::globals->menu_game
#define OMNIPRESENT wedge::globals->omnipresent_game
#define SHOP wedge::globals->shop_game

#define TAG_END GLOBALS->tag_end()
#define NEW_PARAGRAPH std::string("^")

#define GET_TICKS() wedge::globals->get_ticks()

#define WS_MIXED -1
#define WS_UNDEFINED 0
#define WS_WEAK 1
#define WS_STRONG 2
#define WS_SET_TARGET_NAME() if (target_name != "" && target_stats->get_name() != target_name) target_name = "-"; else target_name = target_stats->get_name();
#define WS_SET_WEAK_STRONG_TYPE(t) if (weak_strong_type != WS_UNDEFINED && t != weak_strong_type) weak_strong_type = WS_MIXED; else weak_strong_type = t;
#define WEAK_STRONG(t) WS_SET_TARGET_NAME() WS_SET_WEAK_STRONG_TYPE(t)
#define USE_WEAK_STRONG \
	std::string target_name; \
	int weak_strong_type = WS_UNDEFINED;
#define WEAK_STRONG_NOTIFICATION\
	if (target_name != "" && weak_strong_type != WS_UNDEFINED) {\
		std::string message;\
		if (weak_strong_type == WS_MIXED) {\
			message = GLOBALS->game_t->translate(1733);\
		}\
		else {\
			if (target_name == "-") {\
				if (weak_strong_type == WS_WEAK) {\
					message = GLOBALS->game_t->translate(1734);\
				}\
				else {\
					message = GLOBALS->game_t->translate(1735);\
				}\
			}\
			else {\
				if (weak_strong_type == WS_WEAK) {\
					message = target_name + " " + GLOBALS->game_t->translate(1736);\
				}\
				else {\
					message = target_name + " " + GLOBALS->game_t->translate(1737);\
				}\
			}\
		}\
		gfx::add_notification(message);\
	}


namespace wedge {

class Area_Game;
class Battle_Game;
class Game;
class Map_Entity;
class Omnipresent_Game;
class Spell_Interface;

class Yes_No_GUI : public gui::GUI
{
public:
	struct Callback_Data {
		bool choice;
		bool cancelled;
		void *userdata;
	};
};

enum Direction {
	DIR_NONE = 0,
	DIR_N,
	DIR_E,
	DIR_S,
	DIR_W
};

enum Dialogue_Type {
	DIALOGUE_SPEECH,
	DIALOGUE_MESSAGE
};

enum Dialogue_Position {
	DIALOGUE_AUTO = 0,
	DIALOGUE_TOP,
	DIALOGUE_BOTTOM
};

class NOOSKEWL_WEDGE_EXPORT Globals
{
public:
	class Instance;

	struct Level {
		int experience; // gain this level at this experience amount (absolute)
		Fixed_Stats stat_increases;
	};

//-- games must allocate/create/populate the following variables up until the next comment by subclassing these classes --

	virtual bool add_title_gui() = 0; // returns false to exit the game
	virtual void do_dialogue(std::string tag, std::string text, Dialogue_Type type, Dialogue_Position position, Step *monitor) = 0;
	virtual bool dialogue_active(Game *game, bool only_if_initialised = false) = 0;
	virtual void add_yes_no_gui(std::string text, bool escape_cancels, bool selected, util::Callback callback = 0, void *callback_data = 0) = 0;
	virtual util::Point<float> get_onscreen_button_position(Onscreen_Button button); // Must define this if using onscreen controller
	virtual void draw_custom_status(Map_Entity *entity, int status, util::Point<float> draw_pos);
	virtual bool can_walk() = 0; // used to show onscreen controller
	virtual bool title_gui_is_top() = 0;
	virtual void loop();

	// these ones have defaults
	//--
	int min_battle_steps;
	int max_battle_steps;
	int max_gold;
	int gameover_timeout;
	int gameover_fade_time;
	//--
	std::vector< util::Point<int> > player_start_positions;
	std::vector<Direction> player_start_directions;
	std::vector<Level> levels;
	std::vector<audio::Sound *> item_sfx;
	std::map<std::string, audio::Sound *> spell_sfx;
	Spell_Interface *spell_interface;
	Object_Interface *object_interface;
	SDL_Colour red_triangle_colour;
	SDL_Colour gameover_fade_colour;

	Instance *instance; // single session-related info. this must be allocated by your game (likely your title screen buttons)

	std::string boss_save; // save here to offer retrying boss battles
	wedge::Direction boss_press; // this key will be pressed immediately after loading boss save
	bool retry_boss; // if true, title menu will immediately load boss_save above
	bool retried_boss;
	bool terminate;

	class NOOSKEWL_WEDGE_EXPORT Instance
	{
	protected:
		std::vector<bool> milestones;
	public:
		std::vector<Player_Stats> stats;

//-- everything from here on out is implemented/allocated/created in these base classes. make sure you call the superclass methods from your own --

		Instance(util::JSON::Node *root);
		virtual ~Instance();

		virtual std::string save();

		bool is_milestone_complete(int milestone);
		void set_milestone_complete(int milestone, bool complete);
		int num_milestones();

		int get_gold();
		void add_gold(int amount); // clamps to max_gold, negative amounts clamp to 0

		Inventory inventory;
		int step_count;
		std::map<std::string, std::string> saved_levels;
		
		Uint32 play_time;
		Uint32 play_start;

		bool party_following_player;

		int chests_opened;
		
	protected:
		int gold;
	};

	void add_next_dialogue_monitor(wedge::Step *monitor);
	void create_work_image();
	void create_noise();
	void mini_pause();
	bool is_mini_paused();
	void quit(bool yesno);
	Uint32 get_ticks();
	void load_translation();
	std::string tag_end();

	void reload_fonts();
	
	Globals();
	virtual ~Globals();

	// gfx
	gfx::Sprite *poison_sprite;
	gfx::Sprite *dpad;
#ifdef _WIN32
	gfx::Shader::D3D_Vertex_Shader *default_d3d_vertex_shader;
#endif
	gfx::Shader::OpenGL_Shader *default_opengl_vertex_shader;
	gfx::Shader *enemy_die_shader;
	gfx::Shader *boss_die_shader;
	gfx::Image *work_image; // always a shim::real_screen_size image with RTT
	gfx::Image *noise;
	
	gfx::Font *bold_font;
	
	// sound
	audio::Sound *chest;
	audio::Sound *battle_start;
	audio::Sound *hit;
	audio::Sound *enemy_die;
	audio::Sound *boss_die;
	audio::Sound *enemy_attack;
	audio::Sound *poison;
	audio::Sound *levelup;
	audio::MML *button;
	audio::MML *gameover;

	// these are the main sections of the game, NULL if not available right now
	Area_Game *area_game;
	Battle_Game *battle_game;
	Omnipresent_Game *omnipresent_game;
	Game *menu_game;
	Game *shop_game;

	int key_b1;
	int key_b2;
	int key_b3;
	int key_switch;
	int key_l;
	int key_r;
	int key_u;
	int key_d;
	int joy_b1;
	int joy_b2;
	int joy_b3;
	int joy_b4;
	int joy_switch;

	float onscreen_controls_alpha;

	bool rumble_enabled;

	Uint8 *noise_data;

	Game *temp_battle;
	Game *temp_area;
	Game *temp_menu;
	Game *temp_shop;
	audio::MML *saved_music;

	util::Translation *game_t;
	util::Translation *english_game_t;
	std::string entire_translation;
	std::string language;

	bool onscreen_controller_was_enabled;
	bool onscreen_controller_temporarily_disabled;

#ifdef ANDROID
	bool have_highp_fragment_shaders;
#endif

protected:
	std::vector<wedge::Step *> next_dialogue_monitors;
};

extern Globals NOOSKEWL_WEDGE_EXPORT *globals;

}

#endif // NOOSKEWL_WEDGE_GLOBALS_H
