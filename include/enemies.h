#ifndef ENEMIES_H
#define ENEMIES_H

#include <Nooskewl_Wedge/battle_enemy.h>
#include <Nooskewl_Wedge/battle_player.h>

class Enemy_Drawing_Hook_Step;

class Monster_RPG_3_Battle_Enemy : public wedge::Battle_Enemy
{
public:
	Monster_RPG_3_Battle_Enemy(std::string name);
	virtual ~Monster_RPG_3_Battle_Enemy();

	virtual void set_position(util::Point<int> position);
	virtual void resize(util::Size<int> new_size);

protected:
	int off_y;
};

class Enemy_Fiddler : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_Fiddler();
	virtual ~Enemy_Fiddler();

	bool start();
	bool take_turn();
	void draw();

	float get_poison_odds();

protected:
	int turn_count;
	gfx::Sprite *bottom;
};

class Enemy_Goo : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_Goo();
	virtual ~Enemy_Goo();

	bool start();
	bool take_turn();

	void set_dead();

protected:
};

class Enemy_MiniGoo : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_MiniGoo();
	virtual ~Enemy_MiniGoo();

	bool start();
	bool take_turn();

protected:
};

class Enemy_Mushroom : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_Mushroom();
	virtual ~Enemy_Mushroom();

	bool start();
	bool take_turn();

	float get_poison_odds();

protected:
};

class Enemy_Bloated : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();

	Enemy_Bloated();
	virtual ~Enemy_Bloated();

	bool start();
	bool take_turn();
	void draw_fore();

protected:
	static const int SPIT_TIME = 1000;

	static int count; // for deleting spit
	static gfx::Image *spit;
	static audio::MML *sound;

	bool turn_started;
	wedge::Battle_Player *spit_target;
	Uint32 spit_start;
	float spit_p;
};

class Enemy_Treant : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_Treant();
	virtual ~Enemy_Treant();

	bool start();
	bool take_turn();
	void draw_fore();

protected:
	static const int ACORN_FALL_TIME = 500;
	static const int ACORN_BOUNCE_TIME = 150;
	static const int ACORN_FINAL_FALL_TIME = 300;
	static const int ACORN_TOTAL_TIME = ACORN_FALL_TIME + ACORN_BOUNCE_TIME + ACORN_FINAL_FALL_TIME;
	static const int INITIAL_ACORN_HEIGHT = -10;

	enum Action_Type {
		NONE,
		ATTACK,
		ACORN
	};

	bool turn_acorn();

	Action_Type action;
	bool casting_acorn;
	gfx::Image *acorn;
	wedge::Battle_Player *acorn_target;
	audio::Sound *acorn_sound;
	audio::Sound *acorn_bounce;
	Uint32 acorn_start_time;
	int acorn_stage; // 0=falling, 1=bouncing, 2=final fall
	float acorn_y;
};

class Enemy_Sludge : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();

	Enemy_Sludge();
	virtual ~Enemy_Sludge();

	bool start();
	bool take_turn();
	void draw_fore();
	
	float get_poison_odds();

protected:
	static const int SLUDGE_TIME = 1000;

	static int count; // for deleting gunk
	static gfx::Sprite *gunk;
	static audio::MML *sound;

	bool turn_started;
	wedge::Battle_Player *sludge_target;
	Uint32 sludge_start;
	float sludge_p;
};

class Enemy_Does_Creepy : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();

	Enemy_Does_Creepy(std::string name);
	virtual ~Enemy_Does_Creepy();

protected:
	static const int CREEP_FULL_TIME = 5000;
	static const int CREEP_TIME = 2000;
	static const int LAUGH_TIME = 500;
	static const int DRIFTS = 20;
	static const int ADD_TIME = 250;
	static const int MAX_DRIFT = 5;
	static const int NUM_CREEPS = (CREEP_FULL_TIME - CREEP_TIME - LAUGH_TIME) / ADD_TIME;
	
	bool turn_creepy();
	util::Point<float> new_drift();
	float creep_p(int index);
	bool creep_visible(int index);
	util::Point<float> creep_pos(int index);
	util::Point<float> get_drift(int index);
	SDL_Colour get_tint(int index);
	void start_creepy();
	void draw_creepy();

	struct Creep {
		wedge::Battle_Entity *target;
		util::Point<float> prev_drift;
		util::Point<float> drift;
		int image_index;
	};

	static int count; // for deleting Creepy assets
	static gfx::Image *creep_images[2];
	static audio::MML *sound;

	std::vector<Creep> creeps;
	Uint32 creepy_start_time;
	Uint32 last_drift;
	std::vector<Battle_Entity *> living;

	float creepy_mult;
};

class Enemy_Ghastly : public Enemy_Does_Creepy
{
public:
	Enemy_Ghastly();
	virtual ~Enemy_Ghastly();

	bool start();
	bool take_turn();
	void draw_fore();
	
protected:
	enum Action_Type {
		NONE,
		ATTACK,
		CREEPY
	};

	Action_Type action;
};

class Enemy_Werewolf : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();

	Enemy_Werewolf();
	virtual ~Enemy_Werewolf();

	bool start();
	bool take_turn();

protected:
	static int count; // for deleting bite sound
	static audio::Sound *bite_sound;

	bool turn_started;
	bool punching;
};

class Enemy_Knightly : public Monster_RPG_3_Battle_Enemy
{
public:
	static const int DEFENSE = 10;
	static const int HEAD_RETURN_TIME = 2000;
	static const int THROW_TIME = 1200;
	
	static void static_start();

	Enemy_Knightly();
	virtual ~Enemy_Knightly();

	bool start();
	bool take_turn();
	void draw();

protected:
	static int count; // for deleting bite sound
	static gfx::Image *head_image;
	static gfx::Sprite *head_spin_sprite;
	static audio::Sound *head_spin_sound;
	static audio::Sound *throw_sound;

	void start_attack();
	void start_throw();
	
	bool has_head;
	bool attacking;
	bool head_returning;
	bool throw_started;
	Uint32 head_return_start;
	Uint32 throw_start;
	Battle_Entity *target;
	util::Point<float> throw_start_pos;
	util::Point<float> throw_end_pos;
};

class Enemy_Palla : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_Palla();
	virtual ~Enemy_Palla();

	bool start();
	bool take_turn();
	void draw_fore();

private:
	static const int THROW_TIME = 1500;
	static const int VAMPIRE_TIME = 2000;
	static const int BAT_TIME = 5000;
	static const int BITE_TIME = 500;

	void set_idle();

	enum Turn_Type {
		NONE = 0,
		VAMPIRE,
		BATS,
		THROW
	};

	struct Bat {
		bool player1;
		Uint32 delay;
		bool bit;
		gfx::Sprite *sprite;
		int start_y;
		int stage;
	};

	Turn_Type turn_type;

	Uint32 turn_start;

	std::vector<Bat> bats;

	wedge::Battle_Entity *target;

	int turn;

	gfx::Sprite *throw_sprite;

	Uint32 throw_start;
	bool hit;

	audio::Sound *throw_sound;
	audio::Sound *vampire_sound;
	audio::Sound *vampire_small_sound;
	audio::Sound *bats_sound;

	gfx::Sprite *vamp_sprite;
	gfx::Sprite *vamp_small_sprite;
};

class Enemy_Tentacle : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_Tentacle();
	virtual ~Enemy_Tentacle();

	bool start();
	bool take_turn();
};

class Enemy_Wave : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_Wave();
	virtual ~Enemy_Wave();

	bool start();
	bool take_turn();
	void draw_back();

protected:
	gfx::Sprite *deck_water;
	audio::Sound *splash;
	bool played_splash;
};

class Enemy_Shocker : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();
	static void spell_effect_callback(void *data);

	Enemy_Shocker();
	virtual ~Enemy_Shocker();

	bool start();
	bool take_turn();
	void draw_back();
	void lost_device();
	void found_device();

	void set_spell_effect_done(bool done);

protected:
	static const int MIN_BLURS = 3;
	static const int MAX_BLURS = 8;

	static int count;
	static gfx::Image **prerendered;

	int get_index();
	void prerender();
	int get_num_prerendered_images();
	void delete_prerendered();
	void allocate_prerendered();

	gfx::Sprite *shocker_ring;

	bool turn_started;

	bool spell_effect_done;
	wedge::Battle_Entity *target;
	
	bool attacking;
};

class Enemy_Monster : public Monster_RPG_3_Battle_Enemy
{
public:
	Enemy_Monster();
	virtual ~Enemy_Monster();

	bool start();
	bool take_turn();
	void draw_fore();
	void draw();

	void set_splash_showing(bool showing);

private:
	static const int CIRCLE_RADIUS = 4;
	static const int WHIRLPOOL_RAISE = 500;
	static const int WHIRLPOOL_STATIONARY = 500;
	static const int WHIRLPOOL_JUMP = 500;
	static const int WHIRLPOOL_CIRCLE = 1000;
	static const int WHIRLPOOL_LEAVE = 500;

	static const int SCREECH_TIME = 3250;
	static const int NUM_WIND = 50;

	enum Turn_Type {
		NONE = 0,
		ATTACK,
		WHIRLPOOL,
		SCREECH
	};

	Turn_Type turn_type;

	Uint32 turn_start;

	int turn;

	wedge::Battle_Player *target;

	audio::Sound *screech;
	audio::Sound *monster_sound;
	audio::Sound *whirlpool_sound;

	gfx::Sprite *whirlpool;
	gfx::Sprite *splash;
	gfx::Sprite *top;

	int splashed;
	int splashed_gfx;

	bool splash_showing;

	struct Wind {
		util::Point<int> start_pos;
		float len;
		float x;
	};

	std::vector<Wind> wind;
};

class Enemy_Sandworm : public Monster_RPG_3_Battle_Enemy
{
public:
	static void hide_done_callback(void *data);
	
	Enemy_Sandworm();
	virtual ~Enemy_Sandworm();

	bool start();
	bool take_turn();

	void set_hide_done(bool done);

protected:
	bool turn_started;
	bool hiding;
	bool hide_done;
	Uint32 hide_start;
	bool played_sound;
};

class Enemy_Flare : public Monster_RPG_3_Battle_Enemy
{
public:
	static void spell_effect_callback(void *data);

	Enemy_Flare();
	virtual ~Enemy_Flare();

	bool start();
	bool take_turn();
	void draw_back();

	void set_spell_effect_done();

protected:
	int spell_effect_done_count;
	std::vector<wedge::Battle_Entity *> targets;
	Uint32 start_ticks;
	gfx::Sprite *bottom;
};

class Enemy_Cyclone : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();

	Enemy_Cyclone();
	virtual ~Enemy_Cyclone();

	bool start();
	bool take_turn();
	void draw();
	void draw_fore();

protected:
	static const int CIRCLE_RADIUS = 4;
	static const int THERE = 750;
	static const int CIRCLE = 1500;
	static const int BACK = 750;
	static const int NUM_PIXELS = 200;
	static const int MIN_DUST_DIST = 5;
	static const int MAX_DUST_DIST = 15;
	static const int MAX_DUST_LIFE = 333;

	static int count; // for playing/deleting loop_sound
	static audio::Sound *loop_sound;

	bool taking_turn;
	Uint32 turn_start;

	struct Pixel {
		float angle;
		float dist;
		Uint32 start_time;
		SDL_Colour colour;
	};

	std::vector<Pixel> pixels;

	wedge::Battle_Entity *target;
	util::Point<float> target_pos;
	util::Point<float> start_pos;
	
	audio::Sound *attack_sound;

	bool played_sound;
	bool did_hit;
};

class Enemy_Bones : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();

	Enemy_Bones();
	virtual ~Enemy_Bones();

	bool start();
	bool take_turn();
	void draw_fore();

	void set_spell_effect_done();

protected:
	static const int RAISE = 500;
	static const int THERE = 500;
	static const int BACK = 500;
	static const int ROTATIONS = 8;

	static int count;
	static gfx::Image *big_bone;
	static gfx::Image *small_bone;
	static audio::Sound *throw_sound;

	bool turn_started;
	Uint32 start_time;
	bool big;
	bool hit;
	bool threw;

	util::Point<float> big_bone_start_pos;
	util::Point<float> small_bone_start_pos;

	wedge::Battle_Entity *target;

	int stage;

	util::Point<float> dest_pos;
};

class Enemy_Does_Darkness
{
public:
	Enemy_Does_Darkness();
	virtual ~Enemy_Does_Darkness();

protected:
	static const unsigned DARKNESS_TIME = 5000;
	static const int PHASES = 5;
	static const int WIDTH = 60;
	static const int PEAKS = 2; // during even phases, -1 during odd phases
	static const int NUM_GHOSTS = 50;
	static const int GHOST_TIME = 1000;

	void start_darkness();
	void draw_darkness();
	bool turn_darkness();

	gfx::Image *ghost;
	audio::Sound *darkness_sfx;

	struct Ghost {
		int y;
		int dist;
		Uint32 delay;
	};

	std::vector<Ghost> ghosts;

	gfx::Sprite *enemy_sprite;
	
	bool taking_turn;
	Uint32 turn_start;

	Monster_RPG_3_Battle_Enemy *enemy;

	float darkness_mult;

	bool finished_cast;
	int cast_time;
};

class Enemy_Reaper : public Monster_RPG_3_Battle_Enemy, public Enemy_Does_Darkness
{
public:
	Enemy_Reaper();
	virtual ~Enemy_Reaper();

	bool start();
	bool take_turn();
	void draw_fore();

protected:
	static const int CAST_TIME = 800;

	bool attacking;
	audio::Sound *cast;
};

class Enemy_Rocky : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();
	static void sprite_callback(void *data);

	Enemy_Rocky();
	virtual ~Enemy_Rocky();

	bool start();
	bool take_turn();
	void draw();
	void draw_fore();

	void set_sprite_done();

protected:
	static const unsigned JUMP_TIME = 750;
	static const int COMPRESS1 = 0;
	static const int JUMP1 = 1;
	static const int COMPRESS2 = 2;
	static const int JUMP2 = 3;
	static const int JUMP_H = 50;

	static int count;
	static audio::Sound *slam;

	bool taking_turn;
	Uint32 turn_start;
	bool attacking;
	wedge::Battle_Entity *target;
	int slam_phase;
	int slam_length;
};

class Enemy_Wraith : public Monster_RPG_3_Battle_Enemy
{
public:
	static void spell_effect_callback(void *data);

	Enemy_Wraith();
	virtual ~Enemy_Wraith();

	bool start();
	bool take_turn();

	void set_spell_effect_done();

protected:
	int spell_effect_done_count;
	wedge::Battle_Entity * target;
	bool taking_turn;
	bool attacking;
	int last_frame;
	int num_hits;
	wedge::Battle_Player *first_target;
	wedge::Battle_Player *second_target;
};

class Enemy_Shadow : public Monster_RPG_3_Battle_Enemy
{
public:
	static void static_start();

	Enemy_Shadow();
	virtual ~Enemy_Shadow();

	bool start();
	bool take_turn();

protected:
	static const int WALK_TIME = 1000;
	static const int HIT_TIME = 500;

	static int count;
	static audio::Sample *footsteps;

	bool taking_turn;
	int hits;
	int hits_done;
	Uint32 turn_start;
	util::Point<float> start_pos;
	util::Point<float> end_pos;
	wedge::Battle_Player *target;
	bool did_attack;
	bool played_footsteps;
	bool played_attack_sound;
	int num_hits;
};

class Enemy_Gayan : public Enemy_Does_Creepy, public Enemy_Does_Darkness
{
public:
	static void attack_callback(void *data);
	static void cry_callback(void *data);

	Enemy_Gayan();
	virtual ~Enemy_Gayan();

	bool start();
	bool take_turn();
	void draw_fore();

	void dec_num_attacks();
	void end_cry();
	void set_idle();

protected:
	bool final_attack;
	bool deadly_attack;

	enum Action_Type {
		CREEPY,
		DARKNESS,
		DARKNESS_PLUS,
		ATTACK
	};

	Action_Type action;

	audio::Sample *cry_big;

	int num_attacks;
	int prev_num_attacks;

	int turn_count;

	Uint32 cry_start;
	bool cry_ended;

	gfx::Image *buffer;
	util::Size<int> buffer_size;
	Uint32 darkness_plus_start;
	audio::MML *darkness_plus_sound;

	int num_hits[2];
	wedge::Battle_Player *attacked;

	int draw_count;

	Enemy_Drawing_Hook_Step *drawing_hook;
};

#endif // ENEMIES_H
