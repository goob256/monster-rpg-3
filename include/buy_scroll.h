#ifndef BUY_SCROLL_H
#define BUY_SCROLL_H

#include <Nooskewl_Wedge/systems.h>

class Buy_Scroll_Step : public wedge::Step
{
public:
	Buy_Scroll_Step(int item_id, int cost, wedge::Task *task);
	virtual ~Buy_Scroll_Step();

	bool run();
	void done_signal(wedge::Step *step);

private:
	int item_id;
	int cost;
	bool done;
};

#endif // BUY_SCROLL_H
