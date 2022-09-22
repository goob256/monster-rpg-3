#ifndef CAPTAIN_H
#define CAPTAIN_H

#include <Nooskewl_Wedge/systems.h>

class Captain_Step : public wedge::Step
{
public:
	Captain_Step(wedge::Task *task);
	virtual ~Captain_Step();

	bool run();
	void done_signal(wedge::Step *step);

private:
	bool done;
};

#endif // CAPTAIN_H
