#ifndef BOLT_H
#define BOLT_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Bolt_Step : public wedge::Step
{
public:
	static void static_start();

	Bolt_Step(util::Point<float> start, util::Point<float> end, int rough_segments, bool mirror, SDL_Colour c1, SDL_Colour c2, SDL_Colour c3, SDL_Colour c4, wedge::Task *task);
	virtual ~Bolt_Step();
	
	bool run();
	void draw_fore();
	void start();
	void handle_event(TGUI_Event *event);

protected:
	static int count;

	static const int DURATION = 1000;

	void create();
	void destroy();
	std::vector< util::Point<float> > calc_bolt(util::Point<float> start, util::Point<float> end, int direction, int rough_segments, int snap);
	float get_p(float p, int segment, int nsegments);
	void draw_bolt(float p, float scale, float max_scale, SDL_Colour colour);
	std::vector< util::Point<float> > flip(util::Point<float> end, std::vector< util::Point<float> > v);

	std::vector< util::Point<float> > bolt;
	std::vector< std::vector< util::Point<float> > > branches;

	Uint32 start_time;

	std::vector<int> segments;

	int id;

	util::Point<float> _start;
	util::Point<float> end;
	int rough_segments;
	bool mirror;
	SDL_Colour c1, c2, c3, c4;

	std::vector<int> branch_directions;
};

#endif // BOLT_H
