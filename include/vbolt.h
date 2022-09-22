#ifndef VBOLT_H
#define VBOLT_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

#include "bolt.h"

class vBolt_Step : public Bolt_Step
{
public:
	vBolt_Step(util::Point<float> start, util::Point<float> end, int rough_segments, bool mirror, SDL_Colour c1, SDL_Colour c2, SDL_Colour c3, SDL_Colour c4, wedge::Task *task);
	virtual ~vBolt_Step();
	
	bool run();
	void draw_fore();
	void start();

private:
	static const int POOF_TIME = 3000;
	static const int ZEUS_TIME = 3000;
	static const int MAX_POOF_SCALE = 2;
	static const int NPOOFS = 6;
	static const int NUM_CLOUDS = 32;
	static const float CLOUD_H;
	static const float CLOUD_W;

	void get_zeus_mvp(glm::mat4 &mv, glm::mat4 &p);
	void draw_zeus();

	Uint32 total_start_time; // start of poof
	bool started;
	bool zeus_started;

	struct Poof { // poof at start of spell
		util::Point<float> position;
		util::Point<float> speed;
		float scale;
		float scale_speed;
		float angle;
		float angle_speed;
		int flags;
		float alpha;
	};

	std::vector<Poof> poofs;

	struct Cloud { // for covering zeus's lower half
		util::Point<float> position;
		float z;
		float scale;
		float angle;
		float angle_speed;
		float spin_offset;
		int flags;
	};

	std::vector<Cloud> clouds;

	bool sample_played;
};

#endif // VBOLT_H
