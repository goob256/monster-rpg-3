#ifndef TRANSITION_H
#define TRANSITION_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Transition_Step : public wedge::Step
{
public:
	static const int LENGTH = 333;

	Transition_Step(bool out, wedge::Task *task); // in or out
	virtual ~Transition_Step();
	
	bool run();
	void draw_fore();
	void start();

protected:
	bool out;
	Uint32 start_time;
};

#endif // TRANSITION_H
