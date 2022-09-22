#ifndef INVENTORY_H
#define INVENTORY_H

#include <Nooskewl_Wedge/inventory.h>

const int POTION_HP = 100;
const int POTION_PLUS_HP = 250;
const int POTION_OMEGA_HP = 999;
const int CACTUS_FRUIT_HP = 50;

// 0 is none, defined in Wedge

enum Item_Types
{
	ITEM_POTION = 1,
	ITEM_CURE,
	ITEM_HOLY_WATER,
	ITEM_ELIXIR,
	ITEM_FIRE_SCROLL,
	ITEM_HEAL_SCROLL,
	ITEM_CURE_SCROLL,
	ITEM_BOLT_SCROLL,
	ITEM_APPLE,
	ITEM_BAIT,
	ITEM_POTION_PLUS,
	ITEM_HEAL_PLUS_SCROLL,
	ITEM_FISH,
	ITEM_ICE_SCROLL,
	ITEM_HEAL_OMEGA_SCROLL,
	ITEM_POTION_OMEGA,
	ITEM_CACTUS_FRUIT,
	ITEM_SIZE
};

enum Weapon_Types
{
	WEAPON_AXE = 1,
	WEAPON_SHORT_SWORD,
	WEAPON_GHOUL_SWORD,
	WEAPON_SHANK,
	WEAPON_TOMAHAWK,
	WEAPON_SPEAR,
	WEAPON_LONG_SWORD,
	WEAPON_ICE_SICKLE,
	WEAPON_ONYX_BLADE,
	WEAPON_JADE_SWORD,
	WEAPON_FLAME_SWORD
};

enum Armour_Types
{
	ARMOUR_HELMET = 1,
	ARMOUR_CHAIN_MAIL,
	ARMOUR_RUSTY_CHAIN_MAIL,
	ARMOUR_PLATE_MAIL,
	ARMOUR_ONYX_ARMOUR,
	ARMOUR_JADE_ARMOUR
};

enum Special_Types
{
	SPECIAL_RING1 = 1,
	SPECIAL_RING2,
	SPECIAL_VAMPIRE1,
	SPECIAL_VAMPIRE2,
	SPECIAL_SECOND_CHANCE,
	SPECIAL_RUNE
};

bool is_scroll(int id);
bool scroll_is_white(int id);

bool is_knife(int id);
bool is_sickle(int id);

class Monster_RPG_3_Object_Interface : public wedge::Object_Interface
{
public:
	virtual ~Monster_RPG_3_Object_Interface();

	wedge::Object make_object(wedge::Object_Type type, int id, int quantity);
	int use(wedge::Object object, wedge::Base_Stats *target);
	gfx::Sprite *get_sprite(wedge::Object object);
	wedge::Fixed_Stats get_weapon_stats(int id);
	wedge::Fixed_Stats get_armour_stats(int id);

};

#endif // INVENTORY_H
