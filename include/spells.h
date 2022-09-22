#ifndef SPELLS_H
#define SPELLS_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/battle_entity.h>
#include <Nooskewl_Wedge/spells.h>
#include <Nooskewl_Wedge/stats.h>

class Monster_RPG_3_Spell_Interface : public wedge::Spell_Interface
{
public:
	virtual ~Monster_RPG_3_Spell_Interface();

	bool is_white_magic(std::string name);
	bool can_cast_in_menus(std::string name);
	bool can_multi(std::string name);
	int get_cost(std::string name);
	std::vector<int> use(std::string name, std::vector<wedge::Base_Stats *> stats, float scale = 1.0f);
	void play_sound(std::string name);
	std::vector<wedge::Step *> start_effect(std::string spell, std::vector<wedge::Battle_Entity *> targets, util::Callback callback, void *callback_data);
};

#endif // SPELLS_H
