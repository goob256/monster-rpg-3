#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/shop_step.h"

using namespace wedge;

namespace wedge {

Shop_Step::Shop_Step(Object_Type type, std::vector<Object> items, Task *task) :
	Step(task),
	type(type),
	items(items)
{
}

Shop_Step::~Shop_Step()
{
}

void Shop_Step::start()
{
	AREA->start_shop(type, items);
}

bool Shop_Step::run()
{
	send_done_signal();
	return false;
}

}
