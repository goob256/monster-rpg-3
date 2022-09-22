#ifndef HEAL_H
#define HEAL_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Heal_Step : public wedge::Step
{
public:
	Heal_Step(util::Point<int> draw_pos, util::Size<int> size, int pulse, wedge::Task *task);
	virtual ~Heal_Step();

	bool run();
	void draw_fore();
	void start();

private:
	static const int DURATION = 1000;

	Uint32 start_time;

	util::Point<int> draw_pos;
	util::Size<int> size;
	int pulse;

	int SLOPE;
	int SEGMENTS;
};

#endif // HEAL_H
