#ifndef VICE_H
#define VICE_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

#include "ice.h"

class vIce_Step : public Ice_Step
{
public:
	static void static_start();

	vIce_Step(util::Point<float> target, wedge::Task *task);
	virtual ~vIce_Step();
	
	bool run();
	void draw_fore();
	void start();

private:
	static const int VICE_TIME = 2500;

	static int count;
	static int destroyed_count;

	struct Icicle_Particle {
		glm::vec3 velocity;
	};

	std::vector<Icicle_Particle> gen_icicle_particles();
	void draw_icicle(util::Point<float> target, std::vector<Icicle_Particle> icicle_particles);

	util::Point<float> target;
	Uint32 icicle_start;
	std::vector<Icicle_Particle> icicle_particles;
	int nframes;
	int id;
	bool started;
	bool played_sound;
	Uint32 last_ticks;
};

#endif // VICE_H
