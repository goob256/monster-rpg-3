#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/inventory.h>
#include <Nooskewl_Wedge/stats.h>

#include "inventory.h"

Object_Interface::~Object_Interface()
{
}

wedge::Object Object_Interface::make_object(wedge::Object_Type type, int id, int quantity)
{
	wedge::Object o;

	o.type = type;
	o.id = id;
	o.quantity = quantity;
	o.name = "";
	o.sell_price = 0;

	if (type == wedge::OBJECT_ITEM) {
		if (id == ITEM_POTION) {
			o.name = TRANSLATE("Potion")END;
		}
	}
	
	if (type == wedge::OBJECT_ITEM) {
		if (id == ITEM_POTION) {
			o.sell_price = 5;
		}
	}

	return o;
}

int Object_Interface::use(wedge::Object object, wedge::Base_Stats *target)
{
	int amount = 0;

	if (object.type == wedge::OBJECT_ITEM) {
		if (object.id == ITEM_POTION) {
			if (target->hp > 0) {
				target->hp = MIN(target->fixed.max_hp, target->hp + 100);
				amount = 100;
			}
		}
		if (wedge::globals->item_sfx[object.id] != NULL) {
			wedge::globals->item_sfx[object.id]->play(false);
		}
	}

	return amount;
}
