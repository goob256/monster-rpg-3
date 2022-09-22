#ifndef SCROLL_HELP_H
#define SCROLL_HELP_H

#include <Nooskewl_Wedge/generic_gui.h>
#include <Nooskewl_Wedge/systems.h>

class Scroll_Help_Step : public wedge::Step
{
public:
	Scroll_Help_Step(std::string tag, wedge::Task *task);
	virtual ~Scroll_Help_Step();

	void start();
	bool run();
	void done_signal(Step *step);

	int get_choice();
	void set_choice(int choice);

private:
	bool done;
	std::string tag;
	int count;
};

#endif // SCROLL_HELP_H
