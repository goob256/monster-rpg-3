#ifndef CURE_H
#define CURE_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Cure_Step : public wedge::Step
{
public:
	static void static_start();

	Cure_Step(util::Point<int> draw_pos, util::Size<int> size, wedge::Task *task);
	virtual ~Cure_Step();

	bool run();
	void draw_fore();
	void start();
	void lost_device();
	void found_device();

private:
	static gfx::Image *work1;
	static gfx::Image *work2;
	static gfx::Image *work_src;
	static gfx::Image *work_dest;
	static gfx::Image *image;
	static int count;

	static const int ADD = 50;
	static const int FALL_TIME = 250;
	static const int SPIN1 = 500;
	static const int SPIN2 = 250;

	Uint32 start_time;

	std::vector< util::Point<float> > positions;

	util::Point<int> draw_pos;
	util::Size<int> size;
	
	int NUM;
	int RISE_DURATION;
	int DURATION;

	int id;
};

#endif // CURE_H
