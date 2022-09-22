#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Dialogue_Step : public wedge::Step
{
public:
	// tag is used for Name: and maybe other things in bold
	Dialogue_Step(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Task *task);
	virtual ~Dialogue_Step();
	
	bool run();
	void handle_event(TGUI_Event *event);
	void draw();

private:
	std::string tag;
	std::string text;
	wedge::Dialogue_Type type;
	wedge::Dialogue_Position position;
	bool done;
};

#endif // DIALOGUE_H
