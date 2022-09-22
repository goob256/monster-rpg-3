#include <locale>

#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/inventory.h"
#include "Nooskewl_Wedge/stats.h"

using namespace wedge;

namespace wedge {

Object::Object()
{
	type = OBJECT_NONE;
	id = ITEM_NONE;
	quantity = 0;
	name = "";
	sell_price = 0;
	description = GLOBALS->game_t->translate(12)/* Originally: Unknown... */;
}

Object &Object::operator=(const Object &o)
{
	type = o.type;
	id = o.id;
	quantity = o.quantity;
	name = o.name;
	sell_price = o.sell_price;
	description = o.description;
	return *this;
}

Object::~Object()
{
}

std::string Object::save()
{
	return util::string_printf("{ \"type\": %d, \"id\": %d, \"quantity\": %d }", (int)type, id, quantity);
}
	
//--

Object_Interface::~Object_Interface()
{
}

Object Object_Interface::make_object(Object_Type type, int id, int quantity)
{
	Object o;
	o.type = type;
	o.id = id;
	o.quantity = quantity;
	return o;
}

int Object_Interface::use(Object object, Base_Stats *target)
{
	return 0;
}

gfx::Sprite *Object_Interface::get_sprite(Object object)
{
	return NULL;
}

Fixed_Stats Object_Interface::get_weapon_stats(int id)
{
	return Fixed_Stats();
}

Fixed_Stats Object_Interface::get_armour_stats(int id)
{
	return Fixed_Stats();
}

//--

Inventory::Inventory()
{
}

Inventory::Inventory(util::JSON::Node *json)
{
	for (int i = 0; i < MAX_OBJECTS; i++) {
		Object_Type type = OBJECT_NONE;
		int id = 0;
		int quantity = 0;

		if (i >= (int)json->children.size()) {
			continue;
		}

		util::JSON::Node *n;

		n = json->children[i]->find("\"type\"");
		if (n) {
			type = (Object_Type)json_to_integer(n);
		}
		n = json->children[i]->find("\"id\"");
		if (n) {
			id = json_to_integer(n);
		}
		n = json->children[i]->find("\"quantity\"");
		if (n) {
			quantity = json_to_integer(n);
		}

		objects[i] = OBJECT->make_object(type, id, quantity);
	}
}

int Inventory::add(Object object)
{
	int index = find(object);

	if (index < 0) {
		Object o;
		o.type = OBJECT_NONE;
		index = find(o);
		if (index < 0) {
			return 0;
		}
	}

	int max = MAX_STACK - objects[index].quantity;

	if (max < object.quantity) {
		return max;
	}

	objects[index].type = object.type;
	objects[index].id = object.id;
	objects[index].quantity += object.quantity;
	objects[index].name = object.name;
	objects[index].sell_price = object.sell_price;
	objects[index].description = object.description;

	sort();

	return object.quantity;
}

void Inventory::remove(int index, int quantity)
{
	objects[index].quantity -= quantity;

	if (objects[index].type == OBJECT_WEAPON) {
		int equipped = 0;
		for (size_t i = 0; i < MAX_PARTY; i++) {
			if (INSTANCE->stats[i].weapon.id == objects[index].id) {
				equipped++;
			}
		}
		int to_remove = MIN(2, equipped - objects[index].quantity);
		if (to_remove > 0) {
			for (size_t i = 0; i < MAX_PARTY && to_remove > 0; i++) {
				int player_index = int(MAX_PARTY-i-1);
				if (INSTANCE->stats[player_index].weapon.id == objects[index].id) {
					to_remove--;
					INSTANCE->stats[player_index].weapon = Weapon();
				}
			}
		}
	}
	if (objects[index].type == OBJECT_ARMOUR) {
		int equipped = 0;
		for (size_t i = 0; i < MAX_PARTY; i++) {
			if (INSTANCE->stats[i].armour.id == objects[index].id) {
				equipped++;
			}
		}
		int to_remove = MIN(2, equipped - objects[index].quantity);
		if (to_remove > 0) {
			for (size_t i = 0; i < MAX_PARTY && to_remove > 0; i++) {
				int player_index = int(MAX_PARTY-i-1);
				if (INSTANCE->stats[player_index].armour.id == objects[index].id) {
					to_remove--;
					INSTANCE->stats[player_index].armour = Armour();
				}
			}
		}
	}

	if (objects[index].quantity <= 0) {
		objects[index].type = OBJECT_NONE;
		objects[index].quantity = 0;
	}
}

Object *Inventory::get_all()
{
	return objects;
}

int Inventory::find(Object object)
{
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i].type == object.type) {
			if (objects[i].id == object.id) {
				return i;
			}
		}
	}

	return -1;
}

int Inventory::use(int index, Base_Stats *target)
{
	if (index < 0 || index >= MAX_OBJECTS) {
		util::errormsg("Inventory::use index out of bounds!\n");
		return 0;
	}

	int amount = 0;

	if (objects[index].type == OBJECT_ITEM) {
		amount = OBJECT->use(objects[index], target);
		if (globals->item_sfx[objects[index].id] != NULL) {
			globals->item_sfx[objects[index].id]->play(false);
		}
		remove(index, 1);
	}

	return amount;
}

int Inventory::count(Object_Type type)
{
	int count = 0;

	for (int i = 0; i < Inventory::MAX_OBJECTS; i++) {
		if (objects[i].type == type) {
			count++;
		}
	}

	return count;
}

std::string Inventory::save()
{
	std::string s;
	for (int i = 0; i < MAX_OBJECTS; i++) {
		s += "" + objects[i].save();
		if (i < MAX_OBJECTS-1) {
			s += ",";
		}
	}
	return s;
}

void Inventory::sort()
{
	for (int i = 0; i < MAX_OBJECTS-1; i++) {
		bool done = true;
		for (int j = 0; j < MAX_OBJECTS-1; j++) {
			Object &o1 = objects[j];
			Object &o2 = objects[j+1];
			bool swap;
			if (o1.type == OBJECT_NONE) {
				swap = true;
			}
			else if (o2.type == OBJECT_NONE) {
				swap = false;
			}
			else if (o1.type == OBJECT_SPECIAL && o2.type != OBJECT_SPECIAL) {
				swap = true;
			}
			else if (o2.type == OBJECT_SPECIAL && o1.type != OBJECT_SPECIAL) {
				swap = false;
			}
			else {
				std::string t1 = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(o1.name));
				std::string t2 = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(o2.name));
				if (GLOBALS->language == "English") {
					swap = t1 > t2;
				}
				else {
					try {
						swap = std::locale("")(t1, t2) == false;
					}
					catch (...) { // std::locale can fail, don't know why...
						swap = t1 > t2;
					}
				}
			}
			if (swap) {
				Object o = objects[j];
				objects[j] = objects[j+1];
				objects[j+1] = o;
				done = false;
			}
		}
		if (done) {
			break;
		}
	}
}

}
