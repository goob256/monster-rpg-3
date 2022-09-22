#ifndef VFIRE_H
#define VFIRE_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class vFire_Step : public wedge::Step
{
public:
	vFire_Step(wedge::Task *task);
	virtual ~vFire_Step();
	
	bool run();
	void draw_fore();
	void start();
	
	void count_fireballs();
	void model_done();

private:
	void get_mvp(glm::mat4 &mv, glm::mat4 &p);
	void draw_dragon();
	util::Point<float> get_mouth_pos();

	int prev_frame;
	audio::Sample *wings;
	int wings_played;
	int fireballs;
	int fireballs_done;
	bool _model_done;
};

#endif // VFIRE_H
