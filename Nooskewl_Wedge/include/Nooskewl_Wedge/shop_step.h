#ifndef NOOSKEWL_WEDGE_SHOP_STEP_H
#define NOOSKEWL_WEDGE_SHOP_STEP_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/inventory.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Shop_Step : public Step
{
public:
	Shop_Step(Object_Type type, std::vector<Object> items, Task *task);
	virtual ~Shop_Step();

	void start();
	bool run();

private:
	Object_Type type;
	std::vector<Object> items;
};

}

#endif // NOOSKEWL_WEDGE_SHOP_STEP_H
