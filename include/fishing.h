#ifndef FISHING_H
#define FISHING_H

#include <Nooskewl_Wedge/systems.h>

class Fishing_Step : public wedge::Step
{
public:
	Fishing_Step(wedge::Task *task);
	virtual ~Fishing_Step();

	bool run();
	void done_signal(wedge::Step *step);

	void set_done(bool done);

private:
	bool done;
};

#endif // FISHING_H
