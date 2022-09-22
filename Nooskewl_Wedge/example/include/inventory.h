#ifndef INVENTORY_H
#define INVENTORY_H

#include <Nooskewl_Wedge/inventory.h>

// 0 is none, defined in Wedge

enum Item_Types
{
	ITEM_POTION = 1
};

class Object_Interface : public wedge::Object_Interface
{
public:
	virtual ~Object_Interface();

	wedge::Object make_object(wedge::Object_Type type, int id, int quantity);
	int use(wedge::Object object, wedge::Base_Stats *target);
};

#endif // INVENTORY_H
