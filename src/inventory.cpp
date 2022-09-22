#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/inventory.h>
#include <Nooskewl_Wedge/stats.h>

#include "achievements.h"
#include "globals.h"
#include "gui.h"
#include "inventory.h"
#include "stats.h"

Monster_RPG_3_Object_Interface::~Monster_RPG_3_Object_Interface()
{
}

wedge::Object Monster_RPG_3_Object_Interface::make_object(wedge::Object_Type type, int id, int quantity)
{
	wedge::Object o;

	o.type = type;
	o.id = id;
	o.quantity = quantity;
	o.name = "";
	o.sell_price = 0;
	o.description = "Unknown...";

	if (type == wedge::OBJECT_ITEM) {
		if (id == ITEM_POTION) {
			o.name = "Potion";
			o.sell_price = 5;
			o.description = "A small vial.$Heals 100 HP.";
		}
		else if (id == ITEM_CURE) {
			o.name = "Cure";
			o.sell_price = 8;
			o.description = "Special tonic.$Cures status ailments.";
		}
		else if (id == ITEM_HOLY_WATER) {
			o.name = "Holy Water";
			o.sell_price = 25;
			o.description = "Blessed water.$Revives KO'd party member.";
		}
		else if (id == ITEM_ELIXIR) {
			o.name = "Elixir";
			o.sell_price = 25;
			o.description = "A potent mix.$Restores MP.";
		}
		else if (id == ITEM_FIRE_SCROLL) {
			o.name = "Fire Scroll";
			o.sell_price = 0;
			o.description = "A dusty Magic Scroll.";
		}
		else if (id == ITEM_BOLT_SCROLL) {
			o.name = "Bolt Scroll";
			o.sell_price = 0;
			o.description = "A dusty Magic Scroll.";
		}
		else if (id == ITEM_HEAL_SCROLL) {
			o.name = "Heal Scroll";
			o.sell_price = 0;
			o.description = "A dusty Magic Scroll.";
		}
		else if (id == ITEM_CURE_SCROLL) {
			o.name = "Cure Scroll";
			o.sell_price = 0;
			o.description = "A dusty Magic Scroll.";
		}
		else if (id == ITEM_APPLE) {
			o.name = "Apple";
			o.sell_price = 20;
			o.description = "Fresh fruit.$Heals all HP.";
		}
		else if (id == ITEM_BAIT) {
			o.name = "Bait";
			o.sell_price = 1;
			o.description = "A large nightcrawler.";
		}
		if (id == ITEM_POTION_PLUS) {
			o.name = "Potion Plus";
			o.sell_price = 25;
			o.description = "A flask full of liquid.$Heals 250 HP.";
		}
		else if (id == ITEM_HEAL_PLUS_SCROLL) {
			o.name = "Heal Plus Scroll";
			o.sell_price = 0;
			o.description = "A dusty Magic Scroll.";
		}
		else if (id == ITEM_FISH) {
			o.name = "Fish";
			o.sell_price = 50;
			o.description = "A small tuna.$Restores all HP and MP.";
		}
		else if (id == ITEM_ICE_SCROLL) {
			o.name = "Ice Scroll";
			o.sell_price = 0;
			o.description = "A dusty Magic Scroll.";
		}
		else if (id == ITEM_HEAL_OMEGA_SCROLL) {
			o.name = "Heal Omega Scroll";
			o.sell_price = 0;
			o.description = "A dusty Magic Scroll.";
		}
		if (id == ITEM_POTION_OMEGA) {
			o.name = "Potion Omega";
			o.sell_price = 50;
			o.description = "A jug of glowing fluid.$Heals 999 HP.";
		}
		if (id == ITEM_CACTUS_FRUIT) {
			o.name = "Cactus Fruit";
			o.sell_price = 5;
			o.description = "Yum! Or not!";
		}
	}
	else if (type == wedge::OBJECT_WEAPON) {
		if (id == WEAPON_AXE) {
			o.name = "Axe";
			o.sell_price = 20;
			o.description = "A blunt axe.$Good for chopping wood.";
		}
		else if (id == WEAPON_SHORT_SWORD) {
			o.name = "Short Sword";
			o.sell_price = 25;
			o.description = "A small blade.$Could be a child's.";
		}
		else if (id == WEAPON_GHOUL_SWORD) {
			o.name = "Ghoul Sword";
			o.sell_price = 40;
			o.description = "A green blade.$Was once a Ghoul's.";
		}
		else if (id == WEAPON_SHANK) {
			o.name = "Shank";
			o.sell_price = 50;
			o.description = "A sharpened spoon.";
		}
		else if (id == WEAPON_TOMAHAWK) {
			o.name = "Tomahawk";
			o.sell_price = 75;
			o.description = "A sharp axe.$Nice find!";
		}
		else if (id == WEAPON_SPEAR) {
			o.name = "Spear";
			o.sell_price = 400;
			o.description = "A barbed fishing spear.";
		}
		else if (id == WEAPON_LONG_SWORD) {
			o.name = "Long Sword";
			o.sell_price = 500;
			o.description = "A long blade.$Belonged to a Knight.";
		}
		else if (id == WEAPON_ICE_SICKLE) {
			o.name = "Ice Sickle";
			o.sell_price = 750;
			o.description = "Icy and sharp.$Never melts.";
		}
		else if (id == WEAPON_ONYX_BLADE) {
			o.name = "Onyx Blade";
			o.sell_price = 1500;
			o.description = "A sharp, dark blade.$Made from mountain rock.";
		}
		else if (id == WEAPON_JADE_SWORD) {
			o.name = "Jade Sword";
			o.sell_price = 2000;
			o.description = "A shiny green weapon.$A real gem.";
		}
		else if (id == WEAPON_FLAME_SWORD) {
			o.name = "Flame Sword";
			o.sell_price = 5000;
			o.description = "A molten blade.$Burns continuously.";
		}
	}
	else if (type == wedge::OBJECT_ARMOUR) {
		if (id == ARMOUR_HELMET) {
			o.name = "Helmet";
			o.sell_price = 25;
			o.description = "A bowl-like helmet for a big head.";
		}
		else if (id == ARMOUR_CHAIN_MAIL) {
			o.name = "Chain Mail";
			o.sell_price = 50;
			o.description = "Chain mail.$Never used.";
		}
		else if (id == ARMOUR_RUSTY_CHAIN_MAIL) {
			o.name = "Rusty Mail";
			o.sell_price = 35;
			o.description = "Rusty chain mail.$Good enough.";
		}
		else if (id == ARMOUR_PLATE_MAIL) {
			o.name = "Plate Mail";
			o.sell_price = 250;
			o.description = "Tough plate mail.$Knight's insignia.";
		}
		else if (id == ARMOUR_ONYX_ARMOUR) {
			o.name = "Onyx Armour";
			o.sell_price = 1000;
			o.description = "Tough black armour.$Nearly impenetrable.";
		}
		else if (id == ARMOUR_JADE_ARMOUR) {
			o.name = "Jade Armour";
			o.sell_price = 2000;
			o.description = "Rare armour.$Light and strong.";
		}
	}
	else if (type == wedge::OBJECT_SPECIAL) {
		o.sell_price = 0;
		if (id == SPECIAL_RING1) {
			o.name = "Ring";
			o.description = "A silver ring.$Engraved with initials.";
		}
		else if (id == SPECIAL_RING2) {
			o.name = "Ring";
			o.description = "A silver ring.$Engraved with initials.";
		}
		else if (id == SPECIAL_VAMPIRE1) {
			o.name = "Vampire Pendant";
			o.description = "A jeweled medallion.$Shaped like a bat.";
		}
		else if (id == SPECIAL_VAMPIRE2) {
			o.name = "Vampire Pendant";
			o.description = "A jeweled medallion.$Shaped like a bat.";
		}
		else if (id == SPECIAL_SECOND_CHANCE) {
			o.name = "Second Chance";
			o.description = "Raises the dead.$Rare indeed!";
		}
		else if (id == SPECIAL_RUNE) {
			o.name = "Rune";
			o.description = "NNWNEEN";
		}
	}

	return o;
}

static void learn_spell(std::string spell, wedge::Base_Stats *target)
{
	GLOBALS->levelup->play(false);
	target->add_spell(spell);
	GLOBALS->do_dialogue("", util::string_printf(GLOBALS->game_t->translate(1727)/* Originally: Learned %s! */.c_str(), GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(spell)).c_str()), wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_BOTTOM, NULL);
}

int Monster_RPG_3_Object_Interface::use(wedge::Object object, wedge::Base_Stats *target)
{
	int amount = 0;

	int index = INSTANCE->inventory.find(object);

	if (object.type == wedge::OBJECT_NONE) {
		util::errormsg("Attempt to use wedge::OBJECT_NONE index = %d!\n", index);
	}
	else if (object.type == wedge::OBJECT_ITEM) {
		if (object.id == ITEM_POTION) {
			if (target->hp > 0) {
				target->hp = MIN(target->fixed.max_hp, target->hp + POTION_HP);
				amount = POTION_HP;
			}
		}
		else if (object.id == ITEM_CURE) {
			target->status = wedge::STATUS_OK;
		}
		else if (object.id == ITEM_HOLY_WATER) {
			target->hp = target->fixed.max_hp;
		}
		else if (object.id == ITEM_ELIXIR) {
			target->mp = target->fixed.max_mp;
		}
		else if (object.id == ITEM_HEAL_SCROLL) {
			learn_spell("Heal", target);
		}
		else if (object.id == ITEM_FIRE_SCROLL) {
			learn_spell("Fire", target);
		}
		else if (object.id == ITEM_BOLT_SCROLL) {
			learn_spell("Bolt", target);
		}
		else if (object.id == ITEM_CURE_SCROLL) {
			learn_spell("Cure", target);
		}
		else if (object.id == ITEM_APPLE) {
			if (target->hp > 0) {
				target->hp = target->fixed.max_hp;
				amount = target->fixed.max_hp;
			}
		}
		else if (object.id == ITEM_BAIT) {
			if (target->hp > 0) {
				target->hp = MIN(target->fixed.max_hp, target->hp + 1);
				amount = 1;
			}
		}
		else if (object.id == ITEM_POTION_PLUS) {
			if (target->hp > 0) {
				target->hp = MIN(target->fixed.max_hp, target->hp + POTION_PLUS_HP);
				amount = POTION_PLUS_HP;
			}
		}
		else if (object.id == ITEM_HEAL_PLUS_SCROLL) {
			learn_spell("Heal Plus", target);
		}
		else if (object.id == ITEM_FISH) {
			if (target->hp > 0) {
				target->hp = target->fixed.max_hp;
				target->mp = target->fixed.max_mp;
				amount = target->fixed.max_hp;
			}
		}
		else if (object.id == ITEM_ICE_SCROLL) {
			learn_spell("Ice", target);
		}
		else if (object.id == ITEM_HEAL_OMEGA_SCROLL) {
			learn_spell("Heal Omega", target);
		}
		else if (object.id == ITEM_POTION_OMEGA) {
			if (target->hp > 0) {
				target->hp = MIN(target->fixed.max_hp, target->hp + POTION_OMEGA_HP);
				amount = POTION_OMEGA_HP;
			}
		}
		else if (object.id == ITEM_CACTUS_FRUIT) {
			if (util::rand(0, 3) == 0) {
				util::achieve((void *)ACHIEVE_CACTUS);
				if (target->hp > 0) {
					target->hp = MAX(1, target->hp - CACTUS_FRUIT_HP); // never kills you
					amount = -CACTUS_FRUIT_HP;
				}
				GLOBALS->poison->play(false);
			}
			else {
				if (target->hp > 0) {
					target->hp = MIN(target->fixed.max_hp, target->hp + CACTUS_FRUIT_HP);
					amount = CACTUS_FRUIT_HP;
				}
				wedge::globals->item_sfx[ITEM_POTION]->play(false);
			}
		}
		else {
			util::errormsg("Object %d has type %d but item type is ITEM_NONE!\n", index, object.id);
		}

		if (object.id != ITEM_CACTUS_FRUIT && wedge::globals->item_sfx[object.id] != NULL) { // cactus fruit plays sound above
			wedge::globals->item_sfx[object.id]->play(false);
		}
	}

	return amount;
}

gfx::Sprite * Monster_RPG_3_Object_Interface::get_sprite(wedge::Object object)
{
	if (object.type == wedge::OBJECT_WEAPON) {
		if (object.id == WEAPON_AXE) {
			return new gfx::Sprite("axe");
		}
		else if (object.id == WEAPON_SHORT_SWORD) {
			return new gfx::Sprite("short_sword");
		}
		else if (object.id == WEAPON_GHOUL_SWORD) {
			return new gfx::Sprite("ghoul_sword");
		}
		else if (object.id == WEAPON_SHANK) {
			return new gfx::Sprite("shank");
		}
		else if (object.id == WEAPON_TOMAHAWK) {
			return new gfx::Sprite("tomahawk");
		}
		else if (object.id == WEAPON_SPEAR) {
			return new gfx::Sprite("spear");
		}
		else if (object.id == WEAPON_LONG_SWORD) {
			return new gfx::Sprite("long_sword");
		}
		else if (object.id == WEAPON_ICE_SICKLE) {
			return new gfx::Sprite("ice_sickle");
		}
		else if (object.id == WEAPON_ONYX_BLADE) {
			return new gfx::Sprite("onyx_blade");
		}
		else if (object.id == WEAPON_JADE_SWORD) {
			return new gfx::Sprite("jade_sword");
		}
		else if (object.id == WEAPON_FLAME_SWORD) {
			return new gfx::Sprite("flame_sword");
		}
	}

	return NULL;
}

wedge::Fixed_Stats Monster_RPG_3_Object_Interface::get_weapon_stats(int id)
{
	wedge::Fixed_Stats stats;

	stats.set_extra(LUCK, 0);

	if (id == WEAPON_AXE) {
		stats.attack = 5;
		stats.strength = WEAK_STRONG_FIRE; // Make axes tough against Treants/"earth" enemies
	}
	else if (id == WEAPON_SHORT_SWORD) {
		stats.attack = 7;
	}
	else if (id == WEAPON_GHOUL_SWORD) {
		stats.attack = 10;
	}
	else if (id == WEAPON_SHANK) {
		stats.attack = 12;
	}
	else if (id == WEAPON_TOMAHAWK) {
		stats.attack = 20;
		stats.strength = WEAK_STRONG_FIRE; // Make axes tough against Treants/"earth" enemies
	}
	else if (id == WEAPON_SPEAR) {
		stats.attack = 60;
	}
	else if (id == WEAPON_LONG_SWORD) {
		stats.attack = 150;
	}
	else if (id == WEAPON_ICE_SICKLE) {
		stats.attack = 200;
		stats.strength = WEAK_STRONG_ICE;
	}
	else if (id == WEAPON_ONYX_BLADE) {
		stats.attack = 300;
	}
	else if (id == WEAPON_JADE_SWORD) {
		stats.attack = 300;
		stats.set_extra(LUCK, 10);
	}
	else if (id == WEAPON_FLAME_SWORD) {
		stats.attack = 500;
		stats.set_extra(LUCK, 10);
		stats.strength = WEAK_STRONG_FIRE;
	}

	return stats;
}

wedge::Fixed_Stats Monster_RPG_3_Object_Interface::get_armour_stats(int id)
{
	wedge::Fixed_Stats stats;

	stats.set_extra(LUCK, 0);

	if (id == ARMOUR_HELMET) {
		stats.defense = 5;
	}
	else if (id == ARMOUR_CHAIN_MAIL) {
		stats.defense = 10;
	}
	else if (id == ARMOUR_RUSTY_CHAIN_MAIL) {
		stats.defense = 7;
	}
	else if (id == ARMOUR_PLATE_MAIL) {
		stats.defense = 50;
	}
	else if (id == ARMOUR_ONYX_ARMOUR) {
		stats.defense = 100;
	}
	else if (id == ARMOUR_JADE_ARMOUR) {
		stats.defense = 125;
	}

	return stats;
}

bool is_scroll(int id)
{
	if (
		id == ITEM_HEAL_SCROLL ||
		id == ITEM_FIRE_SCROLL ||
		id == ITEM_CURE_SCROLL ||
		id == ITEM_BOLT_SCROLL ||
		id == ITEM_HEAL_PLUS_SCROLL ||
		id == ITEM_ICE_SCROLL ||
		id == ITEM_HEAL_OMEGA_SCROLL
	) {
		return true;
	}
	return false;
}

bool scroll_is_white(int id)
{
	if (
		id == ITEM_HEAL_SCROLL ||
		id == ITEM_CURE_SCROLL ||
		id == ITEM_HEAL_PLUS_SCROLL ||
		id == ITEM_HEAL_OMEGA_SCROLL
	) {
		return true;
	}
	return false;
}

bool is_knife(int id)
{
	if (
		id == WEAPON_SHANK ||
		id == WEAPON_SPEAR
	) {
		return true;
	}
	return false;
}

bool is_sickle(int id)
{
	if (
		id == WEAPON_ICE_SICKLE
	) {
		return true;
	}
	return false;
}
