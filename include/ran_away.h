#ifndef RAN_AWAY_H
#define RAN_AWAY_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Ran_Away_Step : public wedge::Step
{
public:
	Ran_Away_Step(wedge::Task *task);
	virtual ~Ran_Away_Step();
	
	bool run();
	void done_signal(wedge::Step *step);

private:
	bool done;
};

#endif // RAN_AWAY_H
