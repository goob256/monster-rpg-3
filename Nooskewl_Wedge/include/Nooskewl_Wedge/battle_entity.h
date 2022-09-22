#ifndef NOOSKEWL_WEDGE_BATTLE_ENTITY_H
#define NOOSKEWL_WEDGE_BATTLE_ENTITY_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/stats.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Battle_Entity
{
public:
	enum Type
	{
		PLAYER = 0,
		ENEMY
	};

	Battle_Entity(Type type, std::string name);
	virtual ~Battle_Entity();

	virtual bool start();
	virtual void handle_event(TGUI_Event *event);
	virtual void draw_back();
	virtual void draw();
	virtual void draw_fore();
	virtual void run();
	virtual void lost_device();
	virtual void found_device();
	virtual void resize(util::Size<int> new_size); // screen size change

	virtual bool take_turn(); // return true when finished, can be called over many frames
	virtual bool is_dead();
	virtual void take_damage(int hp, int type, int y_offset = 0);

	Type get_type();
	std::string get_name();
	Base_Stats *get_stats();
	gfx::Sprite *get_sprite();

	bool is_defending();

	util::Point<int> get_decoration_offset(int decoration_width, util::Point<int> offset, int *flags);

	virtual float get_poison_odds(); // 0-1

	util::Point<float> get_spell_effect_offset();

protected:
	Type type;
	std::string name;
	Base_Stats *stats;
	gfx::Sprite *sprite;
	bool defending;
	util::Point<float> spell_effect_offset;
};

}

#endif // NOOSKEWL_WEDGE_BATTLE_ENTITY_H
