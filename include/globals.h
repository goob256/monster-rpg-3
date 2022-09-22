#ifndef GLOBALS_H
#define GLOBALS_H

#include <Nooskewl_Wedge/globals.h>

const int ENY = 0;
const int TIGGY = 1;

const int NUM_CATCHES = 4; // num fish/other things you can catch

#define M3_GLOBALS static_cast<Monster_RPG_3_Globals *>(wedge::globals)
#define M3_INSTANCE static_cast<Monster_RPG_3_Globals::Instance *>(M3_GLOBALS->instance)
#define M3_BATTLE static_cast<Monster_RPG_3_Battle_Game *>(wedge::globals->battle_game)

class Monster_RPG_3_Globals : public wedge::Globals
{
public:
	static const int MAX_BATTLE_STEPS = 50;
	static const int MIN_BATTLE_STEPS = 25;

	Monster_RPG_3_Globals();
	virtual ~Monster_RPG_3_Globals();

	void do_dialogue(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Step *monitor);
	bool dialogue_active(wedge::Game *game, bool only_if_initialised = false);

	void add_yes_no_gui(std::string text, bool escape_cancels, bool selected, util::Callback callback = 0, void *callback_data = 0);
	void set_darken_screen_on_next_dialogue(bool darken);
	bool add_title_gui();
	util::Point<float> get_onscreen_button_position(wedge::Onscreen_Button button);
	void draw_custom_status(wedge::Map_Entity *entity, int status, util::Point<float> draw_pos);
	bool can_walk();
	bool title_gui_is_top();

	class Instance : public wedge::Globals::Instance
	{
	public:
		Instance(util::JSON::Node *root);
		virtual ~Instance();

		std::string save();
	
		std::vector<std::string> get_vampires();
		void add_vampire(std::string name, bool push_back = false);
		std::string vampire(int index);
		int num_vampires();

		bool boatin;
		util::Point<int> boatin_pos;
		wedge::Direction boatin_direction;
		bool boat_w;
		bool boatin_done;

		int fish_caught;

		int lucky_hits;

	private:
		std::vector<std::string> vampires;
	};

	audio::Sound *melee;
	audio::Sound *run;
	audio::Sound *sleep;
	audio::Sound *buysell;
	audio::Sound *fire;
	audio::Sound *jump;
	audio::Sound *second_chance;
	audio::Sound *cast_line;
	audio::Sound *splash;
	audio::Sound *blind;
	audio::Sound *pendants;
	audio::Sound *grow;

	audio::Sample *vbolt_sample;
	audio::Sample *wind1;
	audio::Sample *wind2;
	audio::Sample *wind3;
	audio::Sample *vice_sample;
	audio::Sample *cry;

	gfx::Image *up_arrow;
	gfx::Image *down_arrow;
	gfx::Image *selection_arrow;
	gfx::Image *nomore;
	gfx::Image *mini_profile_images[2];
	gfx::Image *profile_images[2];
	gfx::Image *play_pause;
	gfx::Image *bottom_shadow;

	gfx::Sprite *blind_sprite;
	
	gfx::Shader *alpha_shader;
	gfx::Shader *bander_shader;
	gfx::Shader *darken_shader;
	gfx::Shader *darken_textured_shader;
	gfx::Shader *darken_both_shader;
	gfx::Shader *darkness_shader;
	gfx::Shader *solid_colour_shader;
	gfx::Shader *blur_shader;
	gfx::Shader *heat_wave_shader;
	gfx::Shader *lit_3d_shader;
	gfx::Shader *alpha_test_shader;

	gfx::Model *zeus;
	gfx::Model *icicle;
	gfx::Model *dragon;

	int save_slot;

	int key_b4;
	int joy_xbox_l;
	int joy_xbox_r;
	int joy_xbox_u;
	int joy_xbox_d;
	bool tv_safe_mode;
	bool hide_onscreen_settings_button;

	bool darken_screen_on_next_dialogue;

	util::Point<int> pre_boss_boatin_pos;
	
	int saved_min_battle_steps;
	int saved_max_battle_steps;

	bool loaded;
};

#endif // GLOBALS_H
