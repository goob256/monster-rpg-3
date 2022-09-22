#ifndef FISHING_PRIZE_H
#define FISHING_PRIZE_H

#include <Nooskewl_Wedge/systems.h>

class Fishing_Prize_Step : public wedge::Step
{
public:
	Fishing_Prize_Step(wedge::Task *task);
	virtual ~Fishing_Prize_Step();

	bool run();

private:
	void spawn_splash(util::Point<int> pos);

	int frame;
};

#endif // FISHING_PRIZE_H
