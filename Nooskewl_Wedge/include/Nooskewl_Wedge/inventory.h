#ifndef NOOSKEWL_WEDGE_INVENTORY_H
#define NOOSKEWL_WEDGE_INVENTORY_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/stats.h"

namespace wedge {

typedef int Item;

typedef int Weapon;

typedef int Armour;

typedef int Special;

const int ITEM_NONE = 0;
const int WEAPON_NONE = 0;
const int ARMOUR_NONE = 0;
const int SPECIAL_NONE = 0;

enum Object_Type
{
	OBJECT_NONE = 0,
	OBJECT_ITEM,
	OBJECT_WEAPON,
	OBJECT_ARMOUR,
	OBJECT_SPECIAL
};

class NOOSKEWL_WEDGE_EXPORT Object
{
public:
	Object();
	virtual Object &operator=(const Object &o);
	virtual ~Object();

	std::string save();

	// your make_object fills these in
	Object_Type type;
	int id;
	int quantity;
	std::string name;
	int sell_price;
	std::string description;
};

class NOOSKEWL_WEDGE_EXPORT Object_Interface
{
public:
	virtual ~Object_Interface();

	virtual Object make_object(Object_Type type, int id, int quantity);
	virtual int use(Object object, Base_Stats *target);
	virtual gfx::Sprite *get_sprite(Object object);
	virtual Fixed_Stats get_weapon_stats(int id);
	virtual Fixed_Stats get_armour_stats(int id);
};

class NOOSKEWL_WEDGE_EXPORT Inventory
{
public:
	static const int MAX_OBJECTS = 50;
	static const int MAX_STACK = 99;

	Inventory();
	Inventory(util::JSON::Node *json);

	int add(Object object); // returns number that could be added. if != quantity, it wasn't added
	void remove(int index, int quantity);
	int use(int index, Base_Stats *target);

	Object *get_all();
	int find(Object object); // returns the index, quantity ignored, returns -1 if not found

	int count(Object_Type type);

	std::string save();
	
	void sort();

private:

	Object objects[MAX_OBJECTS];
};

}

#endif // NOOSKEWL_WEDGE_INVENTORY_H
