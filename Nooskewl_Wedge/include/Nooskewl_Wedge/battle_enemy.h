#ifndef NOOSKEWL_WEDGE_BATTLE_ENEMY_H
#define NOOSKEWL_WEDGE_BATTLE_ENEMY_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/battle_entity.h"

namespace wedge {

class Base_Stats;
class Battle_Player;

class NOOSKEWL_WEDGE_EXPORT Battle_Enemy : public Battle_Entity
{
public:
	Battle_Enemy(std::string name);
	virtual ~Battle_Enemy();

	virtual bool start();
	virtual void draw();
	virtual bool take_turn(); // return true when finished, can be called over many frames
	virtual bool is_dead();
	virtual void take_damage(int hp, int type, int y_offset = 0);
	virtual void set_dead();

	int get_experience();
	int get_gold();

	void sprite_done();

	virtual void set_position(util::Point<int> position);
	util::Point<int> get_position();

	void play_die_sound();

	Battle_Player *rand_living_player();

	void draw_enemy(SDL_Colour tint, gfx::Image *image, util::Point<float> draw_pos, float scale);
	void draw_boss(SDL_Colour tint, gfx::Image *image, util::Point<float> draw_pos, float scale, int curr_frame, int num_frames);

	bool remove_when_dead();
	bool use_death_sound();

protected:
	bool turn_attack(Battle_Player *attacked, int y_offset = 0);
	bool turn_attack();
	
	int die_time;

	int experience;
	int gold;
	audio::MML *attack_sound;
	audio::MML *die_sound;
	util::Point<float> position;
	Uint32 dead_start;
	bool started_attack;
	bool done_sprite;
	Battle_Player *attacked;
	bool attack_hits_all;
	bool boss;
	bool use_death_shader;
	bool _use_death_sound;
	bool _remove_when_dead;
	bool play_attack_sound;
};

}

#endif // NOOSKEWL_WEDGE_BATTLE_ENEMY_H
