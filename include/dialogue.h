#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

#include "globals.h"

class Dialogue_Step : public wedge::Step
{
public:
	static const int DELAY = 50;
	static const int FADE_TIME = 250;
	static const int BORDER = 3;

	static void get_positions(int *HEIGHT, int *indicator_pos/*FIXME misnomer: indicator_h is better*/, int *y, util::Size<int> *PAD, util::Point<int> *text_pos, wedge::Dialogue_Position position);

	// tag is used for Name: and maybe other things in bold
	Dialogue_Step(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Task *task, bool darken_screen = false);
	virtual ~Dialogue_Step();
	
	virtual bool run();
	virtual void handle_event(TGUI_Event *event);
	virtual void draw_fore();
	virtual void start();

	void set_dismissable(bool dismissable);
	void dismiss();

	bool get_entity_movement_was_paused();

private:
	std::string tag;
	std::string text;
	wedge::Dialogue_Type type;
	wedge::Dialogue_Position position;
	int started_time;
	int started_time_transition;
	int start_character;
	int tag_width;
	util::Point<int> text_pos;
	bool done;
	wedge::System *entity_movement_system;
	bool entity_movement_was_paused;
	std::list<gfx::Sprite *> unpause;
	Uint32 fade_out_start_time;
	bool dismissable;
	bool in_battle;
	int count;
	bool sent_done;
	bool darken_screen;
};

#endif // DIALOGUE_H
